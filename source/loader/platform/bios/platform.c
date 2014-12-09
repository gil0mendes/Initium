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
 * @brief		BIOS platform main functions
 */

#include <arch/io.h>

#include <bios/vbe.h>
#include <bios/bios.h>
#include <bios/multiboot.h>

#include <disk.h>
#include <loader.h>
#include <screen.h>

extern void loader_main(void);

/**
 * Main function of the BIOS loader
 */
void platform_init(void)
{
    // Initialize architecture code
    arch_init();

    // Initialize the console
    bios_console_init();

    // Detect memory
    bios_memory_init();

    // Initialize disk
    disk_init();

    #ifdef CONFIG_GUI_MODE
    // Initialize VBE video mode
    vbe_init();

    // Initialize with a default mode
    // TODO: Add a config option to specify a mode
    vbe_mode_set(default_vbe_mode);

    // Screen initialization
    screenInit();
    #endif

    // Parse information from Multiboot
    multiboot_init();

    // Call loader main function
    loader_main();
}

/**
 * Reboot the system.
 */
void platform_reboot(void)
{
    uint8_t val;

    // Try the keyboard controller
    do {
        val = in8(0x64);

        if (val & (1<<0)) {
            in8(0x60);
        }
    } while(val & (1<<1));

    out8(0x64, 0xfe);

    // @TODO If this not work try a triple fault
}
