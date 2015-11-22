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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

/**
 * @file
 * @brief               Test kernel console functions.
 */

#include <drivers/video/fb.h>
#include <drivers/video/vga.h>

#include <lib/ctype.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <lib/utility.h>

#include <test.h>
#include <video.h>

/** Initium log buffer. */
static initium_log_t *initium_log = NULL;
static size_t initium_log_size = 0;

/** Framebuffer video mode information. */
static video_mode_t video_mode;

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
static void log_init(initium_tag_t *tags) {
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

/** Raise an internal error.
 * @param fmt           Error format string.
 * @param ...           Values to substitute into format. */
void __noreturn internal_error(const char *fmt, ...) {
    va_list args;

    printf("Internal Error: ");

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");

    while (true)
        arch_pause();
}

/**
 * Initialize the console.
 *
 * @param tags          Tag list.
 */
void console_init(initium_tag_t *tags) {
    initium_tag_video_t *video;

    log_init(tags);

    while (tags->type != INITIUM_TAG_NONE) {
        if (tags->type == INITIUM_TAG_VIDEO)
        {
            video = (initium_tag_video_t *)tags;

            if (video->type == INITIUM_VIDEO_LFB && video->lfb.flags & INITIUM_LFB_RGB)
            {
                video_mode.type = VIDEO_MODE_LFB;
                video_mode.width = video->lfb.width;
                video_mode.height = video->lfb.height;
                video_mode.bpp = video->lfb.bpp;
                video_mode.pitch = video->lfb.pitch;
                video_mode.red_size = video->lfb.red_size;
                video_mode.red_pos = video->lfb.red_pos;
                video_mode.green_size = video->lfb.green_size;
                video_mode.green_pos = video->lfb.green_pos;
                video_mode.blue_size = video->lfb.blue_size;
                video_mode.blue_pos = video->lfb.blue_pos;
                video_mode.mem_phys = video->lfb.fb_phys;
                video_mode.mem_virt = video->lfb.fb_virt;
                video_mode.mem_size = video->lfb.fb_size;

                main_console.out_private = fb_console_out_ops.init(&video_mode);
                main_console.out = &fb_console_out_ops;
            }

            #ifdef CONFIG_ARCH_X86
                if (video->type == INITIUM_VIDEO_VGA) {
                    video_mode.type = VIDEO_MODE_VGA;
                    video_mode.width = video->vga.cols;
                    video_mode.height = video->vga.lines;
                    video_mode.x = video->vga.x;
                    video_mode.y = video->vga.y;
                    video_mode.mem_phys = video->vga.mem_phys;
                    video_mode.mem_virt = video->vga.mem_virt;
                    video_mode.mem_size = video->vga.mem_size;

                    main_console.out_private = vga_console_out_ops.init(&video_mode);
                    main_console.out = &vga_console_out_ops;
                }
            #endif

            break;
        }

        tags = (initium_tag_t *)round_up((ptr_t)tags + tags->size, 8);
    }
}