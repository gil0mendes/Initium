/**
* The MIT License (MIT)
*
* Copyright (c) 2014 Gil Mendes <gil00mendes@gmail.com>
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
* Multiboot support
*/

#include <lib/list.h>
#include <lib/string.h>
#include <lib/utility.h>

#include <bios/multiboot.h>

#include <fs.h>
#include <config.h>
#include <device.h>
#include <memory.h>

// Multiboot FS file information
typedef struct multiboot_file {
    list_t header;          // Link to files list

    void *addr;             // Module data address
    size_t size;            // Module data size
    char *name;             // Module name
} multiboot_file_t;

// List of file loaded by Multiboot
static LIST_DECLARE(multiboot_files);

/**
* Read from a file.
*
* @param handle	Handle to the file.
* @param buf		Buffer to read into.
* @param count		Number of bytes to read.
* @param offset	Offset into the file.
*
* @return		Whether read successfully.
*/
static bool
multiboot_read(file_handle_t *handle, void *buf, size_t count, offset_t offset)
{
    multiboot_file_t *file = handle->data;

    if(offset >= (offset_t)file->size) {
        return false;
    } else if((offset + count) > (offset_t)file->size) {
        return false;
    }

    memcpy(buf, file->addr + (size_t)offset, count);
    return true;
}

/**
* Get the size of a file.
*
* @param handle	Handle to the file.
*
* @return		Size of the file.
*/
static offset_t
multiboot_size(file_handle_t *handle)
{
    multiboot_file_t *file = handle->data;
    return file->size;
}

/**
* Read directory entries.
*
* @param handle	Handle to directory.
* @param cb		Callback to call on each entry.
* @param arg		Data to pass to callback.
*
* @return		Whether read successfully.
*/
static bool
multiboot_iterate(file_handle_t *handle, dir_iterate_cb_t cb, void *arg)
{
    multiboot_file_t *file;
    file_handle_t *child;
    bool ret;

    list_foreach(&multiboot_files, iter) {
        file = list_entry(iter, multiboot_file_t, header);

        child = file_handle_create(handle->mount, false, file);
        ret = cb(file->name, child, arg);
        file_close(child);
        if(!ret) {
            break;
        }
    }

    return true;
}

/**
* Multiboot filesystem type.
*/
static fs_type_t multiboot_fs_type = {
        .read = multiboot_read,
        .size = multiboot_size,
        .iterate = multiboot_iterate,
};

/**
* Perform initialization required when booting from Multiboot.
*/
void multiboot_init(void)
{
    multiboot_module_t *modules;
    multiboot_file_t *file;
    char *tok, *cmdline;
    device_t *device;
    phys_ptr_t addr;
    mount_t *mount;
    uint32_t i;

    if (multiboot_magic != MB_LOADER_MAGIC) {
        return;
    }

    // Parse command line options
    cmdline = (char *)multiboot_info.cmdline;
    while((tok = strsep(&cmdline, " "))) {
        if (strncmp(tok, "config-file=", 12) == 0) {
            config_file_override = tok + 12;
        }
    }

    // If we were given modules, they form the boot filesystem
    if (multiboot_info.mods_count) {
        dprintf("loader: using Multiboot modules as boot FS (addr: %p, count: %u)\n",
                multiboot_info.mods_addr, multiboot_info.mods_count);

        modules = (multiboot_module_t *)multiboot_info.mods_addr;
        for(i = 0; i < multiboot_info.mods_count; i++) {
            if(!modules[i].cmdline) {
                continue;
            }

            file = malloc(sizeof(multiboot_file_t));
            list_init(&file->header);
            file->size = modules[i].mod_end - modules[i].mod_start;

            // We only want the base name, take off any path strings
            file->name = strrchr((char *)modules[i].cmdline, '/');
            file->name = (file->name) ? file->name + 1 : (char *)modules[i].cmdline;

            // We now need to re-allocate the module data as high
            // as possible so as to make it unlikely that we will
            // conflict with fixed load address for a kernel
            memory_alloc(round_up(file->size, PAGE_SIZE),
                    0, 0, 0x100000000LL, MEMORY_TYPE_INTERNAL,
                    MEMORY_ALLOC_HIGH, &addr);
            file->addr = (void *)((ptr_t)addr);
            memcpy(file->addr, (void *)modules[i].mod_start, file->size);

            list_append(&multiboot_files, &file->header);
        }

        // Create a filesystem
        mount = malloc(sizeof(mount_t));
        mount->type = &multiboot_fs_type;
        mount->root = file_handle_create(mount, true, NULL);
        mount->label = strdup("Multiboot");
        mount->uuid = NULL;
        device = malloc(sizeof(device_t));
        device_add(device, "multiboot", DEVICE_TYPE_IMAGE);
        device->fs = mount;
    }
}