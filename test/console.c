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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include <drivers/console/fb.h>
#include <drivers/console/vga.h>

#include <lib/ctype.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <lib/utility.h>

#include <time.h>
#include <test.h>
#include <video.h>
#include <memory.h>

/** Initium log buffer. */
static initium_log_t *initium_log = NULL;
static size_t initium_log_size = 0;

/** primary console */
console_t primary_console;

/** current primary console */
console_t *current_console = &primary_console;

/** debug output console */
console_t *debug_console;

/** Framebuffer video mode information. */
video_mode_t *current_video_mode;

/**
 * Write a character to a console.
 *
 * @param console       Console to write to.
 * @param ch            Character to write.
 */
void console_putc(console_t *console, char ch) {
  if (console && console->out) { console->out->ops->putc(console->out, ch); }
}

/**
 * Helper for console_vprintf().
 *
 * @param ch    Character to display.
 * @param data  Console to use.
 * @param total Pointer to total character count.
 */
void console_vprintf_helper(char ch, void *data, int *total) {
  console_t *console = data;

  console_putc(console, ch);
  *total = *total + 1;
}

/**
 * Output a formatted message to a console.
 *
 * @param  console Console to print to.
 * @param  fmt     Format string used to create the message.
 * @param  args    Arguments to substitute into format.
 * @return         Number of characters printed.
 */
int console_vprintf(console_t *console, const char *fmt, va_list args) {
  return do_vprintf(console_vprintf_helper, console, fmt, args);
}

/**
 * Output a formatted message to a console.
 *
 * @param console       Console to print to.
 * @param fmt           Format string used to create the message.
 * @param ...           Arguments to substitute into format.
 * @return              Number of characters printed.
 */
int console_printf(console_t *console, const char *fmt, ...) {
  va_list args;
  int ret;

  va_start(args, fmt);
  ret = console_vprintf(console, fmt, args);
  va_end(args);

  return ret;
}

/** Helper for vprintf().
 * @param ch            Character to display.
 * @param data          Unused.
 * @param total         Pointer to total character count. */
static void vprintf_helper(char ch, void *data, int *total) {
  console_putc(current_console, ch);
  console_putc(debug_console, ch);

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

  while (true) { arch_pause(); }
}

/**
 * Initialize the primary console.
 *
 * @param tags          Tag list.
 */
void primary_console_init(initium_tag_t *tags) {
  initium_tag_video_t *video;

  log_init(tags);

  while (tags->type != INITIUM_TAG_NONE) {
    if (tags->type == INITIUM_TAG_VIDEO) {
      console_out_t *console = NULL;

      video = (initium_tag_video_t *)tags;

      if (video->type == INITIUM_VIDEO_LFB && video->lfb.flags & INITIUM_LFB_RGB) {
        current_video_mode = malloc(sizeof(*current_video_mode));
        current_video_mode->type = VIDEO_MODE_LFB;
        current_video_mode->width = video->lfb.width;
        current_video_mode->height = video->lfb.height;
        current_video_mode->bpp = video->lfb.bpp;
        current_video_mode->pitch = video->lfb.pitch;
        current_video_mode->red_size = video->lfb.red_size;
        current_video_mode->red_pos = video->lfb.red_pos;
        current_video_mode->green_size = video->lfb.green_size;
        current_video_mode->green_pos = video->lfb.green_pos;
        current_video_mode->blue_size = video->lfb.blue_size;
        current_video_mode->blue_pos = video->lfb.blue_pos;
        current_video_mode->mem_phys = video->lfb.fb_phys;
        current_video_mode->mem_virt = video->lfb.fb_virt;
        current_video_mode->mem_size = video->lfb.fb_size;

        console = fb_console_create();
      }

      #ifdef CONFIG_ARCH_X86
      if (video->type == INITIUM_VIDEO_VGA) {
        current_video_mode = malloc(sizeof(*current_video_mode));
        current_video_mode->type = VIDEO_MODE_VGA;
        current_video_mode->width = video->vga.cols;
        current_video_mode->height = video->vga.lines;
        current_video_mode->x = video->vga.x;
        current_video_mode->y = video->vga.y;
        current_video_mode->mem_phys = video->vga.mem_phys;
        current_video_mode->mem_virt = video->vga.mem_virt;
        current_video_mode->mem_size = video->vga.mem_size;

        console = vga_console_create();
      }
      #endif

      if (console && console->ops->init) { console->ops->init(console); }

      primary_console.out = console;
      break;
    }

    tags = (initium_tag_t *)round_up((ptr_t)tags + tags->size, 8);
  }
}

/**
 * Compatibility stubs.
 */

void console_register(console_t *console) {
  // nothing happens
}

mstime_t current_time(void) { return 0; }