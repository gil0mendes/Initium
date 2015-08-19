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
 * @brief               x86 paging definitions.
 */

#ifndef __ARCH_PAGE_H
#define __ARCH_PAGE_H

/** Page size definitions. */
#define PAGE_WIDTH          12              /**< Width of a page in bits. */
#define PAGE_SIZE           0x1000          /**< Size of a page. */
#define LARGE_PAGE_SIZE_32  0x400000        /**< 32-bit large page size. */
#define LARGE_PAGE_SIZE_64  0x200000        /**< 64-bit large page size. */

/** Masks to clear page offset and unsupported bits from a virtual address. */
#define PAGE_MASK_32        0xfffff000ul
#define PAGE_MASK_64        0x000ffffffffff000ull

/** Native paging definitions. */
#ifdef __LP64__
#   define LARGE_PAGE_SIZE  LARGE_PAGE_SIZE_64
#   define PAGE_MASK        PAGE_MASK_64
#else
#   define LARGE_PAGE_SIZE  LARGE_PAGE_SIZE_32
#   define PAGE_MASK        PAGE_MASK_32
#endif

/** Mask to clear page offset and unsupported bits from a physical address. */
#define PHYS_PAGE_MASK_64   0x000000fffffff000ull
#define PHYS_PAGE_MASK_32   0xfffff000

#endif /* __ARCH_PAGE_H */
