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
 * @brief             EFI boot services utility functions
 */

#include <efi/efi.h>

#include <lib/string.h>

#include <loader.h>
#include <memory.h>

/** Various protocol GUIDs */
static efi_guid_t device_path_guid = EFI_DEVICE_PATH_PROTOCOL_GUID;
static efi_guid_t device_path_to_text_guid = EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;
static efi_guid_t loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

// Device path to text protocol
static efi_device_path_to_text_protocol_t *device_path_to_text;

/**
 * Convert an EFI status code to an internal status.
 *
 * @param  status Status to convert
 * @return        Internal status code
 */
status_t efi_convert_status(efi_status_t status) {
    switch (status) {
        case EFI_SUCCESS:
            return STATUS_SUCCESS;
        case EFI_UNSUPPORTED:
            return STATUS_NOT_SUPPORTED;
        case EFI_INVALID_PARAMETER:
            return STATUS_INVALID_ARG;
        case EFI_DEVICE_ERROR:
        case EFI_NO_MEDIA:
        case EFI_MEDIA_CHANGED:
            return STATUS_DEVICE_ERROR;
        case EFI_WRITE_PROTECTED:
            return STATUS_READ_ONLY;
        case EFI_VOLUME_CORRUPTED:
            return STATUS_CORRUPT_FS;
        case EFI_VOLUME_FULL:
            return STATUS_FS_FULL;
        case EFI_NOT_FOUND:
            return STATUS_NOT_FOUND;
        case EFI_TIMEOUT:
            return STATUS_TIMED_OUT;
        default:
            return STATUS_SYSTEM_ERROR;
    }
}

/**
 * Allocate EFI pool memory.
 *
 * @param pool_type         Pool memory type to allocate.
 * @param size              Size of memory to allocate.
 * @param _buffer           Where to store pointer to allocated memory.
 * @return                  EFI status code
 */
efi_status_t efi_allocate_pool(efi_memory_type_t pool_type, efi_uintn_t size, void **_buffer)
{
    return efi_call(efi_boot_services->allocate_pool, pool_type, size, _buffer);
}

/**
 * Free EFI pool memory.
 *
 * @param buffer            Pointer to memory to free.
 * @return                  EFI status code
 */
efi_status_t efi_free_pool(void *buffer)
{
    return efi_call(efi_boot_services->free_pool, buffer);
}

//============================================================================
// Return an array of handles that support a protocol.
//
// Returns an array of handles that support a specified protocol. This is a
// wrapper for the EFI LocateHandle boot service that handles the allocation
// of a sufficiently sized buffer. The returned buffer should be freed with
// free() once it is no longer needed.
//
// @param search_type  Specifies which handles are to be returned.
// @param protocol     The protocol to search for.
// @param search_key   Search key.
// @param _handles     Where to store pointer to handle array.
// @param _num_handles Where to store the number of handles returned.
//
// @return             EFI status code.
efi_status_t
efi_locate_handle(
    efi_locate_search_type_t search_type, efi_guid_t *protocol,
    void *search_key, efi_handle_t **_handles, efi_uintn_t *_num_handles)
{
       efi_handle_t *handles = NULL;
       efi_uintn_t size = 0;
       efi_status_t ret;

       // Call a first time to get the needed buffer size
       ret = efi_call(efi_boot_services->locate_handle,
               search_type, protocol, search_key, &size, handles);
       if(ret == EFI_BUFFER_TOO_SMALL) {
               handles = malloc(size);

               ret = efi_call(efi_boot_services->locate_handle,
                       search_type, protocol, search_key, &size, handles);
               if(ret != EFI_SUCCESS)
                       free(handles);
       }

       *_handles = handles;
       *_num_handles = size / sizeof(efi_handle_t);
       return ret;
}

