/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Gil Mendes <gil00mendes@gmail.com>
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
 * @brief               EFI platform Initium loader functions.
 */

#include <efi/efi.h>

#include <lib/string.h>

#include <loader/initium.h>

#include <assert.h>
#include <memory.h>

#ifdef __LP64__
#   define INITIUM_EFI_TYPE   INITIUM_EFI_64
#else
#   define INITIUM_EFI_TYPE   INITIUM_EFI_32
#endif

/**
 * Perform platform-specific setup for Initium kernel.
 *
 * @param loader    Loader internal data.
 */
void initium_platform_setup(initium_loader_t *loader) {
  void *memory_map __cleanup_free;
  efi_uintn_t num_entries, desc_size, size;
  efi_uint32_t desc_version;
  initium_tag_efi_t *tag;

  // exit boot services mode and get final memory map.
  efi_exit_boot_services(&memory_map, &num_entries, &desc_size, &desc_version);


  // pass the memory map to the kernel
  size = num_entries * desc_size;
  tag = initium_alloc_tag(loader, INITIUM_TAG_EFI, sizeof(*tag) + size);
  tag->type = INITIUM_EFI_TYPE;
  tag->system_table = virt_to_phys((ptr_t) efi_system_table);
  tag->num_memory_descs = num_entries;
  tag->memory_desc_size = desc_size;
  tag->memory_desc_version = desc_version;
  memcpy(tag->memory_map, memory_map, size);
}
