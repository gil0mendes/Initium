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
 * @brief 				Console functions
 */

#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <types.h>

struct video_mode;

// Structure describing a draw region
typedef struct draw_region {
	int x;				// X position
	int y;				// Y position
	int width;			// Width of region
	int height;			// Height of region
	bool scrollable;	// Whether to scroll when cursor reaches the end
} draw_region_t;

// Console output operations structure
typedef struct console_out_ops {
	/**
	 * Initialize the console.
	 *
	 * @param model 	Video mode being used.
	 */
	void (*init)(struct video_mode *mode);

	/**
	 * Deinitialize the console before changing video modes.
	 */
	void (*deinit)(void);

	// Reset the console to a default state
	void (*reset)(void);

	// Write a character to the console.
	//
	// @param ch		Character to write
	void (*putc)(char ch);

	// Set the draw region
	//
	// @note 			Positions the cursor at 0, 0 in the new region
	// @param region	Structure describing the region
	void (*setRegion)(draw_region_t *region);

	// Get the VGA console draw region.
	//
	// @param region	Region structure to fill in
	void(*getRegion)(draw_region_t *region);

	// Clear a portion of the console.
	//
	// @note			Position is relative to the draw region.
	// @param x			Start X position.
	// @param y			Start Y position.
	// @param width		Width of the highlight.
	// @param height	Height of the highlight.
	void (*clear)(int x, int y, int width, int height);

	// Set whether the cursor is visible
	//
	// @param visible 		Whether the cursor is visible
	void (*showCursor)(bool visible);

	// Change the highlight on a portation of the console
	//
	// @note 				Position is relative to the draw region
	// @param x		Start X position.
	// @param y		Start Y position.
	// @param width		Width of the highlight.
	// @param height	Height of the highlight.
	void (*highlight)(int x, int y, int width, int height);

	// Move the cursor.
	//
	// @note		Position is relative to the draw region.
	// @param x		New X position.
	// @param y		New Y position.
	void (*moveCursor)(int x, int y);

	// Scroll the console up by one row
	void (*scrollUp)(void);

	// Scroll the console down by one row
	void (*scrollDown)(void);
} console_out_ops_t;

// Console input operations structure
typedef struct console_in_ops {
	// Check for a character from the console.
	//
	// @return		Whether a character is available.
	bool (*poll)(void);

	// Read a character from the console.
	//
	// @return		Character read.
	uint16_t (*getc)(void);
} console_in_ops_t;

// Structure describing a console
typedef struct console {
	int width;								/**< Width of the console (columns) */
	int height; 							/**< Height of the console (rows) */

	const console_out_ops_t *out;           /**< Output operations. */
	const console_in_ops_t *in;             /**< Input operations. */
} console_t;


// Special key codes
#define CONSOLE_KEY_UP		0x100
#define CONSOLE_KEY_DOWN	0x101
#define CONSOLE_KEY_LEFT	0x102
#define CONSOLE_KEY_RIGHT	0x103
#define CONSOLE_KEY_HOME	0x104
#define CONSOLE_KEY_END		0x105
#define CONSOLE_KEY_F1		0x106
#define CONSOLE_KEY_F2		0x107
#define CONSOLE_KEY_F3		0x108
#define CONSOLE_KEY_F4		0x109
#define CONSOLE_KEY_F5		0x10a
#define CONSOLE_KEY_F6		0x10b
#define CONSOLE_KEY_F7		0x10c
#define CONSOLE_KEY_F8		0x10d
#define CONSOLE_KEY_F9		0x10e
#define CONSOLE_KEY_F10		0x10f

// Debug log size
#define DEBUG_LOG_SIZE		8192

extern char debug_log[DEBUG_LOG_SIZE];
extern size_t debug_log_start;
extern size_t debug_log_length;

extern console_t main_console;
extern console_t debug_console;

extern void console_vprintf_helper(char ch, void *data, int *total);
extern int console_vprintf(console_t *console, const char *fmt, va_list args);
extern int console_printf(console_t *console, const char *fmt, ...) __printf(2, 3);

#endif // __CONSOLE_H
