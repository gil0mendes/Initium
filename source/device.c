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
 * @brief               Device management.
 */

#include <lib/string.h>

#include <config.h>
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

    if (!count)
        return STATUS_SUCCESS;

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

    if (!name[0])
        return NULL;

    list_foreach(&device_list, iter) {
        device_t *device = list_entry(iter, device_t, header);

        if (uuid || label) {
            if (!device->mount)
                continue;

            if (strcmp((uuid) ? device->mount->uuid : device->mount->label, name) == 0)
                return device;
        } else {
            if (strcmp(device->name, name) == 0)
                return device;
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

    /* Probe for filesystems. */
    device->mount = fs_probe(device);
}

/** Set the current device.
 * @param args          Argument list.
 * @return              Whether successful. */
static bool config_cmd_device(value_list_t *args) {
    device_t *device;
    value_t value;

    if (args->count != 1 || args->values[0].type != VALUE_TYPE_STRING) {
        config_error("Invalid arguments");
        return false;
    }

    device = device_lookup(args->values[0].string);
    if (!device) {
        config_error("Device '%s' not found", args->values[0].string);
        return false;
    }

    current_environ->device = device;

    value.type = VALUE_TYPE_STRING;
    value.string = device->name;
    environ_insert(current_environ, "device", &value);

    if (device->mount) {
        value.string = device->mount->label;
        environ_insert(current_environ, "device_label", &value);
        value.string = device->mount->uuid;
        environ_insert(current_environ, "device_uuid", &value);
    } else {
        environ_remove(current_environ, "device_label");
        environ_remove(current_environ, "device_uuid");
    }

    /* Change directory to the root (NULL indicates root to the FS code). */
    if (current_environ->directory) {
        fs_close(current_environ->directory);
    }

    current_environ->directory = NULL;

    return true;
}

BUILTIN_COMMAND("device", config_cmd_device);

/** Print a list of devices.
 * @param console       Console to write to.
 * @param indent        Indentation level. */
static void print_device_list(console_t *console, size_t indent) {
    list_foreach(&device_list, iter) {
        device_t *device = list_entry(iter, device_t, header);
        size_t child = 0;
        char buf[128];

        /* Figure out how much to indent the string (so we get a tree-like view
         * with child devices indented). */
        for (size_t i = 0; device->name[i]; i++) {
            if (device->name[i] == ',')
                child++;
        }

        snprintf(buf, sizeof(buf), "Unknown");
        if (device->ops->identify)
            device->ops->identify(device, DEVICE_IDENTIFY_SHORT, buf, sizeof(buf));

        console_printf(console, "%-*s%-*s -> %s\n", indent + child, "", 7 - child, device->name, buf);
    }
}

/** Print a list of devices.
 * @param args          Argument list.
 * @return              Whether successful. */
static bool config_cmd_lsdev(value_list_t *args) {
    if (args->count == 0) {
        print_device_list(config_console, 0);
        return true;
    } else if (args->count == 1 && args->values[0].type == VALUE_TYPE_STRING) {
        device_t *device;
        char buf[256];

        device = device_lookup(args->values[0].string);
        if (!device) {
            config_error("Device '%s' not found", args->values[0].string);
            return false;
        }

        config_printf("name       = %s\n", device->name);

        if (device->ops->identify) {
            device->ops->identify(device, DEVICE_IDENTIFY_SHORT, buf, sizeof(buf));
        } else {
            snprintf(buf, sizeof(buf), "Unknown");
        }

        config_printf("identify   = %s\n", buf);

        if (device->ops->identify) {
            device->ops->identify(device, DEVICE_IDENTIFY_LONG, buf, sizeof(buf));
            config_printf("%s", buf);
        }

        if (device->mount) {
            config_printf("fs         = %s\n", device->mount->ops->name);
            config_printf("uuid       = %s\n", device->mount->uuid);
            config_printf("label      = %s\n", device->mount->label);
        }

        return true;
    } else {
        config_error("Invalid arguments");
        return false;
    }
}

BUILTIN_COMMAND("lsdev", config_cmd_lsdev);

/** Initialize the device manager. */
void device_init(void) {
    target_device_probe();

    /* Print out a list of all devices. */
    dprintf("device: detected devices:\n");
    print_device_list(&debug_console, 1);

    /* Set the device in the environment. */
    if (boot_device) {
        value_t value;

        dprintf("device: boot device is %s\n", (boot_device) ? boot_device->name : "unknown");

        root_environ->device = boot_device;

        value.type = VALUE_TYPE_STRING;
        value.string = boot_device->name;
        environ_insert(root_environ, "device", &value);

        if (boot_device->mount) {
            value.string = boot_device->mount->label;
            environ_insert(root_environ, "device_label", &value);
            value.string = boot_device->mount->uuid;
            environ_insert(root_environ, "device_uuid", &value);
        }
    }

    if (!boot_device || !boot_device->mount)
        boot_error("Unable to find boot filesystem");
}
