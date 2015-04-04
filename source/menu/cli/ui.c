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

#include <lib/ctype.h>
#include <lib/string.h>
#include <lib/utility.h>

#include <assert.h>
#include <memory.h>
#include <time.h>
#include <config.h>

#include "ui.h"

struct ui_choice;
struct ui_textbox;

/** Details of a line in a text view. */
typedef struct ui_textview_line {
	const char *ptr;		/**< Pointer to the line. */
	size_t len;			/**< Length of the line. */
} ui_textview_line_t;

/** Structure containing a text view window. */
typedef struct ui_textview {
	ui_window_t header;		/**< Window header. */

	const char *buf;		/**< Buffer containing text. */
	size_t size;			/**< Size of the buffer. */

	/** Array containing details of each line. */
	struct {
		size_t start;		/**< Start of the line. */
		size_t len;		/**< Length of the line. */
	} *lines;

	size_t count;			/**< Number of lines. */
	size_t offset;			/**< Current offset. */
} ui_textview_t;

/** Structure containing a list window. */
typedef struct ui_list {
	ui_window_t header;		/**< Window header. */
	bool exitable;			/**< Whether the menu can be exited. */
	ui_entry_t **entries;		/**< Array of entries. */
	size_t count;			/**< Number of entries. */
	size_t offset;			/**< Offset of first entry displayed. */
	size_t selected;		/**< Index of selected entry. */
} ui_list_t;

/** Structure containing a link. */
typedef struct ui_link {
	ui_entry_t header;		/**< Entry header. */
	ui_window_t *window;		/**< Window that this links to. */
} ui_link_t;

/** Structure containing a checkbox. */
typedef struct ui_checkbox {
	ui_entry_t header;		/**< Entry header. */
	const char *label;		/**< Label for the checkbox. */
	value_t *value;			/**< Value of the checkbox. */
} ui_checkbox_t;

/** Structure containing a textbox. */
typedef struct ui_textbox {
	ui_entry_t header;		/**< Entry header. */
	const char *label;		/**< Label for the textbox. */
	value_t *value;			/**< Value of the textbox. */
} ui_textbox_t;

/** Structure containing a chooser. */
typedef struct ui_chooser {
	ui_entry_t header;		/**< Entry header. */
	const char *label;		/**< Label for the choice. */
	struct ui_choice *selected;	/**< Selected item. */
	value_t *value;			/**< Value to update. */
	ui_window_t *list;		/**< List implementing the chooser. */
} ui_chooser_t;

/** Structure containing an choice. */
typedef struct ui_choice {
	ui_entry_t header;		/**< Entry header. */
	ui_chooser_t *chooser;		/**< Chooser that the entry is for. */
	const char *label;		/**< Label for the choice. */
	value_t value;			/**< Value of the choice. */
} ui_choice_t;

/** State for the textbox editor. */
static ui_window_t *textbox_edit_window = NULL;
static char textbox_edit_buf[1024];
static size_t textbox_edit_len = 0;
static size_t textbox_edit_offset = 0;
static bool textbox_edit_update = false;

// Size of the content region
#define UI_CONTENT_WIDTH	((size_t)main_console.width - 2)
#define UI_CONTENT_HEIGHT	((size_t)main_console.height - 4)

// ---------------------------------------------------------------------------- [HELPER FUNCS]

// ============================================================================
// Print an action (for help text)
//
// @param action			Action to print
static void
uiActionPrint(ui_action_t *action) {
	if(!action->name) {
		return;
	}

	switch(action->key) {
		case CONSOLE_KEY_UP:
			printf("Up");
			break;
		case CONSOLE_KEY_DOWN:
			printf("Down");
			break;
		case CONSOLE_KEY_LEFT:
			printf("Left");
			break;
		case CONSOLE_KEY_RIGHT:
			printf("Right");
			break;
		case CONSOLE_KEY_F1:
			printf("F1");
			break;
		case CONSOLE_KEY_F2:
			printf("F2");
			break;
		case '\n':
			printf("Enter");
			break;
		case '\e':
			printf("Esc");
			break;
		default:
			printf("%c", action->key & 0xFF);
			break;
	}

	printf(" = %s  ", action->name);
}

// ============================================================================
// Set the region to the title region
//
// @param clear 		Whether to clear the region
static inline void
setTitleRegion(bool clear) {
	draw_region_t region = {0, 0, main_console.width, 1, false};
	main_console.out->setRegion(&region);

	// Is to clear the region?
	if (clear) {
		main_console.out->clear(0, 0, region.width, region.height);
	}
}

