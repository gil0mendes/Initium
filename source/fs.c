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
 * @brief               Filesystem support.
 */

#include <lib/string.h>

#include <assert.h>
#include <config.h>
#include <device.h>
#include <fs.h>
#include <memory.h>

/** Probe a device for filesystems.
 * @param device        Device to probe.
 * @return              Pointer to mount if found, NULL if not. */
fs_mount_t *fs_probe(device_t *device) {
    builtin_foreach(BUILTIN_TYPE_FS, fs_ops_t, ops) {
        fs_mount_t *mount;
        status_t ret;

        ret = ops->mount(device, &mount);
        switch (ret) {
        case STATUS_SUCCESS:
            dprintf("fs: mounted %s on %s ('%s') (uuid: %s)\n", ops->name, device->name, mount->label, mount->uuid);

            mount->ops = ops;
            mount->device = device;
            return mount;
        case STATUS_UNKNOWN_FS:
        case STATUS_END_OF_FILE:
            /* End of file usually means no media. */
            break;
        default:
            dprintf("fs: error %d while probing device %s\n", ret, device->name);
            return NULL;
        }
    }

    return NULL;
}

/**
 * Open a handle to a directory entry.
 *
 * Opens a handle given an entry structure provided by fs_iterate(). This is
 * only valid on entry structures provided by that function, as the structure
 * is typically embedded inside some FS-specific structure which contains the
 * information needed to open the file.
 *
 * @param entry         Entry to open.
 * @param _handle       Where to store pointer to opened handle.
 *
 * @return              Status code describing the result of the operation.
 */
status_t fs_open_entry(const fs_entry_t *entry, fs_handle_t **_handle) {
    if (!entry->owner->mount->ops->open_entry)
        return STATUS_NOT_SUPPORTED;

    return entry->owner->mount->ops->open_entry(entry, _handle);
}

/** Structure containing data for fs_open(). */
typedef struct fs_open_data {
    const char *name;                   /**< Name of entry being searched for. */
    status_t ret;                       /**< Result of opening the entry. */
    fs_handle_t *handle;                /**< Handle to found entry. */
} fs_open_data_t;

/** Directory iteration callback for fs_open().
 * @param entry         Entry that was found.
 * @param _data         Pointer to data structure.
 * @return              Whether to continue iteration. */
static bool fs_open_cb(const fs_entry_t *entry, void *_data) {
    fs_open_data_t *data = _data;
    int result;

    result = (entry->owner->mount->case_insensitive)
        ? strcasecmp(entry->name, data->name)
        : strcmp(entry->name, data->name);

    if (!result) {
        data->ret = fs_open_entry(entry, &data->handle);
        return false;
    } else {
        return true;
    }
}

/**
 * Open a handle to a file/directory.
 *
 * Looks up a path and returns a handle to it. If a source handle is given and
 * the path is a relative path, the path will be looked up relative to that
 * directory. If no source is given then relative paths will not be allowed.
 *
 * An absolute path either begins with a '/' character, or a device specifier
 * in the form "(<device name>)" followed by a '/'. If no device specifier is
 * included, the lookup will take place from the root of the current device.
 *
 * @param path          Path to entry to open.
 * @param from          If not NULL, a directory to look up relative to.
 *
 * @return              Pointer to handle on success, NULL on failure.
 */
status_t fs_open(const char *path, fs_handle_t *from, fs_handle_t **_handle) {
    char *orig __cleanup_free;
    char *dup, *tok;
    device_t *device;
    fs_mount_t *mount;
    fs_handle_t *handle;
    status_t ret;

    /* Duplicate the path string so we can modify it. */
    dup = orig = strdup(path);

    if (dup[0] == '(') {
        dup++;
        tok = strsep(&dup, ")");
        if (!tok || !tok[0] || dup[0] != '/')
            return STATUS_INVALID_ARG;

        device = device_lookup(tok);
        if (!device || !device->mount)
            return STATUS_NOT_FOUND;

        mount = device->mount;
    } else if (from) {
        mount = from->mount;
    } else {
        device = (current_environ) ? current_environ->device : boot_device;
        if (!device || !device->mount)
            return STATUS_NOT_FOUND;

        mount = device->mount;
    }

    if (dup[0] == '/') {
        from = mount->root;

        /* Strip leading / characters from the path. */
        while (*dup == '/')
            dup++;
    } else if (!from) {
        return STATUS_INVALID_ARG;
    }

    /* If an open_path() implementation is provided, use it. */
    if (mount->ops->open_path)
        return mount->ops->open_path(mount, dup, from, _handle);

    assert(mount->ops->iterate);
    assert(mount->ops->open_entry);

    handle = from;

    /* Loop through each element of the path string. */
    while (true) {
        fs_open_data_t data;

        tok = strsep(&dup, "/");
        if (!tok) {
            /* The last token was the last element of the path string,
             * return the handle we're currently on. */
            break;
        } else if (!handle->directory) {
            /* The previous node was not a directory: this means the path
             * string is trying to treat a non-directory as a directory.
             * Reject this. */
            if (handle != from)
                fs_close(handle);
            return STATUS_NOT_DIR;
        } else if (!tok[0] || (tok[0] == '.' && !tok[1])) {
            /* Zero-length path component or current directory, do nothing. */
            continue;
        }

        /* Search the directory for the entry. */
        data.name = tok;
        data.ret = STATUS_NOT_FOUND;
        ret = mount->ops->iterate(handle, fs_open_cb, &data);

        if (handle != from)
            fs_close(handle);

        if (ret == STATUS_SUCCESS)
            ret = data.ret;
        if (ret != STATUS_SUCCESS)
            return ret;

        handle = data.handle;
    }

    *_handle = handle;
    return STATUS_SUCCESS;
}

/** Close a filesystem handle.
 * @param handle        Handle to close. */
void fs_close(fs_handle_t *handle) {
    if (handle->mount->ops->close)
        handle->mount->ops->close(handle);

    free(handle);
}

/** Read from a file.
 * @param handle        Handle to the file.
 * @param buf           Buffer to read into.
 * @param count         Number of bytes to read.
 * @param offset        Offset into the file.
 * @return              Status code describing the result of the operation. */
status_t fs_read(fs_handle_t *handle, void *buf, size_t count, offset_t offset) {
    if (handle->directory)
        return STATUS_NOT_FILE;

    if (offset + count > handle->size)
        return STATUS_END_OF_FILE;

    if (!count)
        return STATUS_SUCCESS;

    return handle->mount->ops->read(handle, buf, count, offset);
}

/** Iterate over entries in a directory.
 * @param handle        Handle to directory.
 * @param cb            Callback to call on each entry.
 * @param arg           Data to pass to callback.
 * @return              Status code describing the result of the operation. */
status_t fs_iterate(fs_handle_t *handle, fs_iterate_cb_t cb, void *arg) {
    if (!handle->directory) {
        return STATUS_NOT_DIR;
    } else if (!handle->mount->ops->iterate) {
        return STATUS_NOT_SUPPORTED;
    }

    return handle->mount->ops->iterate(handle, cb, arg);
}
