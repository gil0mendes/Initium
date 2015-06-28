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
 * @brief               BIOS video detection.
 */

#include <lib/utility.h>

#include <drivers/video/vga.h>

#include <bios/bios.h>
#include <bios/vbe.h>

#include <console.h>
#include <loader.h>
#include <memory.h>
#include <video.h>

/** BIOS video mode structure. */
typedef struct bios_video_mode {
    video_mode_t mode;                  /**< Video mode structure. */
    uint16_t num;                       /**< Mode number. */
} bios_video_mode_t;

/** VGA console definitions. */
#define VGA_MEM_BASE        0xb8000
#define VGA_COLS            80
#define VGA_ROWS            25

/**
 * Set a BIOS video mode.
 *
 * @param _mode         Mode to set.
 * @return              Status code describing the result of the operation.
 */
static status_t bios_video_set_mode(video_mode_t *_mode) {
    bios_video_mode_t *mode = (bios_video_mode_t *)_mode;
    bios_regs_t regs;

    bios_regs_init(&regs);
    regs.eax = VBE_FUNCTION_SET_MODE;
    regs.ebx = mode->num;
    bios_call(0x10, &regs);
    if ((regs.ax & 0xff00) != 0) {
        dprintf("bios: failed to set VBE mode 0x%" PRIx16 " (0x%" PRIx16 ")\n", mode->num, regs.ax);
        return STATUS_SYSTEM_ERROR;
    }

    return STATUS_SUCCESS;
}

/** BIOS VBE video operations. */
static video_ops_t bios_vbe_video_ops = {
    .set_mode = &bios_video_set_mode,
};

/** BIOS VGA video operations. */
static video_ops_t bios_vga_video_ops = {
    .console = &vga_console_out_ops,
    .set_mode = &bios_video_set_mode,
};

/** Detect available video modes. */
void bios_video_init(void) {
    bios_video_mode_t *mode;
    vbe_info_t *info = (vbe_info_t *)BIOS_MEM_BASE;
    vbe_mode_info_t *mode_info = (vbe_mode_info_t *)(BIOS_MEM_BASE + sizeof(vbe_info_t));
    bios_regs_t regs;
    uint16_t *location;


    /* Register the VGA video mode, which will set the main console. */
    mode = malloc(sizeof(*mode));
    mode->num = 3;
    mode->mode.type = VIDEO_MODE_VGA;
    mode->mode.ops = &bios_vga_video_ops;
    mode->mode.width = VGA_COLS;
    mode->mode.height = VGA_ROWS;
    mode->mode.mem_phys = mode->mode.mem_virt = VGA_MEM_BASE;
    mode->mode.mem_size = round_up(VGA_COLS * VGA_ROWS * sizeof(uint16_t), PAGE_SIZE);
    video_mode_register(&mode->mode, true);

    /* Try to get controller information. */
    strncpy(info->vbe_signature, VBE_SIGNATURE, 4);
    bios_regs_init(&regs);
    regs.eax = VBE_FUNCTION_CONTROLLER_INFO;
    regs.edi = (ptr_t)info;
    bios_call(0x10, &regs);
    if ((regs.eax & 0xff) != 0x4f) {
        dprintf("bios: VBE is not supported\n");
        return;
    } else if((regs.ax & 0xff00) != 0) {
        dprintf("bios: failed to obtain VBE information (0x%" PRIx16 ")\n", regs.ax);
        return;
    }

    /* Iterate through the modes. 0xffff indicates the end of the list. */
    location = (uint16_t *)segoff_to_linear(info->video_mode_ptr);
    for (size_t i = 0; location[i] != 0xffff; i++) {
        bios_video_mode_t *mode;

        bios_regs_init(&regs);
        regs.eax = VBE_FUNCTION_MODE_INFO;
        regs.ecx = location[i];
        regs.edi = (ptr_t)mode_info;
        bios_call(0x10, &regs);
        if ((regs.eax & 0xFF00) != 0) {
            dprintf("bios: failed to obtain VBE mode information (0x%" PRIx16 ")\n", regs.ax);
            continue;
        }

        /* Check if the mode is suitable. TODO: Indexed modes. */
        if (mode_info->memory_model != 6) {
            continue;
        } else if ((mode_info->mode_attributes & (1<<0)) == 0) {
            /* Not supported. */
            continue;
        } else if ((mode_info->mode_attributes & (1<<3)) == 0) {
            /* Not colour. */
            continue;
        } else if ((mode_info->mode_attributes & (1<<4)) == 0) {
            /* Not a graphics mode. */
            continue;
        } else if ((mode_info->mode_attributes & (1<<7)) == 0) {
            /* Not usable in linear mode. */
            continue;
        } else if (mode_info->bits_per_pixel < 8) {
            continue;
        } else if (mode_info->phys_base_ptr == 0) {
            continue;
        }

        /* Add the mode to the list. */
        mode = malloc(sizeof(*mode));
        mode->num = location[i] | VBE_MODE_LFB;
        mode->mode.type = VIDEO_MODE_LFB;
        mode->mode.ops = &bios_vbe_video_ops;
        mode->mode.width = mode_info->x_resolution;
        mode->mode.height = mode_info->y_resolution;
        mode->mode.bpp = mode_info->bits_per_pixel;
        mode->mode.pitch = (info->vbe_version_major >= 3)
            ? mode_info->lin_bytes_per_scan_line
            : mode_info->bytes_per_scan_line;
        mode->mode.red_size = mode_info->red_mask_size;
        mode->mode.red_pos = mode_info->red_field_position;
        mode->mode.green_size = mode_info->green_mask_size;
        mode->mode.green_pos = mode_info->green_field_position;
        mode->mode.blue_size = mode_info->blue_mask_size;
        mode->mode.blue_pos = mode_info->blue_field_position;
        mode->mode.mem_phys = mode->mode.mem_virt = mode_info->phys_base_ptr;
        mode->mode.mem_size = round_up(mode->mode.height * mode->mode.pitch, PAGE_SIZE);

        video_mode_register(&mode->mode, false);
    }
}
