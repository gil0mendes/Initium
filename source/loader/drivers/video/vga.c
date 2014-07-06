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
 * @brief             VGA console implementation.
 */

#include <arch/io.h>

#include <drivers/video/vga.h>

#include <lib/string.h>

#include <console.h>
#include <loader.h>

// Default attributes to use
#define VGA_ATTRIB		0x0700

// VGA memory pointer
static uint16_t *const vga_mapping = (uint16_t *)VGA_MEM_BASE;

// VGA console state
static uint16_t vga_cols;
static uint16_t vga_lines;
static uint16_t vga_cursor_x;
static uint16_t vga_cursor_y;
static bool vga_cursor_visible = true;

// VGA draw region
static draw_region_t vga_region;

/**
 * Write a cell in VGA memory (character + attributes).
 *
 * @param x		X position.
 * @param y		Y position.
 * @param ch		Character to write.
 * @param attrib	Attributes to set.
 */
static inline void write_cell(uint16_t x, uint16_t y, uint16_t val) {
	write16(&vga_mapping[(y * vga_cols) + x], val);
}

/**
 * Write a character at a position, preserving attributes.
 *
 * @param x		X position.
 * @param y		Y position.
 * @param ch		Character to write.
 */
static inline void write_char(uint16_t x, uint16_t y, char ch) {
	uint16_t attrib;

	attrib = read16(&vga_mapping[(y * vga_cols) + x]) & 0xff00;
	write_cell(x, y, attrib | ch);
}

// Update the hardware cursor
static void update_hw_cursor(void) {
	uint16_t x = (vga_cursor_visible) ? vga_cursor_x : 0;
	uint16_t y = (vga_cursor_visible) ? vga_cursor_y : (vga_lines + 1);
	uint16_t pos = (y * vga_cols) + x;

	out8(VGA_CRTC_INDEX, 14);
	out8(VGA_CRTC_DATA, pos >> 8);
	out8(VGA_CRTC_INDEX, 15);
	out8(VGA_CRTC_DATA, pos & 0xff);
}

// Reset the console to a default state
static void vga_console_reset(void) {
	uint16_t i, j;

	vga_cursor_x = vga_cursor_y = 0;
	vga_cursor_visible = true;

	update_hw_cursor();

	for(i = 0; i < vga_lines; i++) {
		for(j = 0; j < vga_cols; j++) {
            write_cell(j, i, ' ' | VGA_ATTRIB);
        }
	}
}

/**
 * Write a character to the console.
 *
 * @param ch		Character to write.
 */
static void vga_console_putc(char ch) {
	uint16_t i;

	switch(ch) {
	case '\b':
		// Backspace, move back one character if we can
		if(vga_cursor_x) {
			vga_cursor_x--;
		} else if(vga_cursor_y) {
			vga_cursor_x = vga_cols - 1;
			vga_cursor_y--;
		}

		break;
	case '\r':
		// Carriage return, move to the start of the line
		vga_cursor_x = 0;
		break;
	case '\n':
		// Newline, treat it as if a carriage return was also there
		vga_cursor_x = 0;
		vga_cursor_y++;
		break;
	case '\t':
		vga_cursor_x += 8 - (vga_cursor_x % 8);
		break;
	default:
		// If it is a non-printing character, ignore it
		if(ch < ' ')
			break;

		write_char(vga_cursor_x, vga_cursor_y, ch);
		vga_cursor_x++;
		break;
	}

	// If we have reached the edge of the screen insert a new line
	if(vga_cursor_x >= vga_cols) {
		vga_cursor_x = 0;
		vga_cursor_y++;
	}

	// Scroll if we've reached the end of the draw region
	if(vga_cursor_y >= vga_lines) {
		// Shift up the content of the VGA memory
		memmove(vga_mapping, vga_mapping + vga_cols, (vga_lines - 1) * vga_cols * 2);

		// Fill the last line with blanks
		for(i = 0; i < vga_cols; i++)
			write_cell(i, vga_lines - 1, ' ' | VGA_ATTRIB);

		vga_cursor_y = vga_lines - 1;
	}

	update_hw_cursor();
}

// ============================================================================
// Set the VGA console draw region
//
// @param region			Region to set
static void
vgaSetRegion(draw_region_t *region) {
	vga_region = *region;
	vga_cursor_x = vga_region.x;
	vga_cursor_y = vga_region.y;
	update_hw_cursor();
}

// ============================================================================
// Get the VGA console draw region.
//
// @param region	Region structure to fill in
static void
vgaGetRegion(draw_region_t *region) {
	*region = vga_region;
}

