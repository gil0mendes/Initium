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
 * @brief		x86 type definitions.
 */

#ifndef __ARCH_TYPES_H
#define __ARCH_TYPES_H

/** Format character definitions for printf(). */
#ifdef __LP64__
#   define PRIxPHYS	   "lx"		    /**< Format for phys_ptr_t (hexadecimal). */
#   define PRIuPHYS	   "lu"		    /**< Format for phys_ptr_t. */
#   define PRIxLOAD    "lx"         /**< Format for load_ptr_t (hexadecimal). */
#   define PRIuLOAD    "lu"         /**< Format for load_ptr_t. */
#else
#   define PRIxPHYS	   "llx"		/**< Format for phys_ptr_t (hexadecimal). */
#   define PRIuPHYS	   "llu"		/**< Format for phys_ptr_t. */
#   define PRIxLOAD    "llx"        /**< Format for load_ptr_t (hexadecimal). */
#   define PRIuLOAD    "llu"        /**< Format for load_ptr_t. */
#endif

/** Integer type that can represent a pointer in the loader's address space. */
typedef unsigned long ptr_t;

/**
 * Integer pointer/size types for use by OS loaders.
 *
 * On x86 we can load both 32-bit and 64-bit kernels, so there are defined to be
 * 64-bit to account for this.
 */
typedef uint64_t load_ptr_t;
typedef uint64_t load_size_t;

/** Integer types that can represent a physical address/size. */
typedef uint64_t phys_ptr_t;
typedef uint64_t phys_size_t;

#endif /* __ARCH_TYPES_H */
