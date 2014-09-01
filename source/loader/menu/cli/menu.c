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
* @brief 			Menu interface implementation for CLI mode
*/

#include <lib/string.h>
#include <lib/utility.h>

#include <assert.h>
#include <loader.h>
#include <memory.h>
#include <menu.h>
#include <time.h>

#include "ui.h"

// Structure containing a menu entry
typedef struct menu_entry {
	ui_entry_t header;		// UI entry header
	list_t link;					// Link to menu entries list
	char *name;						// Name of the entry
	environ_t *env;				// Environment for the entry
} menu_entry_t;

// List of menu entries
static LIST_DECLARE(menu_entries);

// Selected menu entry
static menu_entry_t *selected_menu_entry = NULL;

// ---------------------------------------------------------------------------- [COMMANDS]

// ============================================================================
//  Add a new menu entry.
//
// @param args		Arguments to the command.
// @return		Whether successful
static bool
config_cmd_entry(value_list_t *args) {
	menu_entry_t *entry;

	if(current_environ != root_environ) {
		dprintf("config: entry: nested entries not allowed\n");
		return false;
	}

	if(args->count != 2 || args->values[0].type != VALUE_TYPE_STRING
		|| args->values[1].type != VALUE_TYPE_COMMAND_LIST)
	{
		dprintf("config: entry: invalid arguments\n");
		return false;
	}

	entry = malloc(sizeof(menu_entry_t));
	list_init(&entry->link);
	entry->name = strdup(args->values[0].string);

	/* Execute the command list. */
	if(!command_list_exec(args->values[1].cmds, &entry->env)) {
		free(entry->name);
		free(entry);
		return false;
	}

	list_append(&menu_entries, &entry->link);
	return true;
}

BUILTIN_COMMAND("entry", config_cmd_entry);

// ---------------------------------------------------------------------------- [STATIC FUNCTIONS]

// ============================================================================
// Find the default menu entry
//
// @return 				Default entry
static menu_entry_t
*menu_find_default(void) {
	menu_entry_t *entry;
	value_t *value;
	uint64_t i = 0;

	if ((value = environ_lookup(root_environ, "default"))) {
		LIST_FOREACH(&menu_entries, iter) {
			entry = list_entry(iter, menu_entry_t, link);

			if (value->type == VALUE_TYPE_INTEGER) {
				if (i == value->integer) {
					return entry;
				}
			} else if (value->type == VALUE_TYPE_STRING) {
				if (strcmp(entry->name, value->string) == 0) {
					return entry;
				}
			}

			i++;
		}
	}

	// No default entry found, return the first list entry
	return list_entry(menu_entries.next, menu_entry_t, link);
}

// ============================================================================
// Check if the menu can be displayed
//
// @return 			Whether the menu can be displayed
static bool
menu_can_display(void) {
	value_t *value;

	if (!main_console.in) {
		return false;
	} else if ((value = environ_lookup(root_environ, "hidden")) && value->boolean) {
		// Menu hidden, wait half a second for Esc to be pressed
		spin(500000);

		while(main_console.in->poll()) {
			if (main_console.in->getc() == '\e') {
				return true;
			}
		}
	}

	return true;
}

// ============================================================================
// Select a menu entry
//
// @param _entry				Entry that was selected
// @return 							Always returns INPUT_CLOSE
static input_result_t
menu_entry_select(ui_entry_t *_entry) {
	menu_entry_t *entry = (menu_entry_t *)_entry;

	selected_menu_entry = entry;
	return INPUT_CLOSE;
}

// ============================================================================
// Configure a menu entry
//
// @param _entry 				Entry that was selected
// @return 							Always returns INPUT_RENDER
static input_result_t
menu_entry_configure(ui_entry_t *_entry) {
	menu_entry_t *entry = (menu_entry_t *)_entry;
	ui_window_t *window;
	environ_t *prev;

	prev = current_environ;
	current_environ = entry->env;
	window = entry->env->loader->configure();
	current_environ = prev;

	uiWindowDisplay(window, 0);
	return INPUT_RENDER;
}

