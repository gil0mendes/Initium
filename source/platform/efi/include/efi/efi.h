/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2016 Gil Mendes <gil00mendes@gmail.com>
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
 * @brief               EFI platform core definitions.
 */

#ifndef __EFI_EFI_H
#define __EFI_EFI_H

#include <efi/arch/efi.h>

#include <efi/api.h>

#include <status.h>

extern char __text_start[], __data_start[], __bss_start[];

extern efi_handle_t efi_image_handle;
extern efi_loaded_image_t *efi_loaded_image;
extern efi_system_table_t *efi_system_table;
extern efi_runtime_services_t *efi_runtime_services;
extern efi_boot_services_t *efi_boot_services;

extern void efi_main(efi_handle_t image_handle, efi_system_table_t *system_table) __noreturn;

#endif /* __EFI_EFI_H */
