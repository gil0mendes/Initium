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
 * @brief               EFI platform core definitions.
 */

#ifndef __EFI_EFI_H
#define __EFI_EFI_H

#include <efi/arch/efi.h>

#include <efi/api.h>

#include <status.h>

extern efi_handle_t efi_image_handle;
extern efi_loaded_image_t *efi_loaded_image;
extern efi_system_table_t *efi_system_table;

extern status_t efi_convert_status(efi_status_t status);

extern efi_status_t efi_allocate_pool(efi_memory_type_t pool_type, efi_uintn_t size, void **_buffer);
extern efi_status_t efi_free_pool(void *buffer);

extern efi_status_t efi_locate_handle(
    efi_locate_search_type_t search_type, efi_guid_t *protocol, void *search_key,
    efi_handle_t **_handles, efi_uintn_t *_num_handles);
extern efi_status_t efi_open_protocol(
    efi_handle_t handle, efi_guid_t *protocol, efi_uint32_t attributes,
    void **_interface);

extern efi_device_path_t *efi_get_device_path(efi_handle_t handle);
extern void efi_print_device_path(efi_device_path_t *path, void (*cb)(void *data, char ch), void *data);
extern bool efi_is_child_device_node(efi_device_path_t *parent, efi_device_path_t *child);

/** Get the next device path node in a device path.
 * @param path          Current path node.
 * @return              Next path node, or NULL if end of list. */
static inline efi_device_path_t *efi_next_device_node(efi_device_path_t *path) {
    path = (efi_device_path_t *)((char *)path + path->length);
    return (path->type != EFI_DEVICE_PATH_TYPE_END) ? path : NULL;
}

/** Get the last device path node in a device path.
 * @param path          Current path node.
 * @return              Last path node. */
static inline efi_device_path_t *efi_last_device_node(efi_device_path_t *path) {
    for (efi_device_path_t *next = path; next; path = next, next = efi_next_device_node(path))
        ;

    return path;
}

extern void efi_console_init(void);
extern void efi_disk_init(void);
extern void efi_memory_init(void);
extern void efi_video_init(void);

extern efi_status_t efi_init(efi_handle_t image_handle, efi_system_table_t *system_table);

#endif /* __EFI_EFI_H */