// ============================================================================
// Open a protocol supported by a handle.
//
// This function is a wrapper for the EFI OpenProtocol boot service which
// passes the correct values for certain arguments.
//
// @param handle       Handle to open on.
// @param protocol     Protocol to open.
// @param attributes   Open mode of the protocol interface.
// @param _interface   Where to store pointer to opened interface.
//
// @return             EFI status code.
efi_status_t
efi_open_protocol(
    efi_handle_t handle, efi_guid_t *protocol, efi_uint32_t attributes,
    void **interface)
{
       return efi_call(efi_boot_services->open_protocol, handle,
        protocol, interface, efi_image_handle, NULL, attributes);
}

/**
 * Get the loaded image protocol from an image handle.
 *
 * @param handle        Image handle.
 * @param _image        Where to store loaded image pointer.
 * @return              EFI status code.
 */
efi_status_t efi_get_loaded_image(efi_handle_t handle, efi_loaded_image_t **_image) {
    return efi_open_protocol(handle, &loaded_image_guid, EFI_OPEN_PROTOCOL_GET_PROTOCOL, (void **)_image);
}

/**
 * Open the device path protocol for a handle.
 *
 * @param handle            Handle to open for.
 * @return                  Pointer to device path protocol on success,
 *                          NULL on failure.
 */
efi_device_path_t *efi_get_device_path(efi_handle_t handle)
{
    efi_device_path_t *path;
    efi_status_t ret;

    ret = efi_open_protocol(handle, &device_path_guid, EFI_OPEN_PROTOCOL_GET_PROTOCOL, (void **)&path);
    if (ret != EFI_SUCCESS) {
        return NULL;
    }

    if (path->type == EFI_DEVICE_PATH_TYPE_END) {
        return NULL;
    }

    return path;
}

/**
 * Helper to print a string representation of a device path.
 *
 * @param path          Device path protocol to print.
 * @param cb            Helper function to print with
 * @param data          Data to pass to helper function
 */
void
efi_print_device_path(efi_device_path_t *path, void (*cb)(void *data, char ch), void *data)
{
    static efi_char16_t unknown[] = {'U', 'n', 'k', 'n', 'o', 'w', 'n'};

    efi_char16_t *str;

    // For now this only works on UEFI 2.0+, previous versions do not jave the device path to text protocol
    if (!device_path_to_text) {
        efi_handle_t *handles;
        efi_uintn_t num_handles;
        efi_status_t ret;

        // Get the device path to text protocol
        ret = efi_locate_handle(EFI_BY_PROTOCOL, &device_path_to_text_guid, NULL, &handles, &num_handles);
        if (ret == EFI_SUCCESS) {
            efi_open_protocol(
                    handles[0], &device_path_to_text_guid, EFI_OPEN_PROTOCOL_GET_PROTOCOL, (void **)&device_path_to_text
            );
            free(handles);
        }
    }

    // Get the device path string
    str = (path && device_path_to_text) ? efi_call(device_path_to_text->convert_device_path_to_text, path, false, false) : NULL;

    if (!str) {
        str = unknown;
    }

    for (size_t i = 0; str[i]; i++) {
        // FIXME: Unicode
        if (str[i] & 0x7f) {
            cb(data, str[i] & 0x7f);
        }
    }

    if (str != unknown) {
        efi_free_pool(str);
    }
}

/**
 * Determine if a device path is a child of another.
 *
 * @param  parent Parent device path
 * @param  child  Child device path
 * @return        Whether child is a child of parent
 */
bool efi_is_child_device_node(efi_device_path_t *parent, efi_device_path_t *child)
{
    while (parent) {
        if (memcmp(child, parent, min(parent->length, child->length)) != 0) {
            return false;
        }

        parent = efi_next_device_node(parent);
        child = efi_next_device_node(child);
    }

    return child != NULL;
}

/**
 * Exit the loader.
 *
 * @param status        Exit status.
 * @param data          Pointer to null-terminated string giving exit reason.
 * @param data_size     Size of the exit data in bytes.
 */
__noreturn void efi_exit(efi_status_t status, efi_char16_t *data, efi_uintn_t data_size) {
    efi_status_t ret;

    ret = efi_call(efi_boot_services->exit, efi_image_handle, status, data_size, data);
    internal_error("EFI exit failed (0x%zx)", ret);
}
