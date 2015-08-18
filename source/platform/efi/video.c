/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Gil Mendes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

 /**
 * @file
 * @brief               EFI video mode detection.
 *
 * TODO:
 *  - Need to implement UGA support for pre-UEFI 2.0 machines, such as Macs.
 */

#include <drivers/video/fb.h>

#include <lib/utility.h>

#include <efi/efi.h>

#include <console.h>
#include <loader.h>
#include <memory.h>
#include <video.h>

/** EFI video mode structure. */
typedef struct efi_video_mode {
    video_mode_t mode;                      /**< Video mode structure. */

    efi_graphics_output_protocol_t *gop;    /**< Graphics output protocol. */
    uint32_t num;                           /**< Mode number. */
} efi_video_mode_t;

/** Graphics output protocol GUID. */
static efi_guid_t graphics_output_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

/** Set an EFI video mode.
 * @param _mode         Mode to set.
 * @return              Status code describing the result of the operation. */
static status_t efi_video_set_mode(video_mode_t *_mode) {
    efi_video_mode_t *mode = (efi_video_mode_t *)_mode;
    efi_status_t ret;

    ret = efi_call(mode->gop->set_mode, mode->gop, mode->num);
    if (ret != EFI_SUCCESS) {
        dprintf("efi: failed to set video mode %u with status 0x%zx\n", mode->num, ret);
        return efi_convert_status(ret);
    }

    /* Get the framebuffer information. */
    mode->mode.mem_phys = mode->gop->mode->frame_buffer_base;
    mode->mode.mem_virt = mode->gop->mode->frame_buffer_base;
    mode->mode.mem_size = mode->gop->mode->frame_buffer_size;

    return STATUS_SUCCESS;
}

/** EFI video operations. */
static video_ops_t efi_video_ops = {
    .console = &fb_console_out_ops,
    .set_mode = efi_video_set_mode,
};

/** Get the depth for a GOP mode.
 * @param info          Mode information.
 * @return              Bits per pixel for the mode, or 0 if not supported. */
static uint8_t get_mode_bpp(efi_graphics_output_mode_information_t *info) {
    uint32_t mask;

    switch (info->pixel_format) {
    case EFI_PIXEL_FORMAT_RGBR8:
    case EFI_PIXEL_FORMAT_BGRR8:
        return 32;
    case EFI_PIXEL_FORMAT_BITMASK:
        /* Get the last set bit in the complete mask. */
        mask = info->pixel_bitmask.red_mask
            | info->pixel_bitmask.green_mask
            | info->pixel_bitmask.blue_mask
            | info->pixel_bitmask.reserved_mask;
        return fls(mask);
    default:
        return 0;
    }
}

/** Calculate a component size and position from a bitmask.
 * @param mask          Mask to convert.
 * @param _size         Where to store component size.
 * @param _pos          Where to store component position. */
static void get_component_size_pos(uint32_t mask, uint8_t *_size, uint8_t *_pos) {
    int first = ffs(mask);
    int last = fls(mask);

    *_size = last - first + 1;
    *_pos = first - 1;
}

/** Detect available video modes. */
void efi_video_init(void) {
    efi_handle_t *handles;
    efi_uintn_t num_handles;
    efi_graphics_output_protocol_t *gop;
    video_mode_t *best;
    efi_status_t ret;

    /* Look for a graphics output handle. */
    ret = efi_locate_handle(EFI_BY_PROTOCOL, &graphics_output_guid, NULL, &handles, &num_handles);
    if (ret != EFI_SUCCESS)
        return;

    /* Just use the first handle. */
    ret = efi_open_protocol(handles[0], &graphics_output_guid, EFI_OPEN_PROTOCOL_GET_PROTOCOL, (void **)&gop);
    free(handles);
    if (ret != EFI_SUCCESS)
        return;

    /* Get information on all available modes. */
    best = NULL;
    for (efi_uint32_t i = 0; i < gop->mode->max_mode; i++) {
        efi_graphics_output_mode_information_t *info;
        efi_uintn_t size;
        efi_video_mode_t *mode;

        ret = efi_call(gop->query_mode, gop, i, &size, &info);
        if (ret != STATUS_SUCCESS)
            continue;

        mode = malloc(sizeof(*mode));
        mode->gop = gop;
        mode->num = i;
        mode->mode.type = VIDEO_MODE_LFB;
        mode->mode.ops = &efi_video_ops;
        mode->mode.width = info->horizontal_resolution;
        mode->mode.height = info->vertical_resolution;

        mode->mode.bpp = get_mode_bpp(info);
        if (!mode->mode.bpp || mode->mode.bpp & 0x3)
            continue;

        mode->mode.pitch = info->pixels_per_scanline * (mode->mode.bpp >> 3);

        switch (info->pixel_format) {
        case EFI_PIXEL_FORMAT_RGBR8:
            mode->mode.red_size = mode->mode.green_size = mode->mode.blue_size = 8;
            mode->mode.red_pos = 0;
            mode->mode.green_pos = 8;
            mode->mode.blue_pos = 16;
            break;
        case EFI_PIXEL_FORMAT_BGRR8:
            mode->mode.red_size = mode->mode.green_size = mode->mode.blue_size = 8;
            mode->mode.red_pos = 16;
            mode->mode.green_pos = 8;
            mode->mode.blue_pos = 0;
            break;
        case EFI_PIXEL_FORMAT_BITMASK:
            get_component_size_pos(info->pixel_bitmask.red_mask, &mode->mode.red_size, &mode->mode.red_pos);
            get_component_size_pos(info->pixel_bitmask.green_mask, &mode->mode.green_size, &mode->mode.green_pos);
            get_component_size_pos(info->pixel_bitmask.blue_mask, &mode->mode.blue_size, &mode->mode.blue_pos);
            break;
        default:
            break;
        }

        /* If the current mode width is less than 1024, we try to set 1024x768,
         * else we just keep the current. */
        if (i == gop->mode->mode) {
            if (!best || mode->mode.width >= 1024)
                best = &mode->mode;
        } else if (mode->mode.width == 1024 && mode->mode.height == 768) {
            if (!best || best->width < 1024 || (best->width == 1024 && best->height == 768 && mode->mode.bpp > best->bpp))
                best = &mode->mode;
        }

        video_mode_register(&mode->mode, false);
    }

    video_set_mode(best);
}
