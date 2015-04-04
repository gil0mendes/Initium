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
 * @brief             Memory management functions
 */

#ifndef __MEMORY_H
#define __MEMORY_H

#include <arch/page.h>

#include <lib/list.h>

// Physical memory range descriptor
typedef struct memory_range {
       list_t header;                  /**< Link to memory range list. */

       phys_ptr_t start;               /**< Start of range. */
       phys_size_t size;               /**< Size of range. */
       uint8_t type;                   /**< Type of the range. */
} memory_range_t;

/**
 * Memory type definitions.
 *
 * Memory types to be used with the memory allocation functions. These match
 * the types specified by the KBoot spec, with some additions.
 */
#define MEMORY_TYPE_FREE       0       /**< Free, usable memory. */
#define MEMORY_TYPE_ALLOCATED  1       /**< Kernel image and other non-reclaimable data. */
#define MEMORY_TYPE_RECLAIMABLE        2       /**< Memory reclaimable when boot information is no longer needed. */
#define MEMORY_TYPE_PAGETABLES 3       /**< Temporary page tables for the kernel. */
#define MEMORY_TYPE_STACK      4       /**< Stack set up for the kernel. */
#define MEMORY_TYPE_MODULES    5       /**< Module data. */
#define MEMORY_TYPE_INTERNAL   6       /**< Freed before the OS is entered. */

// Memory allocation behaviour flags
#define MEMORY_ALLOC_HIGH      (1<<0)  // Allocate highest possible address

extern void *malloc(size_t size);
extern void *realloc(void *addr, size_t size);
extern void free(void *addr);

/**
 * Helper for __cleanup_free.
 */
static inline void freep(void *p) {
    free(*(void **)p);
}

/** Variable attribute to free the pointer when it goes out of scope. */
#define __cleanup_free          __cleanup(freep)

extern void *memory_alloc(phys_size_t size, phys_size_t align,
       phys_ptr_t min_addr, phys_ptr_t max_addr, uint8_t type, unsigned flags,
       phys_ptr_t *_phys);
extern void memory_finalize(list_t *memory_map);
extern void memory_dump(list_t *memory_map);

#ifndef PLATFORM_HAS_MM

extern void target_memory_probe(void);

extern void memory_add(phys_ptr_t start, phys_size_t size, uint8_t type);
extern void memory_protect(phys_ptr_t start, phys_size_t size);
extern void memory_init(void);

#endif /* !PLATFORM_HAS_MM */

#endif /* __MEMORY_H */
