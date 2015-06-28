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
 * @brief               Console functions.
 */

#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <types.h>

struct video_mode;

/** Console output operations structure. */
typedef struct console_out_ops {
    /** Initialize the console.
     * @param mode          Video mode being used.
     * @return              Private data for the console. */
    void *(*init)(struct video_mode *mode);

    /** Deinitialize the console before changing video modes.
     * @param private       Private data for the console. */
    void (*deinit)(void *private);

    /** Reset the console to a default state.
     * @param private       Private data for the console. */
    void (*reset)(void *private);

    /** Write a character to the console.
     * @param private       Private data for the console.
     * @param ch            Character to write. */
    void (*putc)(void *private, char ch);
} console_out_ops_t;

/** Special key codes. */
#define CONSOLE_KEY_UP      0x100
#define CONSOLE_KEY_DOWN    0x101
#define CONSOLE_KEY_LEFT    0x102
#define CONSOLE_KEY_RIGHT   0x103
#define CONSOLE_KEY_HOME    0x104
#define CONSOLE_KEY_END     0x105
#define CONSOLE_KEY_F1      0x106
#define CONSOLE_KEY_F2      0x107
#define CONSOLE_KEY_F3      0x108
#define CONSOLE_KEY_F4      0x109
#define CONSOLE_KEY_F5      0x10a
#define CONSOLE_KEY_F6      0x10b
#define CONSOLE_KEY_F7      0x10c
#define CONSOLE_KEY_F8      0x10d
#define CONSOLE_KEY_F9      0x10e
#define CONSOLE_KEY_F10     0x10f

/** Console input operations structure. */
typedef struct console_in_ops {
    /** Check for a character from the console.
     * @param private       Private data for the console.
     * @return              Whether a character is available. */
    bool (*poll)(void *private);

    /** Read a character from the console.
     * @param private       Private data for the console.
     * @return              Character read. */
    uint16_t (*getc)(void *private);
} console_in_ops_t;

/**
 * Structure describing a console.
 *
 * A console is separated into output and input operations. The reason for this
 * is that we may have separate code for handling input and output. For example,
 * on the BIOS platform's main console we have output via VGA, but input via
 * BIOS calls.
 */
typedef struct console {
    const console_out_ops_t *out;       /**< Output operations. */
    void *out_private;                  /**< Private data for output handler. */
    const console_in_ops_t *in;         /**< Input operations. */
    void *in_private;                   /**< Private data for input handler. */
} console_t;

/** Debug log size. */
#define DEBUG_LOG_SIZE      8192

extern char debug_log[DEBUG_LOG_SIZE];
extern size_t debug_log_start;
extern size_t debug_log_length;

extern console_t main_console;
extern console_t debug_console;

/**
 * Write a character to a console.
 *
 * Writes a character to a console. If the console has no output operations
 * defined, this function will do nothing.
 *
 * @param console           Console to write to.
 * @param ch                Character to write.
 */
static inline void console_putc(console_t *console, char ch) {
    if (console->out)
        console->out->putc(console->out_private, ch);
}

extern void console_vprintf_helper(char ch, void *data, int *total);
extern int console_vprintf(console_t *console, const char *fmt, va_list args);
extern int console_printf(console_t *console, const char *fmt, ...) __printf(2, 3);

#endif /* __CONSOLE_H */
