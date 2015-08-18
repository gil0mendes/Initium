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

/** Read from a disk.
 * @param device        Device to read from.
 * @param buf           Buffer to read into.
 * @param count         Number of bytes to read.
 * @param offset        Offset in the disk to read from.
 * @return              Whether the read was successful. */
status_t disk_device_read(device_t *device, void *buf, size_t count, offset_t offset) {
    disk_device_t *disk = (disk_device_t *)device;
    void *block __cleanup_free = NULL;
    uint64_t start, end;
    size_t size;
    status_t ret;

    if ((uint64_t)(offset + count) > (disk->blocks * disk->block_size))
        return STATUS_END_OF_FILE;

    /* Allocate a temporary buffer for partial transfers if required. */
    if (offset % disk->block_size || count % disk->block_size)
        block = malloc(disk->block_size);

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
        if (ret != STATUS_SUCCESS)
            return ret;

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
        if (ret != STATUS_SUCCESS)
            return ret;

        buf += (size * disk->block_size);
        count -= (size * disk->block_size);
        start += size;
    }

    /* Handle anything that's left. */
    if (count > 0) {
        ret = disk->ops->read_blocks(disk, block, 1, start);
        if (ret != STATUS_SUCCESS)
            return ret;

        memcpy(buf, block, count);
    }

    return STATUS_SUCCESS;
}

/** Get disk device identification information.
 * @param device        Device to identify.
 * @param type          Type of the information to get.
 * @param buf           Where to store identification string.
 * @param size          Size of the buffer. */
static void disk_device_identify(device_t *device, device_identify_t type, char *buf, size_t size) {
    disk_device_t *disk = (disk_device_t *)device;

    if (type == DEVICE_IDENTIFY_LONG) {
        size_t ret = snprintf(buf, size,
            "block size = %zu\n"
            "blocks     = %" PRIu64 "\n",
            disk->block_size, disk->blocks);
        buf += ret;
        size -= ret;
    }

    if (disk->ops->identify)
        disk->ops->identify(disk, type, buf, size);
}

/** Disk device operations. */
static device_ops_t disk_device_ops = {
    .read = disk_device_read,
    .identify = disk_device_identify,
};

/** Read blocks from a partition.
 * @param disk          Disk being read from.
 * @param buf           Buffer to read into.
 * @param count         Number of blocks to read.
 * @param lba           Block number to start reading from.
 * @return              Status code describing the result of the operation. */
static status_t partition_read_blocks(disk_device_t *disk, void *buf, size_t count, uint64_t lba) {
    return disk->parent->ops->read_blocks(disk->parent, buf, count, lba + disk->partition.offset);
}

/** Get partition identification information.
 * @param disk          Disk to identify.
 * @param type          Type of the information to get.
 * @param buf           Where to store identification string.
 * @param size          Size of the buffer. */
static void partition_identify(disk_device_t *disk, device_identify_t type, char *buf, size_t size) {
    if (type == DEVICE_IDENTIFY_SHORT) {
        snprintf(buf, size,
            "%s partition %" PRIu8 " @ %" PRIu64,
            disk->parent->raw.partition_ops->name, disk->id, disk->partition.offset);
    }
}

/** Operations for a partition. */
static disk_ops_t partition_disk_ops = {
    .read_blocks = partition_read_blocks,
    .identify = partition_identify,
};

/** Add a partition to a disk device.
 * @param parent        Parent of the partition.
 * @param id            ID of the partition.
 * @param lba           Start LBA.
 * @param blocks        Size in blocks. */
static void add_partition(disk_device_t *parent, uint8_t id, uint64_t lba, uint64_t blocks) {
    disk_device_t *partition;
    char name[32];

    partition = malloc(sizeof(*partition));
    partition->device.type = DEVICE_TYPE_DISK;
    partition->device.ops = &disk_device_ops;
    partition->type = parent->type;
    partition->ops = &partition_disk_ops;
    partition->blocks = blocks;
    partition->block_size = parent->block_size;
    partition->id = id;
    partition->parent = parent;
    partition->partition.offset = lba;

    snprintf(name, sizeof(name), "%s,%u", parent->device.name, id);

    list_init(&partition->partition.link);
    list_append(&parent->raw.partitions, &partition->partition.link);

    device_register(&partition->device, name);

    /* Check if this is the boot partition. */
    if (boot_device == &parent->device && parent->ops->is_boot_partition) {
        if (parent->ops->is_boot_partition(parent, id, lba))
            boot_device = &partition->device;
    }
}

/** Probe a disk device for partitions.
 * @param disk          Disk device to probe. */
static void probe_partitions(disk_device_t *disk) {
    if (!disk->blocks || disk->device.mount)
        return;

    /* Check for a partition table on the device. */
    builtin_foreach(BUILTIN_TYPE_PARTITION, partition_ops_t, ops) {
        if (ops->iterate(disk, add_partition)) {
            disk->raw.partition_ops = ops;
            return;
        }
    }
}

/** Register a disk device.
 * @param disk          Disk device to register (details should be filled in).
 * @param boot          Whether the device is the boot device or contains the
 *                      boot partition. */
void disk_device_register(disk_device_t *disk, bool boot) {
    char name[16];

    list_init(&disk->raw.partitions);

    /* Assign an ID for the disk and name it. */
    disk->id = next_disk_ids[disk->type]++;
    snprintf(name, sizeof(name), "%s%u", disk_type_names[disk->type], disk->id);

    /* Add the device. */
    disk->device.type = DEVICE_TYPE_DISK;
    disk->device.ops = &disk_device_ops;
    device_register(&disk->device, name);

    if (boot)
        boot_device = &disk->device;

    probe_partitions(disk);
}
