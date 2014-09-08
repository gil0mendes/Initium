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
 * @brief		EFI memory allocation functions.
 *
 * On EFI, we don't use the generic memory management code. This is because
 * while we're still in boot services mode, the firmware is in control of the
 * memory map and we should use the memory allocation services to allocate
 * memory. Since it is possible for the memory map to change underneath us, we
 * cannot just get the memory map once during init and use it with the generic
 * MM code.
 *
 * The AllocatePages boot service cannot provide all the functionality of
 * memory_alloc() (no alignment or minimum address constraints). Therefore,
 * we implement memory_alloc() by getting the current memory map each time it
 * is called and scanning it for a suitable range, and then allocating an exact
 * range with AllocatePages. EFI allows OS-defined memory type values, we use
 * this to store our memory type.
 */

#include <lib/list.h>
#include <lib/utility.h>

#include <efi/efi.h>

#include <assert.h>
#include <loader.h>
#include <memory.h>

// EFI specifies page size as 4KB regardless of the system
#define EFI_PAGE_SIZE		0x1000

// ============================================================================
// Check whether a range can satisfy an allocation.
//
// @param range			Memory descriptor for range to check.
// @param size			Size of the allocation.
// @param align			Alignment of the allocation.
// @param min_addr	Minimum address for the start of the allocated range.
// @param max_addr	Maximum address of the end of the allocated range.
// @param flags			Behaviour flags.
// @param _phys			Where to store address for allocation.
// @return					Whether the range can satisfy the allocation.
static bool
is_suitable_range(efi_memory_descriptor_t *range, phys_size_t size, phys_size_t align,
phys_ptr_t min_addr, phys_ptr_t max_addr, unsigned flags,
efi_physical_address_t *_phys) {
	phys_ptr_t start, range_end, match_start, match_end;

	if(range->type != EFI_CONVENTIONAL_MEMORY) {
		return false;
	}

	range_end = range->physical_start + (range->num_pages * EFI_PAGE_SIZE) - 1;

	// Check if this range contains addresses in the requested range
	match_start = max(min_addr, range->physical_start);
	match_end = min(max_addr, range_end);
	if(match_end <= match_start) {
		return false;
	}

	// Align the base address and check that the range fits
	if(flags & MEMORY_ALLOC_HIGH) {
		start = round_down((match_end - size) + 1, align);

		if(start < match_start) {
			return false;
		}
	} else {
		start = round_up(match_start, align);

		if((start + size - 1) > match_end) {
			return false;
		}
	}

	*_phys = start;
	return true;
}

// ============================================================================
// Sort comparison function for the EFI memory map
static int
forward_sort_compare(const void *a, const void *b) {
	efi_memory_descriptor_t *first = (efi_memory_descriptor_t *)a;
	efi_memory_descriptor_t *second = (efi_memory_descriptor_t *)b;

	return first->physical_start - second->physical_start;
}

// ============================================================================
// Reverse sort comparison function for the EFI memory map
static int
reverse_sort_compare(const void *a, const void *b) {
	efi_memory_descriptor_t *first = (efi_memory_descriptor_t *)a;
	efi_memory_descriptor_t *second = (efi_memory_descriptor_t *)b;

	return second->physical_start - first->physical_start;
}

