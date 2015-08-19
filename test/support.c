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
 * @brief               Test kernel support functions.
 */

#include <lib/ctype.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <lib/utility.h>

#include <memory.h>

#include "test.h"

/** Size of the heap. */
#define HEAP_SIZE       32768

/** Statically allocated heap. */
static uint8_t heap[HEAP_SIZE] __aligned(PAGE_SIZE);
static size_t heap_offset;

/** Initium log buffer. */
static initium_log_t *initium_log = NULL;
static size_t initium_log_size = 0;

/** Main console. */
console_t main_console;

/** Debug console. */
console_t debug_console;

/** Helper for vprintf().
 * @param ch            Character to display.
 * @param data          Unused.
 * @param total         Pointer to total character count. */
static void vprintf_helper(char ch, void *data, int *total) {
    console_putc(&main_console, ch);
    console_putc(&debug_console, ch);

    if (initium_log) {
        initium_log->buffer[(initium_log->start + initium_log->length) % initium_log_size] = ch;
        if (initium_log->length < initium_log_size) {
            initium_log->length++;
        } else {
            initium_log->start = (initium_log->start + 1) % initium_log_size;
        }
    }

    *total = *total + 1;
}

/** Output a formatted message to the console.
 * @param fmt           Format string used to create the message.
 * @param args          Arguments to substitute into format.
 * @return              Number of characters printed. */
int vprintf(const char *fmt, va_list args) {
    return do_vprintf(vprintf_helper, NULL, fmt, args);
}

/** Output a formatted message to the console.
 * @param fmt           Format string used to create the message.
 * @param ...           Arguments to substitute into format.
 * @return              Number of characters printed. */
int printf(const char *fmt, ...) {
    va_list args;
    int ret;

    va_start(args, fmt);
    ret = vprintf(fmt, args);
    va_end(args);

    return ret;
}

/** Initialize the log.
 * @param tags          Tag list. */
void log_init(initium_tag_t *tags) {
    initium_tag_log_t *log;

    while (tags->type != INITIUM_TAG_NONE) {
        if (tags->type == INITIUM_TAG_LOG) {
            log = (initium_tag_log_t *)tags;
            initium_log = (initium_log_t *)((ptr_t)log->log_virt);
            initium_log_size = log->log_size - sizeof(initium_log_t);
            break;
        }

        tags = (initium_tag_t *)round_up((ptr_t)tags + tags->size, 8);
    }
}

/** Allocate memory from the heap.
 * @param size          Size of allocation to make.
 * @return              Address of allocation. */
void *malloc(size_t size) {
    size_t offset = heap_offset;
    size = round_up(size, 8);
    heap_offset += size;
    return (void *)(heap + offset);
}

/** Free memory from the heap.
 * @param addr          Address to free. */
void free(void *addr) {
    /* Nope. */
}
