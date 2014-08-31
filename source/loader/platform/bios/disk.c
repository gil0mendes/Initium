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
* @brief 							BIOS disk device functions
*/

#include <lib/list.h>
#include <lib/string.h>
#include <lib/utility.h>

#include <bios/bios.h>
#include <bios/disk.h>

#include <disk.h>
#include <assert.h>
#include <loader.h>
#include <memory.h>

// Maximum number of blocks per transfer
#define BLOCKS_PER_TRANSFER(disk) ((BIOS_MEM_SIZE / disk->block_size) - 1)

extern uint8_t 	boot_device_id;
extern uint64_t	boot_part_offset;

// ============================================================================
// Check if a partition is the boot partition.
//
// @param disk		Disk the partition resides on.
// @param id			ID of partition.
// @param lba			Block that the partition starts at.
// @return				Whether partition is a boot partition.
//
static bool bios_disk_is_boot_partition(disk_t *disk, uint8_t id, uint64_t lba) {
	if (lba == boot_part_offset) {
		return true;
	}

	return false;
}

// ============================================================================
// Read blocks from a BIOS disk device.
//
// @param disk		Disk to read from.
// @param buf			Buffer to read into.
// @param lba			Starting block number.
// @param count		Number of blocks to read.
// @return				Whether the read succeeded.
//
static bool bios_disk_read(disk_t *disk, void *buf, uint64_t lba, size_t count) {
	disk_address_packet_t *dap = (disk_address_packet_t *)BIOS_MEM_BASE;
	void *dest = (void *)(BIOS_MEM_BASE + disk->block_size);
	size_t i, num, transfers;
	bios_regs_t regs;

	// Have to split large transfers up as we have limited space to
	// transfer to.
	transfers = round_up(count, BLOCKS_PER_TRANSFER(disk)) / BLOCKS_PER_TRANSFER(disk);
	for(i = 0; i < transfers; i++, buf += disk->block_size * num, count -= num) {
		num = min(count, BLOCKS_PER_TRANSFER(disk));

		// Fill in a disk address packet for the transfer
		dap->size = sizeof(disk_address_packet_t);
		dap->reserved1 = 0;
		dap->block_count = num;
		dap->buffer_offset = (ptr_t)dest;
		dap->buffer_segment = 0;
		dap->start_lba = lba + (i * BLOCKS_PER_TRANSFER(disk));

		// Perform the transfer
		bios_regs_init(&regs);
		regs.eax = INT13_EXT_READ;
		regs.edx = disk->id;
		regs.esi = BIOS_MEM_BASE;
		bios_call(0x13, &regs);
		if(regs.eflags & X86_FLAGS_CF) {
			return false;
		}

		// Copy the transferred blocks to the buffer
		memcpy(buf, dest, disk->block_size * num);
	}

	return true;
}

// Operations for a BIOS disk device
static disk_ops_t bios_disk_ops = {
	.is_boot_partition = bios_disk_is_boot_partition,
	.read = bios_disk_read,
};

// ============================================================================
// Get the number of disks in the system.
//
// @return		Number of BIOS hard disks.
//
static uint8_t get_disk_count(void) {
	bios_regs_t regs;

	// Use the Get Drive Parameters call
	bios_regs_init(&regs);
	regs.eax = INT13_GET_DRIVE_PARAMETERS;
	regs.edx = 0x80;
	bios_call(0x13, &regs);

	return (regs.eflags & X86_FLAGS_CF) ? 0 : (regs.edx & 0xFF);
}

// ============================================================================
// Check if booted from CD.
//
// @return		Whether booted from CD.
//
static bool booted_from_cd(void) {
	specification_packet_t *packet = (specification_packet_t *)BIOS_MEM_BASE;
	bios_regs_t regs;

	// Use the bootable CD-ROM status function
	bios_regs_init(&regs);
	regs.eax = INT13_CDROM_GET_STATUS;
	regs.edx = boot_device_id;
	regs.esi = BIOS_MEM_BASE;
	bios_call(0x13, &regs);

	return (!(regs.eflags & X86_FLAGS_CF) && packet->drive_number == boot_device_id);
}

// ============================================================================
// Add the disk with the specified ID.
//
// @param id		ID of the device.
//
static void add_disk(uint8_t id) {
	drive_parameters_t *params = (drive_parameters_t *)BIOS_MEM_BASE;
	bios_regs_t regs;
	disk_t *disk;
	char name[6];

	// Create a data structure for the device
	disk = malloc(sizeof(disk_t));

	// Probe for information on the device. A big "F*CK YOU" to Intel and
	// AMI is required here. When booted from a CD, the INT 13 Extensions
	// Installation Check/Get Drive Parameters functions return an error
	// on Intel/AMI BIOSes, yet the Extended Read function still works.
	// Work around this by forcing use of extensions when booted from CD
	if(id == boot_device_id && booted_from_cd()) {
		disk_add(disk, "cd0", id, 2048, ~0LL, &bios_disk_ops, true);
		dprintf("disk: added boot CD cd0 (id: 0x%x)\n", id);
	} else {
		bios_regs_init(&regs);
		regs.eax = INT13_EXT_INSTALL_CHECK;
		regs.ebx = 0x55AA;
		regs.edx = id;
		bios_call(0x13, &regs);

		if(regs.eflags & X86_FLAGS_CF || (regs.ebx & 0xFFFF) != 0xAA55 || !(regs.ecx & (1<<0))) {
			dprintf("disk: device 0x%x does not support extensions, ignoring\n", id);
			return;
		}

		// Get drive parameters. According to RBIL, some Phoenix BIOSes
		// fail to correctly handle the function if the flags word is
		// not 0. Clear the entire structure to be on the safe side
		memset(params, 0, sizeof(drive_parameters_t));
		params->size = sizeof(drive_parameters_t);
		bios_regs_init(&regs);
		regs.eax = INT13_EXT_GET_DRIVE_PARAMETERS;
		regs.edx = id;
		regs.esi = BIOS_MEM_BASE;
		bios_call(0x13, &regs);

		if(regs.eflags & X86_FLAGS_CF || !params->sector_count || !params->sector_size) {
			dprintf("disk: failed to obtain device parameters for device 0x%x\n", id);
			return;
		}

		// Register the disk with the disk manager
		sprintf(name, "hd%u", id - 0x80);
		disk_add(disk, name, id, params->sector_size, params->sector_count,
			&bios_disk_ops, id == boot_device_id);
		dprintf("disk: added device %s (id: 0x%x, sector_size: %u, sector_count: %zu)\n",
			name, id, params->sector_size, params->sector_count);
	}
}

// ============================================================================
// Detect all disks in the system.
//
void platform_disk_detect(void) {
	uint8_t count, id;

	dprintf("disk: boot device ID is 0x%x, partition offset is 0x%" PRIx64 "\n",
		boot_device_id, boot_part_offset);

	// Probe all hard disks
	count = get_disk_count();
	for(id = 0x80; id < count + 0x80; id++) {
		// If this is the boot device, ignore it - it will be added
		// after the loop is completed. This is done because this loop
		// only probes hard disks, so in order to support CD's, etc,
		// we have to add the boot disk separately
		if(id == boot_device_id) {
			continue;
		}

		add_disk(id);
	}

	// TODO: If not booted from PXE, add the boot device
	//if(!pxe_detect()) {
	add_disk(boot_device_id);
	//}
}
