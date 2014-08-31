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
* @brief                 Filesystem functions
*/

#include <lib/string.h>
#include <lib/utility.h>

#include <assert.h>
#include <config.h>
#include <fs.h>
#include <memory.h>

#if CONFIG_LAOS_FS_LIB
#include <fs/decompress.h>
#endif

/**
 * Create a file handle.
 *
 * @param mount 			Mount the entry resides on
 * @param directory 	Whether the entry is a directory
 * @param data				Implementation-specific data pointer
 * @return 						Pointer to handle structure
 */
file_handle_t *file_handle_create(mount_t *mount, bool directory, void *data) {
	file_handle_t *handle = malloc(sizeof(file_handle_t));
	handle->mount = mount;
	handle->directory = directory;
	handle->data = data;
	handle->count = 1;
	#if CONFIG_KBOOT_FS_ZLIB
	handle->compressed = NULL;
	#endif
	return handle;
}

/**
 * Probe a disk for filesystems.
 *
 * @param disk		Disk to probe.
 * @return				Pointer to mount if detected, NULL if not.
*/
mount_t *fs_probe(disk_t *disk) {
	mount_t *mount;

	mount = malloc(sizeof(mount_t));

	BUILTIN_ITERATE(BUILTIN_TYPE_FS, fs_type_t, type) {
		memset(mount, 0, sizeof(mount_t));
		mount->disk = disk;
		mount->type = type;
		if(mount->type->mount(mount)) {
			return mount;
		}
	}

	free(mount);
	return NULL;
}

// Structure containing data for file_open().
typedef struct file_open_data {
	const char *name;					// Name of entry being searched for
	file_handle_t *handle;		// Handle to found entry
} file_open_data_t;

/**
 * Directory iteration callback for file_open().
 *
 * @param name		Name of entry.
 * @param handle	Handle to entry.
 * @param _data		Pointer to data structure.
 * @return				Whether to continue iteration. */
static bool file_open_cb(const char *name, file_handle_t *handle, void *_data) {
	file_open_data_t *data = _data;

	if(strcmp(name, data->name) == 0) {
		handle->count++;
		data->handle = handle;
		return false;
	} else {
		return true;
	}
}

/**
 * Open a handle to a file/directory.
 *
 * Looks up a path and returns a handle to it. If a source node is given, the
 * path will be looked up relative to that directory, on the device of that
 * directory. Otherwise, relative paths will not be allowed and the lookup will
 * take place on the current device.
 *
 * @param path		Path to entry to open.
 * @param from		If not NULL, a directory to look up relative to.
 *
 * @return		Pointer to handle on success, NULL on failure.
 */
file_handle_t *file_open(const char *path, file_handle_t *from) {
	char *dup, *orig, *tok;
	file_open_data_t data;
	file_handle_t *handle;
	mount_t *mount;

	if(from) {
		assert(from->directory);
		mount = from->mount;
		handle = (path[0] == '/') ? mount->root : from;
	} else {
		if(!current_device || !(mount = current_device->fs)) {
			return NULL;
		}

		handle = mount->root;
	}

	// Use the provided open() implementation if any
	if(mount->type->open) {
		handle = mount->type->open(mount, path, from);
	} else {
		assert(mount->type->iterate);

		// Strip leading / characters from the path
		while(*path == '/')
			path++;

		assert(handle);
		handle->count++;

		// Loop through each element of the path string. The string must be
		// duplicated so that it can be modified.
		dup = orig = strdup(path);
		while(true) {
			tok = strsep(&dup, "/");
			if(tok == NULL) {
				// The last token was the last element of the path
				// string, return the node we're currently on.
				free(orig);
				break;
			} else if(!handle->directory) {
				// The previous node was not a directory: this means
				// the path string is trying to treat a non-directory
				// as a directory. Reject this
				file_close(handle);
				free(orig);
				return NULL;
			} else if(!tok[0]) {
				// Zero-length path component, do nothing
				continue;
			}

			// Search the directory for the entry
			data.name = tok;
			data.handle = NULL;
			if(!mount->type->iterate(handle, file_open_cb, &data) || !data.handle) {
				file_close(handle);
				free(orig);
				return NULL;
			}

			file_close(handle);
			handle = data.handle;
		}
	}

	#if CONFIG_LAOS_FS_ZLIB
	// If the file is compressed, initialize decompression state. This will
	// set handle->compressed to non-NULL if the file is compressed
	if(!handle->directory)
		decompress_open(handle);
	#endif

	return handle;
}

/**
 * Close a handle.
 *
 * @param handle	Handle to close.
 */
void file_close(file_handle_t *handle) {
	if(--handle->count == 0) {
		#if CONFIG_KBOOT_FS_ZLIB
		if(handle->compressed)
			decompress_close(handle);
		#endif

		if(handle->mount->type->close)
			handle->mount->type->close(handle);

		free(handle);
	}
}

/**
 * Read from a file.
 *
 * @param handle	Handle to file to read from.
 * @param buf			Buffer to read into.
 * @param count		Number of bytes to read.
 * @param offset	Offset in the file to read from.
 * @return				Whether the read was successful.
 */
bool file_read(file_handle_t *handle, void *buf, size_t count, offset_t offset) {
	assert(!handle->directory);

	if(!count)
		return true;

	#if CONFIG_LAOS_FS_ZLIB
	if(handle->compressed)
		return decompress_read(handle, buf, count, offset);
	#endif

	return handle->mount->type->read(handle, buf, count, offset);
}

/**
 * Get the size of a file.
 *
 * @param handle	Handle to the file.
 * @return		Size of the file.
 */
offset_t file_size(file_handle_t *handle) {
	assert(!handle->directory);

	#if CONFIG_LAOS_FS_ZLIB
	if(handle->compressed)
		return decompress_size(handle);
	#endif

	return handle->mount->type->size(handle);
}

/**
 * Iterate over entries in a directory.
 *
 * @param handle	Handle to directory.
 * @param cb			Callback to call on each entry.
 * @param arg			Data to pass to callback.
 * @return				Whether read successfully.
*/
bool dir_iterate(file_handle_t *handle, dir_iterate_cb_t cb, void *arg) {
	assert(handle->directory);
	return handle->mount->type->iterate(handle, cb, arg);
}
