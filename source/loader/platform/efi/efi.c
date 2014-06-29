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
 * @brief             EFI library functions
 */

#include <efi/efi.h>

#include <memory.h>

/**
 * Return an array of handles that support a protocol.
 *
 * Returns an array of handles that support a specified protocol. This is a
 * wrapper for the EFI LocateHandle boot service that handles the allocation
 * of a sufficiently sized buffer. The returned buffer should be freed with
 * free() once it is no longer needed.
 *
 * @param search_type  Specifies which handles are to be returned.
 * @param protocol     The protocol to search for.
 * @param search_key   Search key.
 * @param _num_handles Where to store the number of handles returned.
 * @param _handles     Where to store pointer to handle array.
 *
 * @return             EFI status code.
 */
efi_status_t efi_locate_handle(efi_locate_search_type_t search_type,
       efi_guid_t *protocol, void *search_key, efi_uintn_t *_num_handles,
       efi_handle_t **_handles)
{
       efi_handle_t *handles = NULL;
       efi_uintn_t size = 0;
       efi_status_t ret;

       /* Call a first time to get the needed buffer size. */
       ret = efi_call(efi_system_table->boot_services->locate_handle,
               search_type, protocol, search_key, &size, handles);
       if(ret == EFI_BUFFER_TOO_SMALL) {
               handles = malloc(size);

               ret = efi_call(efi_system_table->boot_services->locate_handle,
                       search_type, protocol, search_key, &size, handles);
               if(ret != EFI_SUCCESS)
                       free(handles);
       }

       *_num_handles = size / sizeof(efi_handle_t);
       *_handles = handles;
       return ret;
}

/**
 * Open a protocol supported by a handle.
 *
 * This function is a wrapper for the EFI OpenProtocol boot service which
 * passes the correct values for certain arguments.
 *
 * @param handle       Handle to open on.
 * @param protocol     Protocol to open.
 * @param attributes   Open mode of the protocol interface.
 * @param _interface   Where to store pointer to opened interface.
 *
 * @return             EFI status code.
 */
efi_status_t efi_open_protocol(efi_handle_t handle, efi_guid_t *protocol,
       efi_uint32_t attributes, void **interface)
{
       return efi_call(efi_system_table->boot_services->open_protocol,
               handle, protocol, interface, efi_image_handle, NULL,
               attributes);
}
