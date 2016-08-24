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
 * @brief               Console functions.
 */

#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <lib/list.h>

struct console_in;
struct console_out;

/** Console draw region structure. */
typedef struct draw_region {
  uint16_t x;                         /**< X position. */
  uint16_t y;                         /**< Y position. */
  uint16_t width;                     /**< Width of region. */
  uint16_t height;                    /**< Height of region. */
  bool scrollable;                    /**< Whether to scroll when cursor reaches the end. */
} draw_region_t;

/** Console color definitions (match VGA colors). */
typedef enum color {
  COLOR_BLACK,                       /**< Black. */
  COLOR_BLUE,                        /**< Dark Blue. */
  COLOR_GREEN,                       /**< Dark Green. */
  COLOR_CYAN,                        /**< Dark Cyan. */
  COLOR_RED,                         /**< Dark Red. */
  COLOR_MAGENTA,                     /**< Dark Magenta. */
  COLOR_BROWN,                       /**< Brown. */
  COLOR_LIGHT_GREY,                  /**< Light Grey. */
  COLOR_GREY,                        /**< Dark Grey. */
  COLOR_LIGHT_BLUE,                  /**< Light Blue. */
  COLOR_LIGHT_GREEN,                 /**< Light Green. */
  COLOR_LIGHT_CYAN,                  /**< Light Cyan. */
  COLOR_LIGHT_RED,                   /**< Light Red. */
  COLOR_LIGHT_MAGENTA,               /**< Light Magenta. */
  COLOR_YELLOW,                      /**< Yellow. */
  COLOR_WHITE,                       /**< White. */

  /**
   * Default foreground/background colors.
   *
   * This is defined separately to allow, for example, on serial consoles for
   * the colors to be set back to defaults for the console. For other console
   * implementations, it simply sets the defaults defined below.
   */
  COLOR_DEFAULT,
} color_t;

/** Default console colors. */
#define CONSOLE_COLOR_FG       COLOR_LIGHT_GREY
#define CONSOLE_COLOR_BG       COLOR_BLACK