// ============================================================================
// Clear a portion of the console.
//
// @note			Position is relative to the draw region.
// @param x			Start X position.
// @param y			Start Y position.
// @param width		Width of the highlight.
// @param height	Height of the highlight.
static void
vgaClear(int x, int y, int width, int height) {
	int i, j;

	for(i = vga_region.y + y; i < (vga_region.y + y + height); i++) {
		for(j = vga_region.x + x; j < (vga_region.x + x + width); j++)
			vga_mapping[(i * vga_cols) + j] = ' ' | VGA_ATTRIB;
	}
}

// ============================================================================
// Set whether the cursor is visible
//
// @param visible 			Whether the cursor is visible
static void
vgaShowCursor(bool visible) {
	vga_cursor_visible = visible;
	update_hw_cursor();
}

// ============================================================================
// Change the highlight on a portation of the console
//
// @note 				Position is relative to the draw region
// @param x		Start X position.
// @param y		Start Y position.
// @param width		Width of the highlight.
// @param height	Height of the highlight.
static void
vgaHighlight(int x, int y, int width, int height) {
	uint16_t word, fg, bg;
	int i, j;

	for(i = vga_region.y + y; i < (vga_region.y + y + height); i++) {
		for(j = vga_region.x + x; j < (vga_region.x + x + width); j++) {
			// Swap the foreground/background colour
			word = vga_mapping[(i * vga_cols) + j];
			fg = (word << 4) & 0xF000;
			bg = (word >> 4) & 0x0F00;
			vga_mapping[(i * vga_cols) + j] = (word & 0xFF) | fg | bg;
		}
	}
}

// ============================================================================
// Move the cursor.
//
// @note		Position is relative to the draw region.
// @param x		New X position.
// @param y		New Y position.
static void
vgaMoveCursor(int x, int y) {
	if(x < 0) {
		vga_cursor_x = vga_region.x + vga_region.width + x;
	} else {
		vga_cursor_x = vga_region.x + x;
	}
	if(y < 0) {
		vga_cursor_y = vga_region.y + vga_region.height + y;
	} else {
		vga_cursor_y = vga_region.y + y;
	}

	// Update cursor
	update_hw_cursor();
}

// ============================================================================
// Scroll the console up by one row
static void
vgaScrollUp(void) {
	int i;

	// Shift down the content of the VGA memory
	for(i = 0; i < (vga_region.height - 1); i++) {
		memcpy(vga_mapping + vga_region.x + (vga_cols * (vga_region.y + vga_region.height - i - 1)),
			vga_mapping + vga_region.x + (vga_cols * (vga_region.y + vga_region.height - i - 2)),
			vga_region.width * 2);
	}

	// Fill the first row with blanks
	for(i = 0; i < vga_region.width; i++) {
		vga_mapping[(vga_region.y * vga_cols) + vga_region.x + i] &= 0xFF00;
		vga_mapping[(vga_region.y * vga_cols) + vga_region.x + i] |= ' ';
	}
}

// ============================================================================
// Scroll the console down by one row
static void
vgaScrollDown(void) {
	int i;

	// Shift up the content of the VGA memory
	for(i = 0; i < (vga_region.height - 1); i++) {
		memcpy(vga_mapping + vga_region.x + (vga_cols * (vga_region.y + i)),
			vga_mapping + vga_region.x + (vga_cols * (vga_region.y + i + 1)),
			vga_region.width * 2);
	}

	// Fill the last row with blanks
	for(i = 0; i < vga_region.width; i++) {
		vga_mapping[((vga_region.y + vga_region.height - 1) * vga_cols) + vga_region.x + i] &= 0xFF00;
		vga_mapping[((vga_region.y + vga_region.height - 1) * vga_cols) + vga_region.x + i] |= ' ';
	}
}

// VGA main console output operations
static console_out_ops_t vga_console_out_ops = {
	.reset 		= vga_console_reset,
	.putc 		= vga_console_putc,
	.setRegion 	= vgaSetRegion,
	.clear 		= vgaClear,
	.showCursor = vgaShowCursor,
	.highlight 	= vgaHighlight,
	.moveCursor = vgaMoveCursor,
	.scrollUp 	= vgaScrollUp,
	.scrollDown = vgaScrollDown,
};

/**
 * Initialize the VGA consol
 */
void vga_init(uint16_t cols, uint16_t lines) {
	vga_cols = cols;
	vga_lines = lines;

	// Clear screen
	vga_console_reset();

	// Set output function to console
	main_console.out = &vga_console_out_ops;

	// Save screen resolution
	main_console.width = vga_cols;
	main_console.height = vga_lines;
}
