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
 * @brief               Filesystem support.
 */

#ifndef __FS_H
#define __FS_H

#include <loader.h>

struct device;
struct fs_handle;
struct fs_mount;

/** Length of a standard UUID string (including null terminator). */
#define UUID_STR_LEN 37

/** Type of a dir_iterate() callback.
 * @param name          Name of the entry.
 * @param handle        Handle to entry.
 * @param data          Data argument passed to dir_iterate().
 * @return              Whether to continue iteration. */
typedef bool (*dir_iterate_cb_t)(const char *name, struct fs_handle *handle, void *arg);

/** Structure containing operations for a filesystem. */
typedef struct fs_ops {
    const char *name;                   /**< Name of the filesystem type. */

    /** Mount an instance of this filesystem.
     * @param device        Device to mount.
     * @param _mount        Where to store pointer to mount structure. Should be
     *                      allocated by malloc(). ops and device will be set
     *                      upon return.
     * @return              Status code describing the result of the operation.
     *                      Return STATUS_UNKNOWN_FS to indicate that the
     *                      device does not contain a filesystem of this type. */
    status_t (*mount)(struct device *device, struct fs_mount **_mount);

    /** Open a file/directory on the filesystem.
     * @note                If not provided, a generic implementation will be
     *                      used that uses iterate().
     * @note                This function will always be passed a relative path,
     *                      if fs_open() was called with an absolute path then
     *                      from will be set to mount->root.
     * @param mount         Mount to open from.
     * @param path          Path to file/directory to open (can be modified).
     * @param from          Handle on this FS to open relative to.
     * @param _handle       Where to store pointer to opened handle.
     * @return              Status code describing the result of the operation. */
    status_t (*open)(struct fs_mount *mount, char *path, struct fs_handle *from, struct fs_handle **_handle);

    /** Close a handle (optional).
     * @param handle        Handle to close. The handle will be freed after this
     *                      returns, this only needs to clear up any additional
     *                      allocated data. */
    void (*close)(struct fs_handle *handle);

    /** Read from a file.
     * @param handle        Handle to the file.
     * @param buf           Buffer to read into.
     * @param count         Number of bytes to read.
     * @param offset        Offset into the file.
     * @return              Status code describing the result of the operation. */
    status_t (*read)(struct fs_handle *handle, void *buf, size_t count, offset_t offset);

    /** Get the size of a file.
     * @param handle        Handle to the file.
     * @return              Size of the file. */
    offset_t (*size)(struct fs_handle *handle);

    /** Iterate over directory entries.
     * @param handle        Handle to directory.
     * @param cb            Callback to call on each entry.
     * @param arg           Data to pass to callback.
     * @return              Status code describing the result of the operation. */
    status_t (*iterate)(struct fs_handle *handle, dir_iterate_cb_t cb, void *arg);
} fs_ops_t;

/** Define a builtin filesystem operations structure.. */
#define BUILTIN_FS_OPS(name)   \
    static fs_ops_t name; \
    DEFINE_BUILTIN(BUILTIN_TYPE_FS, name); \
    static fs_ops_t name

/** Structure representing a mounted filesystem. */
typedef struct fs_mount {
    fs_ops_t *ops;                      /**< Operations structure for the filesystem. */
    struct device *device;              /**< Device that the filesystem is on. */
    struct fs_handle *root;             /**< Handle to root of FS (not needed if open() implemented). */
    char *label;                        /**< Label of the filesystem. */
    char *uuid;                         /**< UUID of the filesystem. */
} fs_mount_t;

/** Structure representing a handle to a filesystem entry. */
typedef struct fs_handle {
    fs_mount_t *mount;                  /**< Mount the entry is on. */
    bool directory;                     /**< Whether the entry is a directory. */
    unsigned count;                     /**< Reference count. */
} fs_handle_t;

extern void *fs_handle_alloc(size_t size, fs_mount_t *mount);
extern void fs_handle_retain(fs_handle_t *handle);
extern void fs_handle_release(fs_handle_t *handle);

/** Helper for __cleanup_fs_handle. */
static inline void fs_handle_releasep(void *p) {
    fs_handle_t *handle = *(fs_handle_t **)p;

    if (handle)
        fs_handle_releasep(handle);
}

/** Variable attribute to release a handle when it goes out of scope. */
#define __cleanup_fs_handle __cleanup(fs_handle_releasep)

extern fs_mount_t *fs_probe(struct device *device);

extern status_t fs_open(const char *path, fs_handle_t *from, fs_handle_t **_handle);

extern status_t file_read(fs_handle_t *handle, void *buf, size_t count, offset_t offset);
extern offset_t file_size(fs_handle_t *handle);

extern status_t dir_iterate(fs_handle_t *handle, dir_iterate_cb_t cb, void *arg);

#endif /* __FS_H */