// ============================================================================
// Set the region to the helper region
//
// @param clear 		Whether to clear the region
static inline void
setHelperRegion(bool clear) {
	draw_region_t region = {0, main_console.height - 1, main_console.width, 1, false};
	main_console.out->setRegion(&region);

	// Is to clear the region?
	if (clear) {
		main_console.out->clear(0, 0, region.width, region.height);
	}
}

// ============================================================================
// Set the region to the content region
//
// @param clear 		Whether to clear the region
static inline void
setContentRegion(bool clear) {
	draw_region_t region = {1, 2, main_console.width - 2, UI_CONTENT_HEIGHT, false};
	main_console.out->setRegion(&region);

	// Is to clear the region?
	if (clear) {
		main_console.out->clear(0, 0, region.width, region.height);
	}
}

// ============================================================================
// Render helper text for a window
//
// @param window 		Window to render help text for
// @param wimeout 		Seconds remaining
static void
uiWindowRenderHelper(ui_window_t *window, int seconds) {
	// Set helper region
	setHelperRegion(true);

	// Set window type
	window->type->help(window);

	// Is to print seconds?
	if (seconds > 0) {
		main_console.out->moveCursor(0 - ((seconds > 10) ? 12 : 11), 0);
		printf("%d second(s)", seconds);
	}

	// Highlight the region
	main_console.out->highlight(0, 0, main_console.width, 1);
}

// ============================================================================
// Render the contents of a window
//
// @param window 		Window to render
// @param timeout 		Secound remaining
static void
uiWindowRender(ui_window_t *window, int seconds) {
	// Reset console and hide cursor
	main_console.out->reset();
	main_console.out->showCursor(false);

	// Set title region
	setTitleRegion(true);

	// Print window title
	printf("%s", window->title);

	// Put title highlight
	main_console.out->highlight(0, 0, main_console.width, 1);

	// Render helper region
	uiWindowRenderHelper(window, seconds);


	// Set content region
	setContentRegion(true);

	// Render content
	window->type->render(window);

	//
	if (window->type->place_cursor) {
		setContentRegion(false);
		window->type->place_cursor(window);
		main_console.out->showCursor(true);
	}
}

// ============================================================================
// Update the window after completion of an action.
//
// @param window		Window to update.
// @param timeout		Seconds remaining.
static void
uiWindowUpdate(ui_window_t *window, int seconds) {
	// Hide cursor
	main_console.out->showCursor(false);

	// Render helper area
	uiWindowRenderHelper(window, seconds);

	// Check if window type have a "place_cursor" function
	if(window->type->place_cursor) {
		// Set content region
		setContentRegion(false);

		// Execute "place_cursor" function
		window->type->place_cursor(window);

		// Show cursor
		main_console.out->showCursor(true);
	}
}

// ---------------------------------------------------------------------------- [UI LIST FUNCS]

// ============================================================================
// Print a line from a text view.
//
// @param view		View to render.
// @param line		Index of line to print
static void ui_textview_render_line(ui_textview_t *view, size_t line) {
	size_t i;

	for(i = 0; i < view->lines[line].len; i++) {
		main_console.out->putc(view->buf[(view->lines[line].start + i) % view->size]);
	}

	if(view->lines[line].len < UI_CONTENT_WIDTH) {
		main_console.out->putc('\n');
	}
}

// ============================================================================
// Render a textview window.
//
// @param window	Window to render
static void ui_textview_render(ui_window_t *window) {
	ui_textview_t *view = (ui_textview_t *)window;
	size_t i;

	for(i = view->offset; i < min(view->offset + UI_CONTENT_HEIGHT, view->count); i++)
		ui_textview_render_line(view, i);
}

// ============================================================================
// Write the help text for a textview window.
//
// @param window	Window to write for
static void
ui_textview_help(ui_window_t *window) {
	ui_textview_t *view = (ui_textview_t *)window;

	if(view->offset)
		printf("Up = Scroll Up  ");

	if((view->count - view->offset) > UI_CONTENT_HEIGHT)
		printf("Down = Scroll Down  ");

	printf("Esc = Back");
}

// ============================================================================
// Handle input on the window.
//
// @param window	Window input was performed on.
// @param key		Key that was pressed.
 // @return		Input handling result.
static input_result_t
ui_textview_input(ui_window_t *window, uint16_t key) {
	ui_textview_t *view = (ui_textview_t *)window;
	input_result_t ret = INPUT_HANDLED;

	switch(key) {
	case CONSOLE_KEY_UP:
		if(view->offset) {
			main_console.out->scrollUp();
			ui_textview_render_line(view, --view->offset);
		}
		break;
	case CONSOLE_KEY_DOWN:
		if((view->count - view->offset) > UI_CONTENT_HEIGHT) {
			main_console.out->scrollDown();
			main_console.out->moveCursor(0, -1);
			ui_textview_render_line(view, view->offset++ + UI_CONTENT_HEIGHT);
		}
		break;
	case '\e':
		ret = INPUT_CLOSE;
		break;
	}

	return ret;
}

