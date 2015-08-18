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
  * @brief               EFI executable loader.
  *
  * TODO:
  *  - Need to generate file path and set in loaded image protocol.
  *  - Reset GOP to original mode (add efi_console_reset function)
  *  - Support passing arguments to the image.
  */

 #include <efi/disk.h>
 #include <efi/efi.h>
 #include <efi/memory.h>

 #include <config.h>
 #include <fs.h>
 #include <loader.h>
 #include <memory.h>

 /** Load an EFI executable.
  * @param _handle       Pointer to executable handle. */
 static __noreturn void efi_loader_load(void *_handle) {
     fs_handle_t *handle = _handle;
     void *buf;
     efi_handle_t image_handle;
     efi_loaded_image_t *image;
     efi_char16_t *exit_data;
     efi_uintn_t exit_data_size;
     efi_status_t status;
     status_t ret;

     /* Allocate a buffer to read the image into. */
     buf = memory_alloc(round_up(handle->size, PAGE_SIZE), 0, 0, 0, MEMORY_TYPE_INTERNAL, 0, NULL);
     if (!buf) {
         boot_error("Failed to allocate %" PRIu64 " bytes", handle->size);
     }

     /* Read it in. */
     ret = fs_read(handle, buf, handle->size, 0);
     if (ret != STATUS_SUCCESS) {
         boot_error("Failed to read EFI image (%d)", ret);
     }

     /* Ask the firmware to load the image. */
     status = efi_call(
         efi_boot_services->load_image,
         false, efi_image_handle, NULL, buf, handle->size, &image_handle);
     if (status != EFI_SUCCESS) {
         boot_error("Failed to load EFI image (0x%zx)", status);
     }

     /* Get the loaded image protocol. */
     status = efi_get_loaded_image(image_handle, &image);
     if (status != EFI_SUCCESS) {
         boot_error("Failed to get loaded image protocol (0x%zx)", status);
     }

     /* Try to identify the handle of the device the image was on. */
     image->device_handle = (handle->mount->device->type == DEVICE_TYPE_DISK)
         ? efi_disk_get_handle((disk_device_t *)handle->mount->device)
         : NULL;

     fs_close(handle);

     /* Clear the framebuffer, and reset the EFI consoles. */
     console_reset(&main_console);
     efi_call(efi_system_table->con_in->reset, efi_system_table->con_in, false);
     efi_call(efi_system_table->con_out->reset, efi_system_table->con_out, false);

     /* Release all memory allocated by the loader. */
     efi_memory_cleanup();

     /* Start the image. */
     dprintf("starting image...\n");
     status = efi_call(efi_boot_services->start_image, image_handle, &exit_data_size, &exit_data);
     dprintf("efi: loaded image returned status 0x%zx\n", status);

     /* We can't do anything here - the loaded image may have done things making
      * our internal state invalid. Just pass through the error to whatever
      * loaded us. */
     efi_exit(status, exit_data, exit_data_size);
 }

 /** EFI loader operations. */
 static loader_ops_t efi_loader_ops = {
     .load = efi_loader_load,
 };

 /** Load an EFI executable.
  * @param args          Argument list.
  * @return              Whether successful. */
 static bool config_cmd_efi(value_list_t *args) {
     fs_handle_t *handle;
     status_t ret;

     if (args->count != 1 || args->values[0].type != VALUE_TYPE_STRING) {
         config_error("efi: Invalid arguments");
         return false;
     }

     ret = fs_open(args->values[0].string, NULL, &handle);
     if (ret != STATUS_SUCCESS) {
         config_error("efi: Error %d opening '%s'", ret, args->values[0].string);
         return false;
     } else if (handle->directory) {
         fs_close(handle);
         config_error("efi: '%s' is a directory", args->values[0].string);
         return false;
     }

     environ_set_loader(current_environ, &efi_loader_ops, handle);
     return true;
 }

 BUILTIN_COMMAND("efi", config_cmd_efi);
