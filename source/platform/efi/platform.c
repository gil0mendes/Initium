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
  * @brief       EFI platform main functions.
  */

#include <efi/disk.h>
#include <efi/efi.h>
#include <efi/memory.h>

#include <device.h>
#include <loader.h>
#include <screen.h>

extern void loader_main(void);

/** Handle to the loader image */
efi_handle_t efi_image_handle;

/** Loaded image strucutre for the loader image */
efi_loaded_image_t *efi_loaded_image;

/** Pointer to the EFI system table and boot/runtime services. */
efi_system_table_t *efi_system_table;
efi_runtime_services_t *efi_runtime_services;
efi_boot_services_t *efi_boot_services;

/**
 * Main function of the EFI loader.
 *
 * @param image_handle  Handle to the loader image.
 * @param system_table  Pointer to EFI system table.
 *
 * @return   EFI Status code
 */
 efi_status_t efi_init(efi_handle_t image_handle, efi_system_table_t *system_table) {
    efi_status_t ret;

    // Save image handler
    efi_image_handle = image_handle;

    // Save EFI system table
    efi_system_table = system_table;
    efi_runtime_services = system_table->runtime_services;
    efi_boot_services = system_table->boot_services;

    // Initialize architecture code
    arch_init();

    // Firmware is required to set a 5 minute watchdog timer before
    // running an image. Disable it.
    efi_call(efi_boot_services->set_watchdog_timer, 0, 0, 0, NULL);

    // Initialise console
    efi_console_init();

    // Initialise memory map
    efi_memory_init();

    // Initialize video
    efi_video_init();

    /* Get the loaded image protocol. */
    ret = efi_get_loaded_image(image_handle, &efi_loaded_image);

    if (ret != EFI_SUCCESS) {
        internal_error("Failed to get loaded image protocol (0x%x)", ret);
    }

    // Call loader main function
    loader_main();

    return EFI_SUCCESS;
}

/**
 * Detect and register all devices.
 */
void target_device_probe(void) {
    efi_disk_init();
}

/**
 * Reboot the system.
 */
void target_reboot(void) {
    efi_call(efi_runtime_services->reset_system, EFI_RESET_WARM, EFI_SUCCESS, 0, NULL);
    internal_error("EFI reset failed");
}

/**
 * Exit the loader.
 */
void target_exit(void) {
    efi_exit(EFI_SUCCESS, NULL, 0);
}
