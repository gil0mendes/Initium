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
 * @brief               Virtual memory region allocator.
 */

 #ifndef __LIB_ALLOCATOR_H
 #define __LIB_ALLOCATOR_H

 #include <lib/list.h>

 #include <loader.h>

/** Structure containing a virtual region allocator. */
typedef struct allocator {
        load_ptr_t start;            /**< Start of the region that the allocator manages. */
        load_size_t size;            /**< Size of the region that the allocator manages. */
        list_t regions;              /**< List of regions. */
} allocator_t;

extern bool allocator_alloc(allocator_t *alloc, load_size_t size, load_size_t align, load_ptr_t *_addr);
extern bool allocator_insert(allocator_t *alloc, load_ptr_t addr, load_size_t size);
extern void allocator_reserve(allocator_t *alloc, load_ptr_t addr, load_size_t size);

extern void allocator_init(allocator_t *alloc, load_ptr_t start, load_size_t size);

 #endif /* __LIB_ALLOCATOR_H */