/** Console output operations structure. */
typedef struct console_out_ops {
  /**
   * Initialization operations.
   */

  /**
   * Initialize the console when it is made active.
   *
   * @param console       Console output device.
   */
  void (*init)(struct console_out *console);

  /**
   * Deinitialize the console when it is being made inactive.
   *
   * @param console       Console output device.
   */
  void (*deinit)(struct console_out *console);

  /**
   * Basic operations.
   */

  /**
   * Write a character to the console.
   *
   * @param console       Console output device.
   * @param ch            Character to write.
   */
  void (*putc)(struct console_out *console, char ch);

  /**
   * Set the current colors (optional).
   *
   * @param console       Console output device.
   * @param fg            Foreground color.
   * @param bg            Background color.
   */
  void (*set_color)(struct console_out *console, color_t fg, color_t bg);

  /**
   * UI operations.
   */
  
  /**
   * Begin UI mode (optional).
   *
   * @param console   Console output device.
   */
  void (*begin_ui)(struct console_out *console);

  /**
   * End UI mode (optional).
   *
   * @param console   Console output device.
   */
  void (*end_ui)(struct console_out *console);

  /**
   * Set the draw region of the console.
   *
   * Sets the draw region of the console. All operations on the console (i.e.
   * writing, scrolling) will be constrained to this region. The cursor will
   * be moved to 0, 0 within this region.
   *
   * @param console       Console output device.
   * @param region        New draw region, or NULL to restore to whole console.
   */
  void (*set_region)(struct console_out *console, const draw_region_t *region);

  /**
   * Get the current draw region.
   *
   * @param console       Console output device.
   * @param region        Where to store details of the current draw region.
   */
  void (*get_region)(struct console_out *console, draw_region_t *region);

  /**
   * Set the cursor properties.
   *
   * @param console       Console output device.
   * @param x             New X position (relative to draw region). Negative
   *                      values will move the cursor back from the right edge
   *                      of the draw region.
   * @param y             New Y position (relative to draw region). Negative
   *                      values will move the cursor up from the bottom edge
   *                      of the draw region.
   * @param visible       Whether the cursor should be visible.
   */
  void (*set_cursor)(struct console_out *console, int16_t x, int16_t y, bool visible);

  /**
   * Get the cursor properties.
   *
   * @param console       Console output device.
   * @param _x            Where to store X position (relative to draw region).
   * @param _y            Where to store Y position (relative to draw region).
   * @param _visible      Where to store whether the cursor is visible
   */
  void (*get_cursor)(struct console_out *console, uint16_t *_x, uint16_t *_y, bool *_visible);

  /**
   * Clear an area to the current background color.
   *
   * @param console       Console output device.
   * @param x             Start X position (relative to draw region).
   * @param y             Start Y position (relative to draw region).
   * @param width         Width of the area (if 0, whole width is cleared).
   * @param height        Height of the area (if 0, whole height is cleared).
   */
  void (*clear)(struct console_out *console, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

  /**
   * Scroll the draw region up (move contents down).
   *
   * @param console       Console output device.
   */
  void (*scroll_up)(struct console_out *console);

  /**
   * Scroll the draw region down (move contents up).
   *
   * @param console       Console output device.
   */
  void (*scroll_down)(struct console_out *console);
} console_out_ops_t;

/** Console output structure (embedded in implementation-specific structure) */
typedef struct console_out {
  const console_out_ops_t *ops; /**< output operations. */
  bool in_ui;                   /**< whether in UI mode */
} console_out_t;

/** Special key codes. */
#define CONSOLE_KEY_UP          0x100
#define CONSOLE_KEY_DOWN        0x101
#define CONSOLE_KEY_LEFT        0x102
#define CONSOLE_KEY_RIGHT       0x103
#define CONSOLE_KEY_HOME        0x104
#define CONSOLE_KEY_END         0x105
#define CONSOLE_KEY_F1          0x106
#define CONSOLE_KEY_F2          0x107
#define CONSOLE_KEY_F3          0x108
#define CONSOLE_KEY_F4          0x109
#define CONSOLE_KEY_F5          0x10a
#define CONSOLE_KEY_F6          0x10b
#define CONSOLE_KEY_F7          0x10c
#define CONSOLE_KEY_F8          0x10d
#define CONSOLE_KEY_F9          0x10e
#define CONSOLE_KEY_F10         0x10f

/** Console input operations structure. */
typedef struct console_in_ops {
  /**
   * Initialize the console when it is made active.
   *
   * @param console       Console input device.
   */
  void *(*init)(struct console_in *console);

  /**
   * Deinitialize the console when it is being made inactive.
   *
   * @param console       Console input device.
   */
  void (*deinit)(struct console_in *console);

  /**
   * Check for a character from the console.
   *
   * @param console       Console input device.
   * @return              Whether a character is available.
   */
  bool (*poll)(struct console_in *console);

  /**
   * Read a character from the console.
   *
   * @param console       Console input device.
   * @return              Character read.
   */
  uint16_t (*getc)(struct console_in *console);
} console_in_ops_t;

/** Console input structure (embedded in implementation-specific structure). */
typedef struct console_in {
  const console_in_ops_t *ops;    /**< Input operations. */
} console_in_t;

/**
 * Structure describing a console.
 *
 * A console is a named combination of an output and input device. The reason
 * for splitting output and input is that we may have separate code for handling
 * input and output. For example, on EFI  we have output via the framebuffer,
 * but input via a EFI console.
 */
typedef struct console {
  list_t header;

  const char *name;                   /**< Name of the console. */
  console_out_t *out;                 /**< Output device. */
  console_in_t *in;                   /**< input device. */
} console_t;

/** Console capabilities. */
#define CONSOLE_CAP_OUT         (1<<0)  /**< Console supports basic output. */
#define CONSOLE_CAP_IN          (1<<1)  /**< Console supports input. */
#define CONSOLE_CAP_UI          (1<<2)  /**< Console supports the user interface. */

extern console_t primary_console;

extern console_t *current_console;
extern console_t *debug_console;

extern bool console_has_caps(console_t *console, unsigned caps);

extern void console_putc(console_t *console, char ch);
extern void console_set_color(console_t *console, color_t fg, color_t bg);
extern void console_begin_ui(console_t *console);
extern void console_end_ui(console_t *console);
extern void console_set_region(console_t *console, const draw_region_t *region);
extern void console_get_region(console_t *console, draw_region_t *region);
extern void console_set_cursor(console_t *console, int16_t x, int16_t y, bool visible);
extern void console_get_cursor(console_t *console, uint16_t *_x, uint16_t *_y, bool *_visible);
extern void console_clear(console_t *console, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
extern void console_scroll_up(console_t *console);
extern void console_scroll_down(console_t *console);
extern bool console_poll(console_t *console);
extern uint16_t console_getc(console_t *console);

extern void console_vprintf_helper(char ch, void *data, int *total);
extern int console_vprintf(console_t *console, const char *fmt, va_list args);
extern int console_printf(console_t *console, const char *fmt, ...) __printf(2, 3);

extern console_t *console_lookup(const char *name);
extern void console_register(console_t *console);
extern void console_set_current(console_t *console);
extern void console_set_debug(console_t *console);

extern void target_console_init(void);

extern void console_init(void);

#ifdef CONFIG_TARGET_HAS_UI
extern void debug_log_display(void);
#endif // CONFIG_TARGET_HAS_UI

#endif // __CONSOLE_H