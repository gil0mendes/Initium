/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Gil Mendes <gil00mendes@gmail.com>
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
 * @brief               BIOS Multiboot loader functions.
 *
 * TODO:
 *  - Video mode setting/VBE info.
 *  - APM/config table/drive info.
 */

#include <bios/bios.h>
#include <bios/disk.h>
#include <bios/memory.h>

#include <lib/utility.h>

#include <x86/multiboot.h>

#include <assert.h>
#include <loader.h>

/** Get platform-specific Multiboot information.
 * @param loader        Loader internal data. */
void multiboot_platform_load(multiboot_loader_t *loader)
{
	bios_regs_t regs;
	uint32_t num_entries = 0, entry_size = 0;
	void *mmap = (void*)BIOS_MEM_BASE;

	bios_regs_init(&regs);

	/* Get the E820 memory map. */
	do {
		regs.eax = 0xe820;
		regs.edx = E820_SMAP;
		regs.ecx = 64;
		regs.edi = (ptr_t)mmap + (num_entries * entry_size);
		bios_call(0x15, &regs);

		if (regs.eflags & X86_FLAGS_CF)
			break;

		if (!num_entries) {
			entry_size = regs.ecx;
		} else {
			assert(entry_size == regs.ecx);
		}

		num_entries++;
	} while (regs.ebx != 0);

	if (num_entries) {
		/* Each entry has a size field prepended to it. */
		size_t size = num_entries * (entry_size + 4);
		void *dest = multiboot_alloc_info(loader, size, &loader->info->mmap_addr);

		loader->info->flags |= MULTIBOOT_INFO_MEMORY | MULTIBOOT_INFO_MEM_MAP;
		loader->info->mmap_length = size;

		for (uint32_t i = 0; i < num_entries; i++) {
			e820_entry_t *entry = mmap;

			/* Get upper/lower memory information. */
			if (entry->type == E820_TYPE_FREE) {
				if (entry->start <= 0x100000 && entry->start + entry->length > 0x100000) {
					loader->info->mem_upper = (entry->start + entry->length - 0x100000) / 1024;
				} else if (entry->start == 0) {
					loader->info->mem_lower = min(entry->length, 0x100000) / 1024;
				}
			}

			/* Fill in the memory map entry, with the size field beforehand.
			 * Copy the whole size returned, in case the BIOS has returned some
			 * fields that we don't know about. */
			*(uint32_t*)dest = entry_size;
			memcpy(dest + 4, mmap, entry_size);

			dest += entry_size + 4;
			mmap += entry_size;
		}
	}

	/* Try to get the boot device ID. */
	if (current_environ->device->type == DEVICE_TYPE_DISK) {
		disk_device_t *disk = (disk_device_t*)current_environ->device;

		loader->info->boot_device = (uint32_t)bios_disk_get_id(disk) << 24;
		if (disk_device_is_partition(disk))
			loader->info->boot_device |= (uint32_t)disk->id << 16;

		loader->info->flags |= MULTIBOOT_INFO_BOOTDEV;
	}
}
