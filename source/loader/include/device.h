/**
* The MIT License (MIT)
*
* Copyright (c) 2015 Gil Mendes
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
 * @brief               Device management.
 */

#ifndef __DEVICE_H
#define __DEVICE_H

#include <lib/list.h>

#include <status.h>

struct device;

/** Types of devices. */
typedef enum device_type {
    DEVICE_TYPE_DISK,                   /**< Local disk. */
    DEVICE_TYPE_NET,                    /**< Network device. */
    DEVICE_TYPE_IMAGE,                  /**< Boot image. */
} device_type_t;

/** Device operations structure. */
typedef struct device_ops {
    /** Read from a device.
     * @param device        Device to read from.
     * @param buf           Buffer to read into.
     * @param count         Number of bytes to read.
     * @param offset        Offset in the device to read from.
     * @return              Status code describing the result of the read. */
    status_t (*read)(struct device *device, void *buf, size_t count, offset_t offset);
} device_ops_t;

/** Base device structure (embedded by device type structures). */
typedef struct device {
    list_t header;                      /**< Link to devices list. */

    device_type_t type;                 /**< Type of the device. */
    const device_ops_t *ops;            /**< Operations for the device (can be NULL). */

    char *name;                         /**< Name of the device. */
} device_t;

extern status_t device_read(device_t *device, void *buf, size_t count, offset_t offset);

extern device_t *device_lookup(const char *name);
extern void device_register(device_t *device, const char *name);

#endif /* __DEVICE_H */