// ============================================================================
// Allocate a range of physical memory.
//
// @param size			Size of the range (multiple of PAGE_SIZE).
// @param align			Alignment of the range (power of 2, at least PAGE_SIZE).
// @param min_addr	Minimum address for the start of the allocated range.
// @param max_addr	Maximum address of the last byte of the allocated range.
// @param type			Type to give the allocated range.
// @param flags			Behaviour flags.
// @param _phys			Where to store physical address of allocation.
// @return					Pointer to virtual mapping of the allocation on success,
// 		NULL on failure.
void *memory_alloc(phys_size_t size, phys_size_t align, phys_ptr_t min_addr,
	phys_ptr_t max_addr, uint8_t type, unsigned flags,
	phys_ptr_t *_phys)
{
	efi_memory_descriptor_t *memory_map;
	efi_uintn_t num_entries, map_key, i;
	efi_physical_address_t start;
	efi_status_t ret;

	// If align not defined use page size
	if(!align) {
		align = PAGE_SIZE;
	}

	// Ensure that all addresses allocated are accessible to us, and avoid
	// allocating below 1MB as firmwares tend to do all sorts of funny
	// things with low memory
	if(min_addr < 0x100000) {
		min_addr = 0x100000;
	}

	// Ensure the memory allocated isn't greater than max physical address
	if(!max_addr || max_addr > LOADER_PHYS_MAX) {
		max_addr = LOADER_PHYS_MAX;
	}

	assert(!(size % PAGE_SIZE));
	assert((max_addr - min_addr) >= (size - 1));
	assert(type != MEMORY_TYPE_FREE);

	// Get the current memory map
	ret = efi_get_memory_map(&memory_map, &num_entries, &map_key);
	if(ret != EFI_SUCCESS) {
		internal_error("Failed to get memory map (0x%x)", ret);
	}

	// EFI does not specify that the memory map is sorted, so make sure it
	// is. Sort in forward or reverse order depending on whether we want
	// to allocate highest possible address first
	qsort(memory_map, num_entries, sizeof(*memory_map),
		(flags & MEMORY_ALLOC_HIGH)
			? reverse_sort_compare : forward_sort_compare);

	// Find a free range that is large enough to hold the new range
	for(i = 0; i < num_entries; i++) {
		if(!is_suitable_range(&memory_map[i], size, align, min_addr, max_addr,
			flags, &start)) {
			continue;
		}

		// Ask the firmware to allocate this exact address. The memory
		// type is stored by using the range reserved for OS vendor-
		// defined memory types
		ret = efi_call(efi_system_table->boot_services->allocate_pages,
			EFI_ALLOCATE_ADDRESS, type | EFI_OS_MEMORY_TYPE,
			size / EFI_PAGE_SIZE, &start);
		if(ret != STATUS_SUCCESS) {
			dprintf("efi: failed to allocate memory: 0x%x\n", ret);
			return NULL;
		}

		// Inform the memory range are been allocated
		dprintf("memory: allocated 0x%" PRIxPHYS "-0x%" PRIxPHYS " ("
			"align: 0x%" PRIxPHYS ", type: %u, flags: 0x%x)\n",
			start, start + size, align, type, flags);

		// Free memory map pointer
		free(memory_map);

		// Return the virtual address of the alocated memory range
		*_phys = start;
		return (void *)phys_to_virt(start);
	}

	// Free memory map pointer
	free(memory_map);
	return NULL;
}

// ============================================================================
// Finalize the memory map.
//
// This should be called once all memory allocations have been performed. It
// marks all internal memory ranges as free and returns the final memory map
// to be passed to the OS.
//
// @param memory_map	Head of list to place the memory map into.
//
void
memory_finalize(list_t *memory_map) {
	internal_error("memory_finalize: TODO");
}

// ============================================================================
// Initialize the EFI memory allocator
void
efi_memory_init(void) {
	efi_memory_descriptor_t *memory_map;
	efi_uintn_t num_entries, map_key, i;
	efi_status_t ret;

	// For informational purposes, we print out a list of all the usable
	// memory we see in the memory map. Don't print out everything, the
	// memory map is probably pretty big (e.g. OVMF under QEMU returns a
	// map with nearly 50 entries here)
	ret = efi_get_memory_map(&memory_map, &num_entries, &map_key);

	// Check if function return success
	if(ret != EFI_SUCCESS) {
		internal_error("Failed to get memory map: 0x%x\n", ret);
	}

	// Print total number of memory entries
	dprintf("efi: usable memory ranges: %d\n", num_entries);

	for(i = 0; i < num_entries; i++) {
		// Isn't a general use memory?
		if(memory_map[i].type != EFI_CONVENTIONAL_MEMORY) {
			continue;
		}

		// Print entry range (only for informational pursoses)
		dprintf(" 0x%016" PRIxPHYS "-0x%016" PRIxPHYS "\n",
			memory_map[i].physical_start,
			memory_map[i].physical_start + (memory_map[i].num_pages * EFI_PAGE_SIZE));
	}

	// Free memory map pointer
	free(memory_map);
}
