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
 * @brief		EFI disk interface definitions
 */

#ifndef __EFI_DISK_H
#define __EFI_DISK_H

// Load EFI defenitions
#include <efi/efi.h>

extern UINTN volumesCount;

typedef struct {
   EFI_DEVICE_PATH     *DevicePath;
   EFI_HANDLE          DeviceHandle;
   EFI_FILE            *RootDir;
   //CHAR16              *VolName;
   //EG_IMAGE            *VolIconImage;
   //EG_IMAGE            *VolBadgeImage;
   UINTN               DiskKind;
   BOOLEAN             IsAppleLegacy;
   BOOLEAN             HasBootCode;
   //CHAR16              *OSIconName;
   //CHAR16              *OSName;
   BOOLEAN             IsMbrPartition;
   UINTN               MbrPartitionIndex;
   EFI_BLOCK_IO        *BlockIO;
   UINT64              BlockIOOffset;
   EFI_BLOCK_IO        *WholeDiskBlockIO;
   EFI_DEVICE_PATH     *WholeDiskDevicePath;
   //MBR_PARTITION_INFO  *MbrPartitionTable;
   BOOLEAN             IsReadable;
} REFIT_VOLUME;

#endif // __EFI_DISK_H
