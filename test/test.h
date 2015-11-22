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

#include <arch/page.h>

#include <lib/utility.h>

#include <assert.h>
#include <elf.h>
#include <initium.h>
#include <loader.h>

#undef vprintf
#undef printf
#undef dvprintf
#undef dprintf

/** Address space definitions. */
#ifdef __LP64__
#   define PHYS_MAP_BASE        0xfffffffe00000000
#   define PHYS_MAP_SIZE        0x100000000
#   define VIRT_MAP_BASE        0xffffffff00000000
#   define VIRT_MAP_SIZE        0x100000000
#   define PHYS_MAX             0xffffffffffffffff
#else
#   define VIRT_MAP_BASE        0xc0000000
#   define VIRT_MAP_SIZE        0x40000000
#   define PHYS_MAX             0xffffffff
#endif

extern void mmu_map(ptr_t virt, phys_ptr_t phys, size_t size);

extern ptr_t virt_alloc(size_t size);

extern void *phys_map(phys_ptr_t addr, size_t size);
extern phys_ptr_t phys_alloc(phys_size_t size);

extern int vprintf(const char *fmt, va_list args);
extern int printf(const char *fmt, ...) __printf(1, 2);

extern void console_init(initium_tag_t *tags);
extern void log_init(initium_tag_t *tags);
extern void mm_init(initium_tag_t *tags);
extern void mmu_init(initium_tag_t *tags);

extern void kmain(uint32_t magic, initium_tag_t *tags);

#endif /* __TEST_H */
