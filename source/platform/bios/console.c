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
 * @brief               BIOS platform console functions.
 */

#include <arch/io.h>

#include <bios/bios.h>

#include <drivers/serial/ns16550.h>

#include <lib/utility.h>

#include <console.h>
#include <loader.h>

/** Serial port definitions. */
#define SERIAL_CLOCK        1843200

/** Array of serial port numbers. */
static uint16_t serial_ports[] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8 };

/**
 * Check for a character from the console.
 *
 * @param console       Console input device.
 *
 * @return              Whether a character is available.
 */
static bool bios_console_poll(console_in_t *console) {
    bios_regs_t regs;

    bios_regs_init(&regs);
    regs.eax = 0x0100;
    bios_call(0x16, &regs);
    return !(regs.eflags & X86_FLAGS_ZF);
}

/**
 * Read a character from the console.
 *
 * @param console       Console input device.
 *
 * @return              Character read.
 */
static uint16_t bios_console_getc(console_in_t *console) {
    bios_regs_t regs;
    uint8_t ascii, scan;

    bios_regs_init(&regs);

    /* INT16 AH=00h on Apple's BIOS emulation will hang forever if there is no
     * key available, so loop trying to poll for one first. */
    do {
        regs.eax = 0x0100;
        bios_call(0x16, &regs);
    } while (regs.eflags & X86_FLAGS_ZF);

    /* Get the key code. */
    regs.eax = 0x0000;
    bios_call(0x16, &regs);

    /* Convert certain scan codes to special keys. */
    ascii = regs.eax & 0xff;
    scan = (regs.eax >> 8) & 0xff;
    switch (scan) {
    case 0x48:
        return CONSOLE_KEY_UP;
    case 0x50:
        return CONSOLE_KEY_DOWN;
    case 0x4b:
        return CONSOLE_KEY_LEFT;
    case 0x4d:
        return CONSOLE_KEY_RIGHT;
    case 0x47:
        return CONSOLE_KEY_HOME;
    case 0x4f:
        return CONSOLE_KEY_END;
    case 0x53:
        return 0x7f;
    case 0x3b ... 0x44:
        return CONSOLE_KEY_F1 + (scan - 0x3b);
    default:
        /* Convert CR to LF. */
        return (ascii == '\r') ? '\n' : ascii;
    }
}

/** BIOS console input operations. */
static console_in_ops_t bios_console_in_ops = {
    .poll = bios_console_poll,
    .getc = bios_console_getc,
};

/** BIOS console input device. */
static console_in_t bios_console_in = {
    .ops = &bios_console_in_ops,
};

/** 
 * Initialize the console.
 */
void target_console_init(void) {
    serial_config_t config;

    config.baud_rate = SERIAL_DEFAULT_BAUD_RATE;
    config.data_bits = SERIAL_DEFAULT_DATA_BITS;
    config.parity = SERIAL_DEFAULT_PARITY;
    config.stop_bits = SERIAL_DEFAULT_STOP_BITS;

    // register serial ports
    for (size_t i = 0; i < array_size(serial_ports); i++) {
        serial_port_t *port = ns16550_register(serial_ports[i], i, SERIAL_CLOCK);

        if (port) {
            serial_port_config(port, &config);

            // register the first as the debug console
            if (i == 0) { console_set_debug(&port->console); }
        }
    }

    // use BIOS for input
    primary_console.in = &bios_console_in;
}
