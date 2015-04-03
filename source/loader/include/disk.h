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

#ifndef __DISK_H
#define __DISK_H

#include <device.h>

struct disk_device;

/** Types of disk devices (primarily used for naming purposes). */
typedef enum disk_type {
    DISK_TYPE_HD,                       /**< Hard drive/other. */
    DISK_TYPE_CDROM,                    /**< CDROM. */
    DISK_TYPE_FLOPPY,                   /**< Floppy drive. */
} disk_type_t;

/** Structure containing operations for a disk. */
typedef struct disk_ops {
    /**
     * Read blocks from a disk.
	 *
     * @param disk          Disk device being read from.
     * @param buf           Buffer to read into.
     * @param count         Number of blocks to read.
     * @param lba           Block number to start reading from.
     *
     * @return              Status code describing the result of the operation.
     */
    status_t (*read_blocks)(struct disk_device *disk, void *buf, size_t count, uint64_t lba);
} disk_ops_t;

/** Structure representing a disk device. */
typedef struct disk_device {
    device_t device;                    /**< Device header. */

    disk_type_t type;                   /**< Type of the disk. */
    const disk_ops_t *ops;              /**< Disk operations structure. */
    size_t block_size;                  /**< Size of a block on the disk. */
    uint64_t blocks;                    /**< Total number of blocks on the disk. */
    uint8_t id;                         /**< ID of the disk. */
} disk_device_t;

extern void disk_device_register(disk_device_t *disk);

#endif /* __DISK_H */
