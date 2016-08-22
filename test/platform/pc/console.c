/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Gil Mendes <gil00mendes@gmail.com>
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

/** Initialize the debug console. */
void debug_console_init(void) {
  serial_port_t *port = ns16550_register(SERIAL_PORT, 0, SERIAL_CLOCK);

  if (port) { debug_console = &port->console; }
}