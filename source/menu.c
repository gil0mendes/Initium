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
 * @brief                       Menu interface.
 */

#include <lib/list.h>
#include <lib/string.h>

#include <config.h>
#include <loader.h>
#include <memory.h>
#include <menu.h>
#include <shell.h>
#include <time.h>
#include <ui.h>

/** Structure containing a menu entry. */
typedef struct menu_entry {
    ui_entry_t entry;                   /**< UI entry header. */

    list_t header;                      /**< Link to menu entries list. */
    char *name;                         /**< Name of the entry. */
    environ_t *env;                     /**< Environment for the entry. */
} menu_entry_t;

/** List of menu entries. */
static LIST_DECLARE(menu_entries);

/** Selected menu entry. */
static menu_entry_t *selected_menu_entry = NULL;

/** Render a menu entry.
 * @param _entry        Entry to render. */
static void menu_entry_render(ui_entry_t *_entry) {
    menu_entry_t *entry = (menu_entry_t *)_entry;

    ui_printf("%s", entry->name);
}

/** Write the help text for a menu entry.
 * @param _entry        Entry to write for. */
static void menu_entry_help(ui_entry_t *_entry) {
    menu_entry_t *entry = (menu_entry_t *)_entry;

    ui_print_action('\n', "Select");

    if (entry->env->loader->configure)
	ui_print_action(CONSOLE_KEY_F1, "Configure");

    ui_print_action(CONSOLE_KEY_F2, "Shell");
}

/** Handle input on a menu entry.
 * @param _entry        Entry input was performed on.
 * @param key           Key that was pressed.
 * @return              Input handling result. */
static input_result_t menu_entry_input(ui_entry_t *_entry, uint16_t key) {
    menu_entry_t *entry = (menu_entry_t *)_entry;

    switch (key) {
	case '\n':
	    selected_menu_entry = entry;
	    return INPUT_CLOSE;
	case CONSOLE_KEY_F1:
	    if (entry->env->loader->configure) {
            size_t len;
            char *title __cleanup_free;
		    ui_window_t *window;
		    environ_t *prev;

            /* Determine the window title. */
            len = strlen(entry->name) + 13;
            title = malloc(len);
            snprintf(title, len, "Configure '%s'", entry->name);

		    prev = current_environ;
		    current_environ = entry->env;
		    window = entry->env->loader->configure(entry->env->loader_private, title);
		    current_environ = prev;

		    ui_display(window, ui_console, 0);
            ui_window_destroy(window);
		    return INPUT_RENDER_WINDOW;
		} else {
		    return INPUT_HANDLED;
		}
	case CONSOLE_KEY_F2:
	    /* This is taken to mean the shell should be entered. */
	    selected_menu_entry = NULL;
	    return INPUT_CLOSE;
	default:
	    return INPUT_HANDLED;
	}
}

/** Menu entry type. */
static ui_entry_type_t menu_entry_type = {
    .render = menu_entry_render,
    .help = menu_entry_help,
    .input = menu_entry_input,
};

/** Get the default menu entry.
 * @return              Default menu entry. */
static menu_entry_t *get_default_entry(void) {
    const value_t *value = environ_lookup(root_environ, "default");

    if (value) {
	    size_t i = 0;

	    list_foreach(&menu_entries, iter) {
		menu_entry_t *entry = list_entry(iter, menu_entry_t, header);

		if (value->type == VALUE_TYPE_INTEGER) {
			if (i == value->integer)
			    return entry;
		    } else if (value->type == VALUE_TYPE_STRING) {
			if (strcmp(entry->name, value->string) == 0)
			    return entry;
		    }

		i++;
	    }
	}

    /* No default entry found, return the first list entry. */
    return list_first(&menu_entries, menu_entry_t, header);
}

/** Display the menu interface.
 * @return              Environment to boot. */
environ_t *menu_display(void) {
    ui_window_t *window;

    /* Assume if no entries are declared the root environment is bootable. */
    if (list_empty(&menu_entries))
	return root_environ;

    /* Determine the default entry. */
    selected_menu_entry = get_default_entry();

    /* Build the UI. */
    window = ui_list_create("Boot Menu", false);
    list_foreach(&menu_entries, iter) {
	menu_entry_t *entry = list_entry(iter, menu_entry_t, header);
	ui_list_insert(window, &entry->entry, entry == selected_menu_entry);
    }

    // TODO: timeout, hidden, destroy window
    ui_display(window, &main_console, 0);

    if (selected_menu_entry) {
	    dprintf("menu: booting menu entry '%s'\n", selected_menu_entry->name);
	    return selected_menu_entry->env;
	} else {
	    shell_main();
	    target_reboot();
	}
}

/** Add a new menu entry.
 * @param args          Arguments to the command.
 * @return              Whether successful. */
static bool config_cmd_entry(value_list_t *args) {
    menu_entry_t *entry;

    if (args->count != 2
        || args->values[0].type != VALUE_TYPE_STRING
        || args->values[1].type != VALUE_TYPE_COMMAND_LIST)
	{
	    config_error("entry: Invalid arguments");
	    return false;
	}

    if (current_environ != root_environ) {
	    config_error("entry: Nested entries not allowed");
	    return false;
	}

    entry = malloc(sizeof(menu_entry_t));
    entry->entry.type = &menu_entry_type;
    entry->name = args->values[0].string;
    args->values[0].string = NULL;
    entry->env = environ_create(current_environ);

    /* Execute the command list. */
    if (!command_list_exec(args->values[1].cmds, entry->env)) {
	    goto err;
	} else if (!entry->env->loader) {
	    config_error("entry: Entry '%s' does not have a loader", entry->name);
	    goto err;
	}

    list_init(&entry->header);
    list_append(&menu_entries, &entry->header);
    return true;

err:
    environ_destroy(entry->env);
    free(entry->name);
    free(entry);
    return false;
}

BUILTIN_COMMAND("entry", config_cmd_entry);