// ============================================================================
// Show a debug log window
//
// @param _entry 				Entry that was selected
// @return 							Always returns INPUT_RENDER
static input_result_t menu_entry_debug(ui_entry_t *_entry) {
	ui_window_t *window;

	// Create the debug window
	window = ui_textview_create("Debug Log", debug_log, DEBUG_LOG_SIZE,
		debug_log_start, debug_log_length);
	uiWindowDisplay(window, 0);
	return INPUT_RENDER;
}

// Actions for a menu entry
static ui_action_t menu_entry_actions[] = {
	{ "Boot", '\n', menu_entry_select },
	{ NULL, CONSOLE_KEY_F10, menu_entry_debug },
};

// Actions for a configurable menu entry
static ui_action_t configurable_menu_entry_actions[] = {
	{ "Boot", '\n', menu_entry_select },
	{ "Configure", CONSOLE_KEY_F1, menu_entry_configure },
	{ NULL, CONSOLE_KEY_F10, menu_entry_debug },
};

// ============================================================================
// Render a menu entry.
//
// @param _entry	Entry to render
static void
menu_entry_render(ui_entry_t *_entry) {
	menu_entry_t *entry = (menu_entry_t *)_entry;
	printf("%s", entry->name);
}

// Menu entry UI entry type.
static ui_entry_type_t menu_entry_type = {
	.actions = menu_entry_actions,
	.action_count = ARRAY_SIZE(menu_entry_actions),
	.render = menu_entry_render,
};

// Configurable menu entry UI entry type
static ui_entry_type_t configurable_menu_entry_type = {
	.actions = configurable_menu_entry_actions,
	.action_count = ARRAY_SIZE(configurable_menu_entry_actions),
	.render = menu_entry_render,
};

// ============================================================================
// Return whether a menu entry is configurable.
//
// @param entry		Entry to check
// @return				Whether configurable
static bool menu_entry_configurable(menu_entry_t *entry) {
	bool ret = false;
	environ_t *prev;

	if(entry->env->loader && entry->env->loader->configure) {
		prev = current_environ;
		current_environ = entry->env;
		ret = entry->env->loader->configure() != NULL;
		current_environ = prev;
	}

	return ret;
}

// ---------------------------------------------------------------------------- [EXTERN FUNCTIONS]

// ============================================================================
// Display the menu interface
//
// @return		Environment for the entry to boot
environ_t
*menuDisplay(void) {
	menu_entry_t *entry;
	ui_window_t *window;
	int timeout = 0;
	value_t *value;

	// If no menu entires are defined, assume that the top-level environment
	// has been configured with something to boot
	if (list_empty(&menu_entries)) {
		return root_environ;
	}

	// Find the default entry
	selected_menu_entry = menu_find_default();

	if (menu_can_display()) {
		// Construct the menu
		window = uiListCreate("Boot Menu", false);

		LIST_FOREACH(&menu_entries, iter) {
			entry = list_entry(iter, menu_entry_t, link);

			// If the entry's loader returns a configuration window,
			// use the configurable entry type
			if (menu_entry_configurable(entry)) {
				ui_entry_init(&entry->header, &configurable_menu_entry_type);
			} else {
				ui_entry_init(&entry->header, &menu_entry_type);
			}

			ui_list_insert(window, &entry->header, entry == selected_menu_entry);
		}

	/*	if ((value = environ_lookup(root_environ, "timeout")) && value->type == VALUE_TYPE_INTEGER) {
			timeout = value->integer;
		}*/

		// Display it. The selected entry point will be updated
		uiWindowDisplay(window, timeout);
	}

	//dprintf("loader: booting menu entry '%s'\n", selected_menu_entry->name);

	//return selected_menu_entry->env;
	return NULL;
}
