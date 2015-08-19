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
 * @brief               MMU functions.
 */

#ifndef __MMU_H
#define __MMU_H

#include <loader.h>

/** Type of an MMU context. */
typedef struct mmu_context mmu_context_t;

extern bool mmu_map(mmu_context_t *ctx, load_ptr_t virt, phys_ptr_t phys, load_size_t size);

extern bool mmu_memset(mmu_context_t *ctx, load_ptr_t addr, uint8_t value, load_size_t size);
extern bool mmu_memcpy_to(mmu_context_t *ctx, load_ptr_t dest, const void *src, load_size_t size);
extern bool mmu_memcpy_from(mmu_context_t *ctx, void *dest, load_ptr_t src, load_size_t size);

extern mmu_context_t *mmu_context_create(load_mode_t mode, unsigned phys_type);

#endif /* __MMU_H */