// Text view window type
static ui_window_type_t ui_textview_window_type = {
	.render = ui_textview_render,
	.help = ui_textview_help,
	.input = ui_textview_input,
};

// ============================================================================
// Add a line to a text view.
//
// @param view		View to add to.
// @param start		Start offset of the line.
// @param len		Length of line
static void
ui_textview_add_line(ui_textview_t *view, size_t start, size_t len) {
	// If the line is larger than the content width, split it
	if(len > UI_CONTENT_WIDTH) {
		ui_textview_add_line(view, start, UI_CONTENT_WIDTH);
		ui_textview_add_line(view, (start + UI_CONTENT_WIDTH) % view->size,
			len - UI_CONTENT_WIDTH);
	} else {
		view->lines = realloc(view->lines, sizeof(*view->lines) * (view->count + 1));
		view->lines[view->count].start = start;
		view->lines[view->count++].len = len;
	}
}

// ============================================================================
// Create a text view window.
//
// @param title		Title for the window.
// @param buf		Circular buffer containing text to display.
// @param size		Total size of the buffer.
// @param start		Start character of the buffer.
// @param length	Length of the data from the start (will wrap around).
// @return		Pointer to created window
ui_window_t
*ui_textview_create(const char *title, const char *buf, size_t size, size_t start,
	size_t length) {
	ui_textview_t *view = malloc(sizeof(ui_textview_t));
	size_t i, line_start, line_len;

	ui_window_init(&view->header, &ui_textview_window_type, title);
	view->buf = buf;
	view->size = size;
	view->lines = NULL;
	view->offset = 0;
	view->count = 0;

	/* Store details of all the lines in the buffer. */
	line_start = start;
	line_len = 0;
	for(i = 0; i < length; i++) {
		if(view->buf[(start + i) % view->size] == '\n') {
			ui_textview_add_line(view, line_start, line_len);
			line_start = (start + i + 1) % view->size;
			line_len = 0;
		} else {
			line_len++;
		}
	}

	/* If there is still data at the end (no newline before end), add it. */
	if(line_len)
		ui_textview_add_line(view, line_start, line_len);

	return &view->header;
}

// ============================================================================
// Initialise a list entry structure.
//
// @param entry		Entry structure to initialise.
// @param type		Type of the entry.
void
ui_entry_init(ui_entry_t *entry, ui_entry_type_t *type) {
	entry->type = type;
}

// ============================================================================
// Render an entry from a list.
//
// @note				Current draw region should be the content region.
// @param entry			Entry to render.
// @param pos			Position to render at.
// @param selected		Whether to highlight.
static void
uiListRenderEntry(ui_entry_t *entry, size_t pos, bool selected) {
	draw_region_t region, content;

	// Work out where to put the entry
	main_console.out->getRegion(&content);
	region.x = content.x;
	region.y = content.y + pos;
	region.width = content.width;
	region.height = 1;
	region.scrollable = false;
	main_console.out->setRegion(&region);

	// Render the entry
	entry->type->render(entry);

	// Highlight if necessary
	if(selected) {
		main_console.out->highlight(0, 0, region.width, 1);
	}

	// Restore content region
	main_console.out->setRegion(&content);
}

// ============================================================================
// Render a list window.
//
// @param window	Window to render. */
static void
uiListRender(ui_window_t *window) {
	ui_list_t *list = (ui_list_t *)window;
	size_t i;

	// Render each entry
	for(i = list->offset; i < min(list->offset + UI_CONTENT_HEIGHT, list->count); i++) {
		uiListRenderEntry(list->entries[i], i - list->offset, list->selected == i);
	}
}

// ============================================================================
// Write the helper text for a list window
//
// @param window		Window to write for
static void
uiListHelp(ui_window_t *window) {
	ui_list_t *list = (ui_list_t *)window;
	size_t i;

	if (list->count) {
		// Print help for each of the selected entry's actions
		for(i = 0; i < list->entries[list->selected]->type->action_count; i++) {
			uiActionPrint(&list->entries[list->selected]->type->actions[i]);
		}

		if(list->selected > 0) {
			printf("Up = Scroll Up  ");
		}

		if(list->selected < (list->count - 1)) {
			printf("Down = Scroll Down  ");
		}
	}

	if(list->exitable) {
		printf("Esc = Back");
	}
}

