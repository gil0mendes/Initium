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

#include <bios/vbe.h>
#include <bios/bios.h>
#include <bios/multiboot.h>

#include <device.h>
#include <loader.h>
#include <screen.h>
#include <memory.h>

extern void loader_main(void);

/**
 * Main function of the BIOS loader
 */
void bios_init(void)
{
    // Initialize architecture code
    arch_init();

    // Initialize the console
    bios_console_init();

    // Call loader main function
    loader_main();
}

/**
 * Detect and register all devices.
 */
void target_device_probe(void) {
    bios_disk_init();
}
