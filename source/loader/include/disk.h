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
 * @brief 					Disk device layer
 */

#ifndef __DISK_H
#define __DISK_H

#include <device.h>
#include <loader.h>

struct disk;

/** Partition map iteration callback function type.
 * @param disk		Disk containing the partition.
 * @param id		ID of the partition.
 * @param lba		Start LBA.
 * @param blocks	Size in blocks. */
typedef void (*partition_map_iterate_cb_t)(struct disk *disk, uint8_t id, uint64_t lba,
	uint64_t blocks);

/** Partition map operations. */
typedef struct partition_map_ops {
	/** Iterate over the partitions on the device.
	 * @param disk		Disk to iterate over.
	 * @param cb		Callback function.
	 * @return		Whether the device contained a partition map of
	 *			this type. */
	bool (*iterate)(struct disk *disk, partition_map_iterate_cb_t cb);
} partition_map_ops_t;

// Define a builtin partition map type
#define BUILTIN_PARTITION_MAP(name) 	\
	static partition_map_ops_t name; \
	DEFINE_BUILTIN(BUILTIN_TYPE_PARTITION_MAP, name); \
	static partition_map_ops_t name

// Operations for a disk device
typedef struct disk_ops {
	/** Check if a partition is the boot partition.
	 * @param disk		Disk the partition is on.
	 * @param id		ID of partition.
	 * @param lba		Block that the partition starts at.
	 * @return		Whether partition is a boot partition. */
	bool (*is_boot_partition)(struct disk *disk, uint8_t id, uint64_t lba);

	/** Read blocks from the disk.
	 * @param disk		Disk being read from.
	 * @param buf		Buffer to read into.
	 * @param lba		Block number to start reading from.
	 * @param count		Number of blocks to read.
	 * @return		Whether reading succeeded. */
	bool (*read)(struct disk *disk, void *buf, uint64_t lba, size_t count);
} disk_ops_t;

// Structure representing a disk device
typedef struct disk {
	device_t device;		 /**< Device header. */

	uint8_t id;			     /**< Identifier of the disk. */
	size_t block_size;	 /**< Size of one block on the disk. */
	uint64_t blocks;		 /**< Number of blocks on the disk. */
	disk_ops_t *ops;		 /**< Pointer to operations structure. */
	struct disk *parent; /**< Parent device. */
} disk_t;

extern bool disk_read(disk_t *disk, void *buf, size_t count, offset_t offset);
extern void disk_add(disk_t *disk, const char *name, uint8_t id, size_t block_size,
	uint64_t blocks, disk_ops_t *ops, bool boot);
extern disk_t *disk_parent(disk_t *disk);

extern void platform_disk_detect(void);

extern void disk_init(void);

#endif // __DISK_H
