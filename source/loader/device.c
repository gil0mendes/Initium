/**
* The MIT License (MIT)
*
* Copyright (c) 2014-2015 Gil Mendes
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

#include <lib/string.h>

#include <device.h>
#include <fs.h>
#include <loader.h>
#include <memory.h>

/** List of all registered devices. */
static LIST_DECLARE(device_list);

/** Boot device. */
device_t *boot_device;

/** Read from a device.
 * @param device        Device to read from.
 * @param buf           Buffer to read into.
 * @param count         Number of bytes to read.
 * @param offset        Offset in the device to read from.
 * @return              Status code describing the result of the read. */
status_t device_read(device_t *device, void *buf, size_t count, offset_t offset) {
    if (!device->ops || !device->ops->read)
        return STATUS_NOT_SUPPORTED;

    if (!count) {
        return STATUS_SUCCESS;
    }

    return device->ops->read(device, buf, count, offset);
}

/**
 * Look up a device.
 *
 * Looks up a device. If given a string in the format "uuid:<uuid>", the device
 * will be looked up by filesystem UUID. If given a string in the format
 * "label:<label>", will be looked up by filesystem label. Otherwise, will be
 * looked up by the device name.
 *
 * @param name          String to look up.
 *
 * @return              Matching device, or NULL if no matches found.
 */
device_t *device_lookup(const char *name) {
    bool uuid = false, label = false;

    if (strncmp(name, "uuid:", 5) == 0) {
        uuid = true;
        name += 5;
    } else if (strncmp(name, "label:", 6) == 0) {
        label = true;
        name += 6;
    }

    if (!name[0]) {
        return NULL;
    }

    list_foreach(&device_list, iter) {
        device_t *device = list_entry(iter, device_t, header);

        if (uuid || label) {
            if (!device->mount) {
                continue;
            }

            if (strcmp((uuid) ? device->mount->uuid : device->mount->label, name) == 0) {
                return device;
            }
        } else {
            if (strcmp(device->name, name) == 0) {
                return device;
            }
        }
    }

    return NULL;
}

/** Register a device.
 * @param device        Device to register (details should be filled in).
 * @param name          Name to give the device (string will be duplicated). */
void device_register(device_t *device, const char *name) {
    if (device_lookup(name))
        internal_error("Device named '%s' already exists", name);

    device->name = strdup(name);

    list_init(&device->header);
    list_append(&device_list, &device->header);

    /* Probe for filesystems, */
    device->mount = fs_probe(device);
}

/**
 * Initialize the device manager.s
 */
void device_init(void) {
    target_device_probe();

    dprintf("device: detect devices:\n");
    list_foreach(&device_list, iter) {
        device_t *device = list_entry(iter, device_t, header);
        size_t indent = 0;
        char buf[128];

        /**
         * Figure out how much to indent the string (so we get a tree-like view
         * with child devices indented).
         */
        for (size_t i = 0; device->name[i]; i++) {
            if (device->name[i] == ',') {
                indent++;
            }
        }

        snprintf(buf, sizeof(buf), "Unknow");
        if (device->ops->identify) {
            device->ops->identify(device, buf, sizeof(buf));
        }

        dprintf(" %-*s%-*s -> %s\n", indent, "", 7 - indent, device->name, buf);
    }

    dprintf("device: boot device is %s\n", (boot_device) ? boot_device->name : "unknown");
}
