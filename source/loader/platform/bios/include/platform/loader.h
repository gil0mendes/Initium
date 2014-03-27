/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 <author>
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
 * @brief		BIOS platform core definitions.
 */

#ifndef __PLATFORM_LOADER_H
#define __PLATFORM_LOADER_H

/** Load address of the boot loader. */
#define LOADER_LOAD_ADDR	0x10000

/**
 * Multiboot load address.
 *
 * When we are loaded via Multiboot, we cannot load to our real load address
 * as the boot loader that loads us is probably there. Therefore, we load
 * higher up, and the entry code will relocate us to the correct place.
 */
#define MULTIBOOT_LOAD_ADDR	0x100000

/** Load offset for Multiboot. */
#define MULTIBOOT_LOAD_OFFSET	(MULTIBOOT_LOAD_ADDR - LOADER_LOAD_ADDR)

#ifndef __ASM__

extern void system_reboot(void);

#endif /* __ASM__ */
#endif /* __PLATFORM_LOADER_H */