// ============================================================================
// Handle input on the window.
//
// @param window		Window input was performed on.
// @param key			Key that was pressed.
// @return				Input handling result.
static input_result_t
uiListInput(ui_window_t *window, uint16_t key) {
	ui_list_t *list = (ui_list_t *)window;
	input_result_t ret = INPUT_HANDLED;
	size_t i;

	switch(key) {
		case CONSOLE_KEY_UP:
			if(!list->selected) {
				break;
			}

			// Un-highlight current entry
			main_console.out->highlight(0, list->selected - list->offset, UI_CONTENT_WIDTH, 1);

			// If selected becomes less than the offset, must scroll up
			if(--list->selected < list->offset) {
				list->offset--;

				// Scroll region up
				main_console.out->scrollUp();

				// Render list entry
				uiListRenderEntry(list->entries[list->selected], 0, true);
			} else {
				// Highlight new entry
				main_console.out->highlight(0, list->selected - list->offset, UI_CONTENT_WIDTH, 1);
			}
			break;
		case CONSOLE_KEY_DOWN:
			if(list->selected >= (list->count - 1)) {
				break;
			}

			// Un-highlight current entry
			main_console.out->highlight(0, list->selected - list->offset, UI_CONTENT_WIDTH, 1);

			// If selected is now off screen, must scroll down
			if(++list->selected >= list->offset + UI_CONTENT_HEIGHT) {
				list->offset++;
				main_console.out->scrollDown();
				uiListRenderEntry(list->entries[list->selected], UI_CONTENT_HEIGHT - 1, true);
			} else {
				// Highlight new entry
				main_console.out->highlight(0, list->selected - list->offset, UI_CONTENT_WIDTH, 1);
			}
			break;
		case '\e':
			if(list->exitable) {
				ret = INPUT_CLOSE;
			}
			break;
		default:
			// Handle custom actions
			for(i = 0; i < list->entries[list->selected]->type->action_count; i++) {
				if(key != list->entries[list->selected]->type->actions[i].key)
					continue;

				ret = list->entries[list->selected]->type->actions[i].callback(
					list->entries[list->selected]);
				if(ret == INPUT_HANDLED) {
					// Need to re-render the entry
					main_console.out->highlight(0, list->selected - list->offset,
						UI_CONTENT_WIDTH, 1);
					uiListRenderEntry(list->entries[list->selected],
						list->selected - list->offset, true);
				}
				break;
			}
			break;
	}
	return ret;
}

// List window type
static ui_window_type_t ui_list_window_type = {
	.render = uiListRender,
	.help 	= uiListHelp,
	.input 	= uiListInput,
};

// ---------------------------------------------------------------------------- [UI FUNCTION]

// ============================================================================
// Insert an entry into a list window.
//
// @param window	Window to insert into.
// @param entry		Entry to insert.
// @param selected	Whether the entry should be selected.
void
ui_list_insert(ui_window_t *window, ui_entry_t *entry, bool selected) {
	ui_list_t *list = (ui_list_t *)window;
	size_t i = list->count++;

	list->entries = realloc(list->entries, sizeof(ui_entry_t *) * list->count);
	list->entries[i] = entry;
	if(selected) {
		list->selected = i;
		if(i >= UI_CONTENT_HEIGHT)
			list->offset = (i - UI_CONTENT_HEIGHT) + 1;
	}
}

// ============================================================================
// Initialise a window structure
//
// @param window 		Window to initialize
// @param type			Type of window
// @param title			Title of the window
void
ui_window_init(ui_window_t *window, ui_window_type_t *type, const char *title) {
	window->type 	= type;
	window->title 	= title;
}

// ============================================================================
void
uiWindowDisplay(ui_window_t *window, int timeout) {
	timeout_t us = timeout * 1000000;
	input_result_t result;
	uint16_t key;

	// Render window
	uiWindowRender(window, timeout);

	// Main window loop
	while (true) {
		if (timeout > 0) {

		} else {
			// Wait for a key press
			key = main_console.in->getc();

			// Set content region
			setContentRegion(false);

			// Do the apropiated action
			result = window->type->input(window, key);

			// Check if is to close window
			if (result == INPUT_CLOSE) {
				uiWindowRender(window, timeout);
			} else {
				// Need to re-render help text each key press
				// for example if the action moved to a list
				// entry with different actions
				uiWindowUpdate(window, timeout);
			}
		}
	}

	// Reset console
	main_console.out->reset();
}

// ============================================================================
// Create a list window.
//
// @param title			Title for the window
// @param exitable		Whether the window can be exited
// @return				Pointer to created window
ui_window_t
*uiListCreate(const char *title, bool exitable) {
	// Allocate memory for list window
	ui_list_t *list = malloc(sizeof(ui_list_t));

	// Init the window structure
	ui_window_init(&list->header, &ui_list_window_type, title);
	list->exitable = exitable;
	list->entries = NULL;
	list->count = 0;
	list->offset = 0;
	list->selected = 0;

	return &list->header;
}
