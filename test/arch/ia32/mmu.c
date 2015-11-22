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
 * @brief               IA32 MMU code.
 */

#include <x86/mmu.h>

#include <lib/string.h>

#include <test.h>

/** Address of recursive mapping. */
static uint32_t *recursive_mapping;

/**
 * Map physical memory.
 *
 * @param virt          Virtual address to map at.
 * @param phys          Physical address to map to.
 * @param size          Size of range to map.
 */
void mmu_map(ptr_t virt, phys_ptr_t phys, size_t size) {
    ptr_t rmap_addr = (ptr_t)recursive_mapping;

    assert(!(size % PAGE_SIZE));

    while (size) {
        unsigned pde, pte;

        pde = (rmap_addr / PAGE_SIZE) + (virt / X86_PTBL_RANGE_32);
        pte = virt / PAGE_SIZE;

        if (!(recursive_mapping[pde] & X86_PTE_PRESENT))
        {
            phys_ptr_t phys = phys_alloc(PAGE_SIZE);
            recursive_mapping[pde] = phys | X86_PTE_PRESENT | X86_PTE_WRITE;
            memset(&recursive_mapping[pte & ~1023], 0, PAGE_SIZE);
        }

        recursive_mapping[pte] = phys | X86_PTE_PRESENT | X86_PTE_WRITE;

        virt += PAGE_SIZE;
        phys += PAGE_SIZE;
        size -= PAGE_SIZE;
    }
}

/**
 * Initialize the MMU code.
 *
 * @param tags          Tag list.
 */
void mmu_init(initium_tag_t *tags) {
    while (tags->type != INITIUM_TAG_NONE) {
        if (tags->type == INITIUM_TAG_PAGETABLES) {
            initium_tag_pagetables_t *tag = (initium_tag_pagetables_t *)tags;

            recursive_mapping = (uint32_t *)((ptr_t)tag->mapping);
            return;
        }

        tags = (initium_tag_t *)round_up((ptr_t)tags + tags->size, 8);
    }

    internal_error("No pagetables tag found");
}