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
 * @brief 				Disk Block IO protocol
 */

#ifndef __EFI_PROTOCOL_DISK_H
#define __EFI_PROTOCOL_DISK_H

#include <efi/api.h>

#define EFI_DISK_IO_PROTOCOL_GUID \
	{ 0xce345171, 0xba0b, 0x11d2, 0x8e, 0x4f, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

#define EFI_DISK_IO_PROTOCOL_REVISION 	0x00010000

// Define DISK IO Interface
INTERFACE_DECL(_EFI_DISK_IO_PROTOCOL);

// ============================================================================
// Reads a specified number of bytes from a device
//
// @param This 					Indicates a pointer to the calling context
// @param MediaId 			ID of the medium to be read
// @param Offset				The starting byte offset on the logical block I/O device
//		to read from
// @param BufferSize		The size in bytes of Buffer. The number of bytes to read
//		from the device.
// @param Buffer				A pointer to the destination buffer for the data. The
//		caller is responsible for either having implicit or explicit ownership of
//		the buffer.
//
// @retval EFI_SUCCESS 						The data was read correctly from the device.
// @retval EFI_DEVICE_ERROR 			The device reported an error while performing
//		the read operation.
// @retval EFI_NO_MEDIA 					There is no medium in the device.
// @retval EFI_MEDIA_CHANGED 			The MediaId is not for the current medium.
// @retval EFI_INVALID_PARAMETER 	The read request contains device addresses
//		that are not valid for the device.
typedef
EFI_STATUS
(EFIAPI *EFI_DISK_READ) (
	IN struct _EFI_DISK_IO_PROTOCOL      	*This,
	IN UINT32                   					MediaId,
	IN UINT64                   					Offset,
	IN UINTN                    					BufferSize,
	OUT VOID                    					*Buffer
	);

// ============================================================================
// Writes a specified number of bytes to a device
//
// @param This				Indicates a pointer to the calling context
// @param MediaId			ID of the medium to be written
// @param Offset			The starting byte offset on the logical block I/O device
// 		to write
// @param BufferSize	The size in bytes of Buffer. The number of bytes to write
//		to the device
// @param Buffer			A pointer to the buffer containing the data to be written
//
// @retval EFI_SUCCESS 						The data was written correctly to the device.
// @retval EFI_WRITE_PROTECTED 		The device cannot be written to.
// @retval EFI_NO_MEDIA 					There is no medium in the device.
// @retval EFI_MEDIA_CHANGED 			The MediaId is not for the current medium.
// @retval EFI_DEVICE_ERROR 			The device reported an error while performing
//		the write operation.
// @retval EFI_INVALID_PARAMETER 	The write request contains device addresses
//		that are not valid for the device.
typedef
EFI_STATUS
(EFIAPI *EFI_DISK_WRITE) (
	IN struct _EFI_DISK_IO_PROTOCOL      	*This,
	IN UINT32                   					MediaId,
	IN UINT64                   					Offset,
	IN UINTN                    					BufferSize,
	IN VOID                     					*Buffer
	);


typedef struct _EFI_DISK_IO_PROTOCOL {
	UINT64              Revision;
	EFI_DISK_READ       ReadDisk;
	EFI_DISK_WRITE      WriteDisk;
} EFI_DISK_IO;

#endif // __EFI_PROTOCOL_DISK_H
