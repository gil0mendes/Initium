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
* @brief 			User interface functions
*/

#ifndef __UI_H
#define __UI_H

#include <console.h>

struct ui_entry;
struct ui_window;

// Return codes for input handling functions
typedef enum input_result {
	INPUT_HANDLED,		// No special action needed
	INPUT_RENDER,		// Re-render the window
	INPUT_CLOSE			// Close the window
} input_result_t;

// Structu defining a window action
typedef struct ui_action {
	const char *name;		// Name of action
	uint16_t key;			// Key to perform action

	// Callback for the action
	//
	// @param entry 		Entry the action was performed on
	// @return				Input handling result
	input_result_t (*callback)(struct ui_entry *entry);
} ui_action_t;

// Structure defining a window type
typedef struct ui_window_type {
	// Render the window
	//
	// @note 			The draw region will be set to the content area
	// @param window 	Window to rend
	void (*render)(struct ui_window *window);

	// Write the help text for the window
	//
	// @note 			The cursor will be positioned where to write
	// @note 			This is called after each action is handled
	// @param window 	Window to write for
	void (*help)(struct ui_window *window);

	// Handle input on the window
	//
	// @note 			The draw region will be set to the content are
	// @param window	Window input was performed on
	// @param key 		Key that was pressed
	// @return 			Input handling result
	input_result_t (*input)(struct ui_window *window, uint16_t key);

	// Move the cursor to where it needs to be in the content area
	//
	// @note 			The draw region will be set to the content are
	// @param window 	Window to draw cursor for
	void (*place_cursor)(struct ui_window *window);
} ui_window_type_t;

// Window header structure
typedef struct ui_window {
	ui_window_type_t *type; 	// Type of the window
	const char *title;				// Title of the window
} ui_window_t;

// Sctucture defining a UI list entry type
typedef struct ui_entry_type {
	ui_action_t *actions;		// Actions that can be performed on the entry
	size_t action_count;		// Number of actions in the array

	// Render the entry
	//
	// @note 			The draw region will set to where to render
	// @param entry		Entry to render
	void (*render)(struct ui_entry *entry);
} ui_entry_type_t;

// List entry header structure
typedef struct ui_entry {
	ui_entry_type_t *type;		// Type of the entry
} ui_entry_t;

extern void ui_list_insert(ui_window_t *window, ui_entry_t *entry, bool selected);

extern void ui_entry_init(ui_entry_t *entry, ui_entry_type_t *type);

extern ui_window_t *ui_textview_create(const char *title, const char *buf,
	size_t size, size_t start, size_t length);

extern void ui_window_init(ui_window_t *window, ui_window_type_t *type, const char *title);
extern void uiWindowDisplay(ui_window_t *window, int timeout);

extern ui_window_t *uiListCreate(const char *title, bool exitable);

#endif // __UI_H
