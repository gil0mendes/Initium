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
* @brief 				Device management
*/

#ifndef __DEVICE_H
#define __DEVICE_H

#include <lib/list.h>

struct mount;

// Type of a device
typedef enum device_type {
	DEVICE_TYPE_IMAGE,		// Boot image
	DEVICE_TYPE_NET,			// Network boot server
	DEVICE_TYPE_DISK,			// Disk device
} device_type_t;

// Structure containing details of a device
typedef struct device {
	list_t header;			// Link to device list

	char *name;					// Name of the device
	device_type_t type;	// Type of the device
	struct mount *fs;		// Filesystem that resides on the device
} device_t;

extern device_t *boot_device;

// Macro expanding to the current device
#define current_device		((current_environ) ? current_environ->device : boot_device)

extern device_t *device_lookup(const char *str);
extern void device_add(device_t *device, const char *name, device_type_t type);

#endif // __DEVICE_H
