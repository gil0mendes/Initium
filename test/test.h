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
* FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

/**
 * @file
 * @brief               Initium test kernel definitions.
 */

#ifndef __TEST_H
#define __TEST_H

#include <arch/loader.h>

#include <platform/loader.h>

#ifdef __LP64__
#   define PHYS_MAP_BASE        0xffffffff00000000
#   define PHYS_MAP_SIZE        0x80000000
#   define VIRT_MAP_BASE        0xffffffff80000000
#   define VIRT_MAP_SIZE        0x80000000
#else
#   define PHYS_MAP_BASE        0x40000000
#   define PHYS_MAP_SIZE        0x80000000
#   define VIRT_MAP_BASE        0xc0000000
#   define VIRT_MAP_SIZE        0x40000000
#endif

/* Platform-dependent. */
#define PHYS_MAP_OFFSET         0

/* Make use of the virt/phys conversion functions in loader.h, but override the
 * arch/platform definitions for the loader and set them to be correct for our
 * address space. */
#undef TARGET_VIRT_OFFSET
#define TARGET_VIRT_OFFSET      PHYS_MAP_BASE - PHYS_MAP_OFFSET

#include <elf.h>
#include <initium.h>
#include <loader.h>

#undef vprintf
#undef printf
#undef dvprintf
#undef dprintf

extern int vprintf(const char *fmt, va_list args);
extern int printf(const char *fmt, ...) __printf(1, 2);

extern void console_init(initium_tag_t *tags);
extern void log_init(initium_tag_t *tags);

extern void kmain(uint32_t magic, initium_tag_t *tags);

#endif /* __TEST_H */
