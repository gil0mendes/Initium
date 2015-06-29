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

/** Console draw region structure. */
typedef struct draw_region {
    uint16_t x;                         /**< X position. */
    uint16_t y;                         /**< Y position. */
    uint16_t width;                     /**< Width of region. */
    uint16_t height;                    /**< Height of region. */
    bool scrollable;                    /**< Whether to scroll when cursor reaches the end. */
} draw_region_t;

/** Console colour definitions (match VGA colours). */
typedef enum colour {
    COLOUR_BLACK,                       /**< Black. */
    COLOUR_BLUE,                        /**< Dark Blue. */
    COLOUR_GREEN,                       /**< Dark Green. */
    COLOUR_CYAN,                        /**< Dark Cyan. */
    COLOUR_RED,                         /**< Dark Red. */
    COLOUR_MAGENTA,                     /**< Dark Magenta. */
    COLOUR_BROWN,                       /**< Brown. */
    COLOUR_LIGHT_GREY,                  /**< Light Grey. */
    COLOUR_GREY,                        /**< Dark Grey. */
    COLOUR_LIGHT_BLUE,                  /**< Light Blue. */
    COLOUR_LIGHT_GREEN,                 /**< Light Green. */
    COLOUR_LIGHT_CYAN,                  /**< Light Cyan. */
    COLOUR_LIGHT_RED,                   /**< Light Red. */
    COLOUR_LIGHT_MAGENTA,               /**< Light Magenta. */
    COLOUR_YELLOW,                      /**< Yellow. */
    COLOUR_WHITE,                       /**< White. */
} colour_t;

