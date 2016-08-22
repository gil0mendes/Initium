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
#include <console.h>

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
  efi_status_t ret;

  /* Try multiple times to call ExitBootServices, it can change the memory map
   * the first time. This should not happen more than once however, so only
   * do it twice.
   */
  for (unsigned i = 0; i < 2; i++) {
    efi_uintn_t size, map_key, desc_size;
    efi_uint32_t desc_version;
    void *buf __cleanup_free = NULL;

    /** Call a first time to get the needed buffer size. */
    size = 0;
    ret = efi_call(efi_boot_services->get_memory_map, &size, NULL, &map_key, &desc_size, &desc_version);
    if (ret != EFI_BUFFER_TOO_SMALL) {
      internal_error("Failed to get memory map size (0x%zx)", ret);
    }

    buf = malloc(size);

    ret = efi_call(efi_boot_services->get_memory_map, &size, buf, &map_key, &desc_size, &desc_version);
    if (ret != EFI_SUCCESS) {
      internal_error("Failed to get memory map (0xzx)", ret);
    }

    /* Try to exit boot services. */
    ret = efi_call(efi_boot_services->exit_boot_services, efi_image_handle, map_key);
    if (ret == EFI_SUCCESS) {
      initium_tag_efi_t *tag;

      /* Disable the console, i could now be invalid. */
      console_set_debug(NULL);

      tag = initium_alloc_tag(loader, INITIUM_TAG_EFI, sizeof(*tag) + size);
      tag->type = INITIUM_EFI_TYPE;
      tag->system_table = virt_to_phys((ptr_t)efi_system_table);
      tag->num_memory_descs = size / desc_size;
      tag->memory_desc_size = desc_size;
      tag->memory_desc_version = desc_version;
      memcpy(tag->memory_map, buf, size);

      return;
    }

    internal_error("Failed to exit boot services (0x%zx)", ret);
  }
}
