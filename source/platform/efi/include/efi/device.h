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
 * @brief               EFI device utility functions.
 */

 #ifndef __EFI_DEVICE_H
 #define __EFI_DEVICE_H

 #include <efi/api.h>

struct device;

extern efi_device_path_t *efi_get_device_path(efi_handle_t handle);
extern void efi_print_device_path(efi_device_path_t *path, void (*cb)(void *data, char ch), void *data);
extern bool efi_is_child_device_node(efi_device_path_t *parent, efi_device_path_t *child);

/** Get the next device path node in a device path.
 * @param path          Current path node.
 * @return              Next path node, or NULL if end of list. */
static inline efi_device_path_t *efi_next_device_node(efi_device_path_t *path)
{
	path = (efi_device_path_t*)((char*)path + path->length);
	return (path->type != EFI_DEVICE_PATH_TYPE_END) ? path : NULL;
}

/** Get the last device path node in a device path.
 * @param path          Current path node.
 * @return              Last path node. */
static inline efi_device_path_t *efi_last_device_node(efi_device_path_t *path)
{
	for (efi_device_path_t *next = path; next; path = next, next = efi_next_device_node(path))
		;

	return path;
}

extern efi_handle_t efi_device_get_handle(struct device *device);

 #endif /* __EFI_DEVICE_H */
