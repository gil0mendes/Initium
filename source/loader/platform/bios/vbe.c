/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Gil Mendes
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
 * @brief             VBE video mode support
 */

#include <lib/string.h>

#include <bios/bios.h>
#include <bios/vbe.h>

#include <loader.h>
#include <memory.h>

// VBE controller information
vbe_info_t vbe_info;

// List of detected VBE modes
LIST_DECLARE(vbe_modes);

// Default VBE mode
vbe_mode_t *default_vbe_mode = NULL;

// Perferred/fallback video modes
#define PREFERRED_MODE_WIDTH     1024
#define PREFERRED_MODE_HEIGHT    768
#define FALLBACK_MODE_WIDTH      800
#define FALLBACK_MODE_HEIGHT     600

// ============================================================================
// Set a VBE video mode
//
// @param mode         Mode to set
//
void vbe_mode_set(vbe_mode_t *mode) {
    bios_regs_t regs;

    // Set the mode. Bit 14 in the mode ID indicates that we wish to use
    // the linear framebuffer model
    bios_regs_init(&regs);
    regs.eax = VBE_FUNCTION_SET_MODE;
    regs.ebx = mode->id | (1<<14);
    bios_call(0x10, &regs);

    // Print debuf indo
    dprintf("vbe: set VBE mode 0x%x: %dx%dx%dx (framebuffer: 0x%" PRIx32 ")\n",
        mode->id, mode->info.x_resolution, mode->info.y_resolution,
        mode->info.bits_per_pixel, mode->info.phys_base_ptr);
}

// ============================================================================
// Find a video mode
//
// @param width        Width of the mode
// @param height       Height of the mode
// @param depth        Gepth of the mode, or 0 to search for highest depth.
// @return             Pointer to mode found, or NULL if no matching mode
//
vbe_mode_t *vbe_mode_find(uint16_t width, uint16_t height, uint8_t depth) {
    vbe_mode_t *mode, *ret = NULL;

    // Iterate all modes
    LIST_FOREACH(&vbe_modes, iter) {
        mode = list_entry(iter, vbe_mode_t, header);

        // Check if the mode match with the required resolution
        if (mode->info.x_resolution == width && mode->info.y_resolution == height) {
            // Is to check for the highest depth?
            if (depth) {
                // Check if depth value matches with the required value
                if (mode->info.bits_per_pixel == depth) {
                    return mode;
                }
            } else {
                ret = mode;
            }
        }
    }

    return ret;
}

// ============================================================================
// Detect available video modes
//
void vbe_init(void) {
    // THis will store modes info
    vbe_mode_info_t *minfo = (vbe_mode_info_t *)(BIOS_MEM_BASE + sizeof(vbe_info_t));

    // This will store VBE info
    vbe_info_t *info = (vbe_info_t *)BIOS_MEM_BASE;

    uint16_t *location;
    bios_regs_t regs;
    vbe_mode_t *mode;
    size_t i;

    // Try to get VBE information
    strncpy(info->vbe_signature, "VBE2", 4);
    bios_regs_init(&regs);
    regs.eax = VBE_FUNCTION_CONTROLLER_INFO;
    regs.edi = BIOS_MEM_BASE;
    bios_call(0x10, &regs);
    if ((regs.eax & 0xFF) != 0x4F) {
        dprintf("vbe: VBE is not supported\n");
        return;
    } else if((regs.eax & 0xFF00) != 0) {
        dprintf("vbe: could not obtain VBE information (0x%x)\n", regs.eax & 0xFFFF);
        return;
    }

    // Show debug info
    dprintf("vbe: vbe presence was detected:\n");
    dprintf(" signature:    %s\n", info->vbe_signature);
    dprintf(" version:      %u.%u\n", info->vbe_version_major, info->vbe_version_minor);
    dprintf(" capabilities: 0x%" PRIx32 "\n", info->capabilities);
    dprintf(" mode pointer: 0x%" PRIx32 "\n", info->video_mode_ptr);
    dprintf(" total memory: %" PRIu16 "KB\n", info->total_memory * 64);
    if(info->vbe_version_major >= 2) {
        dprintf(" OEM revision: 0x%" PRIx16 "\n", info->oem_software_rev);
    }

    // Put the info available to all code
    memcpy(&vbe_info, info, sizeof(vbe_info_t));

    // Iterate through the modes.
    // 0xFFFF indicates the end of the list
    location = (uint16_t *)SEGOFF2LIN(info->video_mode_ptr);
    for (i = 0; location[i] != 0xFFFF; i++) {
        // Get mode info
        bios_regs_init(&regs);
        regs.eax = VBE_FUNCTION_MODE_INFO;
        regs.ecx = location[i];
        regs.edi = (BIOS_MEM_BASE + sizeof(vbe_info_t));
        bios_call(0x10, &regs);

        // Check if supports the current mode
        if ((regs.eax & 0xFF00) != 0) {
            dprintf("vbe: could not obtain VBE mode information (0x%x)\n",
                (regs.eax & 0xFFFF));
                continue;
        }

        // Check if the mode is suitable
        if (minfo->memory_model != 4 && minfo->memory_model != 6) {
            // Not packed-pixel or direct colour
            continue;
        } else if ((minfo->mode_attributes & (1<<0)) == 0) {
            // Not supported
            continue;
        } else if ((minfo->mode_attributes & (1<<3)) == 0) {
            // Color not supported
            continue;
        } else if ((minfo->mode_attributes & (1<<4)) == 0) {
            // Not support graphics mode
            continue;
        } else if ((minfo->mode_attributes & (1<<7)) == 0) {
            // Linear mode not available
            continue;
        } else if (minfo->bits_per_pixel < 8) {
            // Not suport 256 colors
            continue;
        } else if (minfo->phys_base_ptr == 0) {
            continue;
        }

        // Add mode to the list
        mode = malloc(sizeof(vbe_mode_t));
        list_init(&mode->header);
        mode->id = location[i];
        memcpy(&mode->info, minfo, sizeof(mode->info));
        list_append(&vbe_modes, &mode->header);

        // Try to find the mode to use.
        if(!(default_vbe_mode = vbe_mode_find(PREFERRED_MODE_WIDTH, PREFERRED_MODE_HEIGHT, 0))) {
            default_vbe_mode = vbe_mode_find(FALLBACK_MODE_WIDTH, FALLBACK_MODE_HEIGHT, 0);
        }
    }
}
