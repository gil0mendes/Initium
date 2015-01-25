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
* @brief             EFI disk functions
*/


#include <disk.h>
#include <assert.h>
#include <loader.h>
#include <memory.h>

// Load EFI defenitions
#include <efi/efi.h>

// Load EFI disk interface definition
#include <efi/disk.h>

// ============================================================================
// Get the number of disks in the system.
//
// @return		Number of EFI hard disks.
//
static uint8_t
get_disk_count(void) {
	return 0;
}

// ============================================================================
// Detect all disks in the system.
//
void
platform_disk_detect(void) {
	// For handles
	efi_status_t 			status;
	UINTN						handleCount = 0;
	UINTN						handleIndex;
	EFI_HANDLE 			*handles;



}
