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

// VGA main console output operations
static console_out_ops_t vga_console_out_ops = {
	.reset = vga_console_reset,
	.putc = vga_console_putc,
};

/**
 * Initialize the VGA consol
 */
void vga_init(uint16_t cols, uint16_t lines) {
	vga_cols = cols;
	vga_lines = lines;

	vga_console_reset();

	main_console.out = &vga_console_out_ops;
}
