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
* @brief 							BIOS disk interface definitions
*/

#ifndef __BIOS_DISK_H
#define __BIOS_DISK_H

// INT13 functions defenitions
#define INT13_GET_DRIVE_PARAMETERS 			0x0800 	// Get drive parameters
#define INT13_EXT_INSTALL_CHECK 				0x4100 	// INT13 Extensions - Installation check
#define INT13_EXT_READ 									0x4200 	// INT13 Extensions - Extended read
#define INT13_EXT_GET_DRIVE_PARAMETERS 	0x4800 	// INT13 Extensions - Get drive parameters
#define INT13_CDROM_GET_STATUS 					0x4B01	// Bootable CD-ROM - Get status

#ifndef __ASM__

#include <types.h>

struct disk;

// Drive parameters structure. We only care about the EDD 1.x fields
typedef struct drive_parameters {
	uint16_t size;
	uint16_t flags;
	uint32_t cylinders;
	uint32_t heads;
	uint32_t spt;
	uint64_t sector_count;
	uint16_t sector_size;
} __packed drive_parameters_t;

// Disk address packet structure
typedef struct disk_address_packet {
	uint8_t 	size;
	uint8_t 	reserved1;
	uint16_t 	block_count;
	uint16_t 	buffer_offset;
	uint16_t 	buffer_segment;
	uint64_t 	start_lba;
} __packed disk_address_packet_t;

// Bootable CD-ROM specification packet
typedef struct specification_packet {
	uint8_t 	size;
	uint8_t 	media_type;
	uint8_t 	drive_number;
	uint8_t 	controller_num;
	uint32_t 	image_lba;
	uint16_t 	device_spec;
} __packed specification_packet_t;

#endif // __ASM__

#endif // __BIOS_DISK_H
