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
 * @brief               Disk device management.
 */

#include <lib/string.h>

#include <assert.h>
#include <disk.h>
#include <loader.h>
#include <memory.h>

/** Next disk IDs. */
static uint8_t next_disk_ids[DISK_TYPE_FLOPPY + 1];

/** Disk type names. */
static const char *const disk_type_names[] = {
    [DISK_TYPE_HD] = "hd",
    [DISK_TYPE_CDROM] = "cdrom",
    [DISK_TYPE_FLOPPY] = "floppy",
};

/**
 * Read from a disk.
 *
 * @param device        Device to read from.
 * @param buf           Buffer to read into.
 * @param count         Number of bytes to read.
 * @param offset        Offset in the disk to read from.
 *
 * @return              Whether the read was successful.
 */
status_t disk_device_read(device_t *device, void *buf, size_t count, offset_t offset) {
    disk_device_t *disk = (disk_device_t *)device;
    void *block;
    uint64_t start, end, size;
    status_t ret;

    if (!count)
        return STATUS_SUCCESS;

    if ((offset + count) > (disk->blocks * disk->block_size)) {
        dprintf("disk: requested read beyond end of disk (offset: %" PRId64 ", size: %zu, total: %" PRIu64 ")\n",
            offset, count, disk->blocks * disk->block_size);
        return STATUS_DEVICE_ERROR;
    }

    /* Allocate a temporary buffer for partial transfers if required. */
    block = (offset % disk->block_size || count % disk->block_size)
        ? malloc(disk->block_size)
        : NULL;

    /* Now work out the start block and the end block. Subtract one from count
     * to prevent end from going onto the next block when the offset plus the
     * count is an exact multiple of the block size. */
    start = offset / disk->block_size;
    end = (offset + (count - 1)) / disk->block_size;

    /* If we're not starting on a block boundary, we need to do a partial
     * transfer on the initial block to get up to a block boundary. If the
     * transfer only goes across one block, this will handle it. */
    if (offset % disk->block_size) {
        /* Read the block into the temporary buffer. */
        ret = disk->ops->read_blocks(disk, block, 1, start);
        if (ret != STATUS_SUCCESS) {
            free(block);
            return ret;
        }

        size = (start == end) ? count : disk->block_size - (size_t)(offset % disk->block_size);
        memcpy(buf, block + (offset % disk->block_size), size);
        buf += size;
        count -= size;
        start++;
    }

    /* Handle any full blocks. */
    size = count / disk->block_size;
    if (size) {
        ret = disk->ops->read_blocks(disk, buf, size, start);
        if (ret != STATUS_SUCCESS) {
            free(block);
            return ret;
        }

        buf += (size * disk->block_size);
        count -= (size * disk->block_size);
        start += size;
    }

    /* Handle anything that's left. */
    if (count > 0) {
        ret = disk->ops->read_blocks(disk, block, 1, start);
        if (ret != STATUS_SUCCESS) {
            free(block);
            return ret;
        }

        memcpy(buf, block, count);
    }

    free(block);
    return STATUS_SUCCESS;
}

/** Disk device operations. */
static device_ops_t disk_device_ops = {
    .read = disk_device_read,
};

/**
 * Register a disk device.
 *
 * @param disk          Disk device to register (details should be filled in).
 * */
void disk_device_register(disk_device_t *disk) {
    char name[16];

    /* Assign an ID for the disk and name it. */
    disk->id = next_disk_ids[disk->type]++;
    snprintf(name, sizeof(name), "%s%u", disk_type_names[disk->type], disk->id);

    /* Add the device. */
    disk->device.type = DEVICE_TYPE_DISK;
    disk->device.ops = &disk_device_ops;
    device_register(&disk->device, name);
}
