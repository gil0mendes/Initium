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
 * @brief 			Filesystem functions
 */

#ifndef __FS_H
#define __FS_H

#include <disk.h>
#include <loader.h>

struct mount;
struct file_handle;

/**
 * Type of a dir_iterate() callback.
 *
 * @param name		Name of the entry.
 * @param handle	Handle to entry.
 * @param data		Data argument passed to dir_iterate().
 * @return		Whether to continue iteration.
 */
typedef bool (*dir_iterate_cb_t)(const char *name, struct file_handle *handle, void *arg);

// Structure containing operations for a filesystem
typedef struct fs_type {
	/**
	 * Mount an instance of this filesystem.
	 *
	 * @param mount		Mount structure to fill in.
	 * @return		Whether succeeded in mounting.
	 */
	bool (*mount)(struct mount *mount);

	/**
	 * Open a file/directory on the filesystem.
	 *
	 * @note		If not provided, a generic implementation will
	 *			be used that uses iterate().
	 * @param mount		Mount to open from.
	 * @param path		Path to file/directory to open.
	 * @param from		Handle on this FS to open relative to.
	 * @return		Pointer to handle on success, NULL on failure.
	 */
	struct file_handle *(*open)(struct mount *mount, const char *path, struct file_handle *from);

	/**
	 * Close a handle.
	 *
	 * @param handle	Handle to close.
	*/
	void (*close)(struct file_handle *handle);

	/**
	 * Read from a file.
	 *
	 * @param handle	Handle to the file.
	 * @param buf		Buffer to read into.
	 * @param count		Number of bytes to read.
	 * @param offset	Offset into the file.
	 * @return		Whether read successfully.
	 */
	bool (*read)(struct file_handle *handle, void *buf, size_t count, offset_t offset);

	/**
	 * Get the size of a file.
	 *
	 * @param handle	Handle to the file.
	 * @return		Size of the file.
	 */
	offset_t (*size)(struct file_handle *handle);

	/**
	 * Iterate over directory entries.
	 *
	 * @param handle	Handle to directory.
	 * @param cb		Callback to call on each entry.
	 * @param arg		Data to pass to callback.
	 * @return		Whether read successfully.
	 */
	bool (*iterate)(struct file_handle *handle, dir_iterate_cb_t cb, void *arg);
} fs_type_t;

// Define a builtin filesystem type
#define BUILTIN_FS_TYPE(name) 	\
	static fs_type_t name; \
	DEFINE_BUILTIN(BUILTIN_TYPE_FS, name); \
	static fs_type_t name

// Structure representing a mounted filesystem
typedef struct mount {
	fs_type_t *type;						// Type structure for the filesystem
	struct file_handle *root;		// Handle to root of FS (not needed if open() implemented)
	void *data;									// Implementation-specific data pointer
	//disk_t *disk;								// Disk that the filesystem resides on
	char *label;								// Label of the filesystem
	char *uuid;									// UUID of the filesystem
} mount_t;

// Structure representing a handle to a filesystem entry
typedef struct file_handle {
	mount_t *mount;						// Mount the entry is on
	bool directory;						// Whether the entry is a directory
	void *data;								// Implementation-specific data pointer
	int count;								// Reference count
	#if CONFIG_INITIUM_FS_ZLIB
	void *compressed;					// If the file is compressed, pointer to decompress data
	#endif
} file_handle_t;

extern file_handle_t *file_handle_create(mount_t *mount, bool directory, void *data);

//extern mount_t *fs_probe(disk_t *disk);

extern file_handle_t *file_open(const char *path, file_handle_t *from);
extern void file_close(file_handle_t *handle);
extern bool file_read(file_handle_t *handle, void *buf, size_t count, offset_t offset);
extern offset_t file_size(file_handle_t *handle);

extern bool dir_iterate(file_handle_t *handle, dir_iterate_cb_t cb, void *arg);

#endif /* __FS_H */
