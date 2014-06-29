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
 * @brief		EFI platform core definitions.
 */

#ifndef __EFI_EFI_H
#define __EFI_EFI_H

#include <efi/arch/efi.h>

#include <efi/api.h>

extern efi_handle_t efi_image_handle;
extern efi_system_table_t *efi_system_table;

extern efi_status_t efi_locate_handle(efi_locate_search_type_t search_type,
       efi_guid_t *protocol, void *search_key, efi_handle_t **_handles,
       efi_uintn_t *_num_handles);
extern efi_status_t efi_open_protocol(efi_handle_t handle, efi_guid_t *protocol,
        efi_uint32_t attributes, void **_interface);
extern efi_status_t efi_get_memory_map(efi_memory_descriptor_t **_memory_map,
       efi_uintn_t *_num_entries, efi_uintn_t *_map_key);

extern void efi_console_init(void);
extern void efi_memory_init(void);

#endif /* __EFI_EFI_H */
