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
  * @brief       EFI platform main functions.
  */

#include <efi/efi.h>

#include <loader.h>

// Pointer to the EFI system table
efi_system_table_t *efi_system_table;

/**
 * Main function of the EFI loader.
 *
 * @param image       Handle to the loader image
 * @param systab      Pointer to EFI system table.
 *
 * @return   EFI Status code
 */
 efi_status_t platform_init(efi_handle_t image, efi_system_table_t *systab) {
   // Save EFI system table
   efi_system_table = systab;

   // Initialise console
   efi_console_init();

   // Hey LAOS. Say hello!
   printf("Hello, I'm LAOS!\n");

   // For test
   while(true) {}
 }
