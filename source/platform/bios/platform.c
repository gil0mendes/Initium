/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2015 Gil Mendes
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

#include <x86/cpu.h>
#include <x86/descriptor.h>

#include <bios/vbe.h>
#include <bios/bios.h>
#include <bios/multiboot.h>

#include <device.h>
#include <loader.h>
#include <screen.h>
#include <memory.h>

extern void loader_main(void);

/**
 * Reboot the system.
 */
void target_reboot(void) {
    uint8_t val;
    uint64_t target;

    /* Try the keyboard controller. */
    do {
        val = in8(0x64);
        if (val & (1 << 0)) {
            in8(0x60);
        }
    } while (val & (1 << 1));
    out8(0x64, 0xfe);

    /* FIXME: delay function. */
    target = x86_rdtsc() + 1000000000ull;
    while (x86_rdtsc() < target) {
        __asm__ volatile("pause");
    }

    /* Fall back on a triple fault. */
    x86_lidt(0, 0);
    __asm__ volatile("ud2");

    while (true);
}

/**
 * Main function of the BIOS loader
 */
void bios_main(void)
{
    // Initialize architecture code
    arch_init();

    // Initialize the console and video
    bios_console_init();
    bios_video_init();

    // Call loader main function
    loader_main();
}

/**
 * Detect and register all devices.
 */
void target_device_probe(void) {
    bios_disk_init();
}
