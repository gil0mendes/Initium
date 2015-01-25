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
 * @brief		EFI Disk API definitions.
 */

#ifndef __EFI_PROTOCOL_BLOCK_H
#define __EFI_PROTOCOL_BLOCK_H

#include <efi/api.h>

// Block IO protocol
#define BLOCK_IO_PROTOCOL \
	{ 0x964e5b21, 0x6459, 0x11d2, 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

#define EFI_BLOCK_IO_INTERFACE_REVISION   0x00010000
#define EFI_BLOCK_IO_INTERFACE_REVISION2  0x00020001
#define EFI_BLOCK_IO_INTERFACE_REVISION3  ((2<<16) | 31)

// --- Protocol Interface Structure

// Declare BLOCK IO structure
INTERFACE_DECL(_EFI_BLOCK_IO_PROTOCOL);

// ============================================================================
// Resets the block device hardware
//
// @param This										Indicates a pointer to the calling context.
// @param Extended Verification 	Indicates that the driver may perform a more
// 		exhaitive verification operation of the device during reset.
//
// @retval EFI_SUCCESS 				The block device was reset
// @retval EFI_DEVICE_ERROR 	The block device is not functioning correctly and
//		could not be reset.
typedef
efi_status_t
(__efiapi *EFI_BLOCK_RESET) (
	IN struct _EFI_BLOCK_IO_PROTOCOL 		*This,
	IN BOOLEAN 													ExtendedVerification
	);

// ============================================================================
// Reads the requested number of blocks from the device.
//
// @param This 					Indicates a pointer to the calling context.
// @param MediaId 			The media ID that the read request is for
// @param LBA 					The starting logical block address to read from on the
// 		device.
// @param BufferSize 		The size of the Buffer in bytes. This must be a multiple
// 		of the intrinsic block size of the device.
// @param Buffer 				A pointer to the destination buffer for the data. The
// 		caller is responsible for either having implicit or explicit ownership of
// 		the buffer.
//
// @retval EFI_SUCCESS 					The data was read correctly from the device.
// @retval EFI_DEVICE_ERROR  		The device reported an error while attempting to
// 		perform the read operation.
// @retval EFI_NO_MEDIA 				There is no media in the device.
// @retval EFI_MEDIA_CHANGED 		The MediaId is not for the current media.
// @retval EFI_BAD_BUFFER_SIZE 	The BufferSize parameter is not a multiple of
// 		the intrinsic block size of the device.
// @retval EFI_INVALID_PARAMETER The read request contains LBAs that are not
// 		valid, or the buffer is not on proper alignment.
typedef
efi_status_t
(__efiapi *EFI_BLOCK_READ) (
	IN struct _EFI_BLOCK_IO_PROTOCOL     	*This,
	IN UINT32                   					MediaId,
	IN EFI_LBA                  					LBA,
	IN UINTN                    					BufferSize,
	OUT VOID                    					*Buffer
	);

// ============================================================================
// Writes a specified number of blocks to the device
//
// @param This				Indicates a pointer to the calling context
// @param MediaId			The media ID that the write request is for
// @param LBA					The starting logical block address to be written
// @param BufferSize	The size in bytes of Buffer. This must be a multiple of
//		the intrinsic block size of the device
// @param Buffer			A pointer to the source buffer for the data
//
// @retval EFI_SUCCESS 						The data were written correctly to the device.
// @retval EFI_WRITE_PROTECTED 		The device cannot be written to.
// @retval EFI_NO_MEDIA 					There is no media in the device.
// @retval EFI_MEDIA_CHANGED 			The MediaId is not for the current media.
// @retval EFI_DEVICE_ERROR 			The device reported an error while attempting
// 		to perform the write operation.
// @retval EFI_BAD_BUFFER_SIZE 		The BufferSize parameter is not a multiple of
//		the intrinsic block size of the device.
// @retval EFI_INVALID_PARAMETER 	The write request contains LBAs that are not
// 		valid, or the buffer is not on proper alignment.
typedef
efi_status_t
(__efiapi *EFI_BLOCK_WRITE) (
	IN struct _EFI_BLOCK_IO_PROTOCOL     	*This,
	IN UINT32                   					MediaId,
	IN EFI_LBA                  					LBA,
	IN UINTN                    					BufferSize,
	IN VOID                     					*Buffer
	);

// ============================================================================
// Flushes all modified data to a physical block device
//
// @param This				Indicates a pointer to the calling context
//
// @retval EFI_SUCCESS 				All outstanding data were written correctly to
//		the device.
// @retval EFI_DEVICE_ERROR 	The device reported an error while attempting to
//		write data.
// @retval EFI_NO_MEDIA 			There is no media in the device.
typedef
efi_status_t
(__efiapi *EFI_BLOCK_FLUSH) (
	IN struct _EFI_BLOCK_IO_PROTOCOL     *This
	);

typedef struct {
	UINT32              MediaId;
	BOOLEAN             RemovableMedia;
	BOOLEAN             MediaPresent;

	BOOLEAN             LogicalPartition;
	BOOLEAN             ReadOnly;
	BOOLEAN             WriteCaching;

	UINT32              BlockSize;
	UINT32              IoAlign;

	EFI_LBA             LastBlock;

	// revision 2
	EFI_LBA             LowestAlignedLba;
	UINT32              LogicalBlocksPerPhysicalBlock;

	// revision 3
	UINT32              OptimalTransferLengthGranularity;
} EFI_BLOCK_IO_MEDIA;

typedef struct _EFI_BLOCK_IO_PROTOCOL {
	UINT64                  Revision;

	EFI_BLOCK_IO_MEDIA      *Media;

	EFI_BLOCK_RESET         Reset;
	EFI_BLOCK_READ          ReadBlocks;
	EFI_BLOCK_WRITE         WriteBlocks;
	EFI_BLOCK_FLUSH         FlushBlocks;
} EFI_BLOCK_IO;

#endif // __EFI_PROTOCOL_BLOCK_H
