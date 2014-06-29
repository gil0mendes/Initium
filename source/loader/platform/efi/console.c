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
#include <loader.h>

// Serial port parameters (match the defaults given in the EFI spec)
#define SERIAL_BAUD_RATE	115200
#define SERIAL_PARITY		EFI_NO_PARITY
#define SERIAL_DATA_BITS	8
#define SERIAL_STOP_BITS	EFI_ONE_STOP_BIT

// Serial protocol GUID
static efi_guid_t serial_io_guid = EFI_SERIAL_IO_PROTOCOL_GUID;

// Console protocols
static efi_simple_text_output_protocol_t *console_out;
static efi_simple_text_input_protocol_t *console_in;
static efi_serial_io_protocol_t *serial_io;

// Saved key press
static efi_input_key_t saved_key;

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

// EFI main console output operations
static console_out_ops_t efi_console_out_ops = {
  .reset = efi_console_reset,
  .putc = efi_console_putc,
};

/**
 * Check for a character from the console.
 *
 * @return             Whether a character is available.
 */
static bool efi_console_poll(void) {
       efi_input_key_t key;
       efi_status_t ret;

       if(saved_key.scan_code || saved_key.unicode_char) {
           return true;
       }

       ret = efi_call(console_in->read_key_stroke, console_in, &key);
       if(ret != EFI_SUCCESS) {
           return false;
       }

       // Save the key press to be returned by getc()
       saved_key = key;
       return true;
}

// EFI scan code conversion table
static uint16_t efi_scan_codes[] = {
       0,
       CONSOLE_KEY_UP, CONSOLE_KEY_DOWN, CONSOLE_KEY_RIGHT, CONSOLE_KEY_LEFT,
       CONSOLE_KEY_HOME, CONSOLE_KEY_END, 0, CONSOLE_KEY_DELETE, 0, 0,
       CONSOLE_KEY_F1, CONSOLE_KEY_F2, CONSOLE_KEY_F3, CONSOLE_KEY_F4,
       CONSOLE_KEY_F5, CONSOLE_KEY_F6, CONSOLE_KEY_F7, CONSOLE_KEY_F8,
       CONSOLE_KEY_F9, CONSOLE_KEY_F10, '\e',
};

#include <loader.h>

/**
 * Read a character from the console.
 *
 * @return     Character read.
 */
static uint16_t efi_console_getc(void) {
       efi_input_key_t key;
       efi_status_t ret;

       while(true) {
               if(saved_key.scan_code || saved_key.unicode_char) {
                       key = saved_key;
                       saved_key.scan_code = saved_key.unicode_char = 0;
               } else {
                       ret = efi_call(console_in->read_key_stroke, console_in, &key);
                       if(ret != EFI_SUCCESS)
                               continue;
               }

               if(key.scan_code) {
                       if(key.scan_code >= ARRAY_SIZE(efi_scan_codes)) {
                               continue;
                       } else if(!efi_scan_codes[key.scan_code]) {
                               continue;
                       }

                       return efi_scan_codes[key.scan_code];
               } else if(key.unicode_char & 0x7f) {
                       /* Whee, Unicode! */
                       return key.unicode_char & 0x7f;
               }
       }
}

// EFI main console input operations
static console_in_ops_t efi_console_in_ops = {
       .poll = efi_console_poll,
       .getc = efi_console_getc,
};

/**
 * Write a character to the serial console.
 *
 * @param ch     Character to write.
 */
static void efi_serial_putc(char ch) {
    efi_uintn_t size = 1;

    if(ch == '\n') {
        efi_serial_putc('\r');
    }

    efi_call(serial_io->write, serial_io, &size, &ch);
}

// EFI serial console output operations
static console_out_ops_t efi_serial_out_ops = {
       .putc = efi_serial_putc,
};

/**
 * Initialize the EFI console
 */
void efi_console_init() {
    efi_handle_t *handles;
    efi_uintn_t num_handles;
    efi_status_t ret;
    console_out = efi_system_table->con_out;
    console_in = efi_system_table->con_in;

    // Clear screen
    efi_call(console_out->clear_screen, console_out);

    main_console.out = &efi_console_out_ops;
    main_console.in = &efi_console_in_ops;

    // Look for a serial console
    ret = efi_locate_handle(EFI_BY_PROTOCOL, &serial_io_guid, NULL,
           &num_handles, &handles);
    if(ret != EFI_SUCCESS)
           return;

    /* Just use the first handle. */
    ret = efi_open_protocol(handles[0], &serial_io_guid,
           EFI_OPEN_PROTOCOL_GET_PROTOCOL, (void **)&serial_io);
    if(ret != EFI_SUCCESS)
           return;

    /* Configure the port. */
    ret = efi_call(serial_io->set_attributes, serial_io, SERIAL_BAUD_RATE,
           0, 0, SERIAL_PARITY, SERIAL_DATA_BITS, SERIAL_STOP_BITS);
    if(ret != EFI_SUCCESS)
           return;

    ret = efi_call(serial_io->set_control, serial_io,
           EFI_SERIAL_DATA_TERMINAL_READY | EFI_SERIAL_REQUEST_TO_SEND);
    if(ret != EFI_SUCCESS)
           return;

    debug_console.out = &efi_serial_out_ops;
}
