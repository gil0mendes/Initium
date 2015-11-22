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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * @brief               PC platform console functions.
 */

#include <arch/io.h>

#include <drivers/serial/ns16550.h>

#include <test.h>

INITIUM_VIDEO(INITIUM_VIDEO_VGA | INITIUM_VIDEO_LFB, 0, 0, 0);

/** Serial port definitions. */
#define SERIAL_PORT         0x3f8
#define SERIAL_CLOCK        1843200
#define SERIAL_BAUD_RATE    115200

/** Initialize the debug console. */
void debug_console_init(void) {
    uint8_t status;

    /* Initialize the serial port as the debug console. TODO: Disable for
     * non-debug builds? */
    status = in8(SERIAL_PORT + 6);
    if ((status & ((1 << 4) | (1 << 5))) && status != 0xff) {
        ns16550_init(SERIAL_PORT);
        ns16550_config(SERIAL_PORT, SERIAL_CLOCK, SERIAL_BAUD_RATE);
    }
}