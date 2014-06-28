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
  * @brief       EFI console functions
  */

#include <efi/efi.h>

#include <console.h>

// Console out protocol
static efi_simple_text_output_protocol_t *console_out;

// Reset the console to a default state
static void efi_console_reset(void) {
  efi_call(console_out->clear_screen, console_out);
}

/**
 * Write a character to the console.
 *
 * @param ch   Character to write
 */
static void efi_console_putc(char ch) {
  efi_char16_t str[3] = {};

  if (ch == '\n') {
    str[0] = '\r';
    str[1] = '\n';
  } else {
    str[0] = ch & 0x7F;
  }

  efi_call(console_out->output_string, console_out, str);
}

// EFI main console implementation
static console_t efi_console = {
  .reset = efi_console_reset,
  .putc = efi_console_putc,
};

/**
 * Initialize the EFI console
 */
void efi_console_init() {
  // --- Set up main console
  // Get console
  console_out = efi_system_table->con_out;

  // Clear screen
  efi_call(console_out->clear_screen, console_out);

  main_console = &efi_console;
}