/** Default console colours. */
#define CONSOLE_COLOUR_FG       COLOUR_LIGHT_GREY
#define CONSOLE_COLOUR_BG       COLOUR_BLACK

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

    /**
     * Set the draw region of the console.
     *
     * Sets the draw region of the console. All operations on the console (i.e.
     * writing, scrolling) will be constrained to this region. The cursor will
     * be moved to 0, 0 within this region.
     *
     * @param private       Private data for the console.
     * @param region        New draw region, or NULL to restore to whole console.
     */
    void (*set_region)(void *private, const draw_region_t *region);

    /** Get the current draw region.
     * @param private       Private data for the console.
     * @param region        Where to store details of the current draw region. */
    void (*get_region)(void *private, draw_region_t *region);

    /** Set the current colours.
     * @param private       Private data for the console.
     * @param fg            Foreground colour.
     * @param bg            Background colour. */
    void (*set_colour)(void *private, colour_t fg, colour_t bg);

    /** Set the cursor properties.
     * @param private       Private data for the console.
     * @param x             New X position (relative to draw region). Negative
     *                      values will move the cursor back from the right edge
     *                      of the draw region.
     * @param y             New Y position (relative to draw region). Negative
     *                      values will move the cursor up from the bottom edge
     *                      of the draw region.
     * @param visible       Whether the cursor should be visible. */
    void (*set_cursor)(void *private, int16_t x, int16_t y, bool visible);

    /** Get the cursor properties.
     * @param private       Private data for the console.
     * @param _x            Where to store X position (relative to draw region).
     * @param _y            Where to store Y position (relative to draw region).
     * @param _visible      Where to store whether the cursor is visible */
    void (*get_cursor)(void *private, uint16_t *_x, uint16_t *_y, bool *_visible);

    /** Clear an area to the current background colour.
     * @param private       Private data for the console.
     * @param x             Start X position (relative to draw region).
     * @param y             Start Y position (relative to draw region).
     * @param width         Width of the area (if 0, whole width is cleared).
     * @param height        Height of the area (if 0, whole height is cleared). */
    void (*clear)(void *private, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

    /** Scroll the draw region up (move contents down).
     * @param private       Private data for the console. */
    void (*scroll_up)(void *private);

    /** Scroll the draw region down (move contents up).
     * @param private       Private data for the console. */
    void (*scroll_down)(void *private);

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

extern void console_vprintf_helper(char ch, void *data, int *total);
extern int console_vprintf(console_t *console, const char *fmt, va_list args);
extern int console_printf(console_t *console, const char *fmt, ...) __printf(2, 3);

/**
 * Write a character to a console.
 *
 * Writes a character to a console. If the console has no output operations
 * defined, this function will do nothing.
 *
 * @param console       Console to write to.
 * @param ch            Character to write.
 */
static inline void console_putc(console_t *console, char ch) {
    if (console->out)
        console->out->putc(console->out_private, ch);
}

/** Reset the console to a default state.
 * @param console       Console to operate on. */
static inline void console_reset(console_t *console) {
    if (console->out)
        console->out->reset(console->out_private);
}

/**
 * Set the draw region of the console.
 *
 * Sets the draw region of the console. All operations on the console (i.e.
 * writing, scrolling) will be constrained to this region. The cursor will
 * be moved to 0, 0 within this region.
 *
 * @param console       Console to operate on (must have out ops).
 * @param region        New draw region, or NULL to restore to whole console.
 */
static inline void console_set_region(console_t *console, const draw_region_t *region) {
    console->out->set_region(console->out_private, region);
}

/** Get the current draw region.
 * @param console       Console to operate on (must have out ops).
 * @param region        Where to store details of the current draw region. */
static inline void console_get_region(console_t *console, draw_region_t *region) {
    console->out->get_region(console->out_private, region);
}

/** Set the current colours.
 * @param console       Console to operate on (must have out ops).
 * @param fg            Foreground colour.
 * @param bg            Background colour. */
static inline void console_set_colour(console_t *console, colour_t fg, colour_t bg) {
    console->out->set_colour(console->out_private, fg, bg);
}

/** Set the cursor properties.
 * @param console       Console to operate on (must have out ops).
 * @param x             New X position (relative to draw region). Negative
 *                      values will move the cursor back from the right edge
 *                      of the draw region.
 * @param y             New Y position (relative to draw region). Negative
 *                      values will move the cursor up from the bottom edge
 *                      of the draw region.
 * @param visible       Whether the cursor should be visible. */
static inline void console_set_cursor(console_t *console, int16_t x, int16_t y, bool visible) {
    console->out->set_cursor(console->out_private, x, y, visible);
}

/** Get the cursor properties.
 * @param console       Console to operate on (must have out ops).
 * @param _x            Where to store X position (relative to draw region).
 * @param _y            Where to store Y position (relative to draw region).
 * @param _visible      Where to store whether the cursor is visible */
static inline void console_get_cursor(console_t *console, uint16_t *_x, uint16_t *_y, bool *_visible) {
    console->out->get_cursor(console->out_private, _x, _y, _visible);
}

/** Clear an area to the current background colour.
 * @param console       Console to operate on (must have out ops).
 * @param x             Start X position (relative to draw region).
 * @param y             Start Y position (relative to draw region).
 * @param width         Width of the area (if 0, whole width is cleared).
 * @param height        Height of the area (if 0, whole height is cleared). */
static inline void console_clear(console_t *console, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    console->out->clear(console->out_private, x, y, width, height);
}

/** Scroll the draw region up (move contents down).
 * @param console       Console to operate on (must have out ops). */
static inline void console_scroll_up(console_t *console) {
    console->out->scroll_up(console->out_private);
}

/** Scroll the draw region down (move contents up).
 * @param console       Console to operate on (must have out ops). */
static inline void console_scroll_down(console_t *console) {
    console->out->scroll_down(console->out_private);
}

/** Check for a character from a console.
 * @param console       Console to operate on (must have in ops).
 * @return              Whether a character is available. */
static inline bool console_poll(console_t *console) {
    return console->in->poll(console->in_private);
}

/** Read a character from a console.
 * @param console       Console to operate on (must have in ops).
 * @return              Character read. */
static inline uint16_t console_getc(console_t *console) {
    return console->in->getc(console->in_private);
}

#endif /* __CONSOLE_H */
