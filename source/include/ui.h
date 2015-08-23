/**
* The MIT License (MIT)
*
* Copyright (c) 2015 Gil Mendes
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
 * @brief               User interface.
 */

#ifndef __UI_H
#define __UI_H

#include <config.h>
#include <console.h>

#ifdef CONFIG_TARGET_HAS_UI

struct ui_entry;
struct ui_window;

/** Return codes for input handling functions. */
typedef enum input_result {
    INPUT_HANDLED,                      /**< No special action needed. */
    INPUT_RENDER_ENTRY,                 /**< Re-render the list entry. */
    INPUT_RENDER_HELP,                  /**< Re-render help (in case possible actions change). */
    INPUT_RENDER_WINDOW,                /**< Re-render the whole window. */
    INPUT_CLOSE,                        /**< Close the window. */
} input_result_t;

/** Structure defining a window type. */
typedef struct ui_window_type {
    /** Destroy the window (optional).
     * @param window        Window to destroy. */
    void (*destroy)(struct ui_window *window);

    /** Render the window.
     * @note                The draw region will be set to the content area,
     *                      cursor will be positioned at (0, 0). If the cursor
     *                      should be visible, this function should position and
     *                      enable it.
     * @param window        Window to render. */
    void (*render)(struct ui_window *window);

    /** Write the help text for the window.
     * @note                The draw region will be set to the help area, cursor
     *                      will be positioned where to write.
     * @param window        Window to write for. */
    void (*help)(struct ui_window *window);

    /** Handle input on the window.
     * @note                Draw region and cursor state are maintained from the
     *                      state initially set by render() and across all calls
     *                      to this until one returns INPUT_RENDER_WINDOW.
     * @param window        Window input was performed on.
     * @param key           Key that was pressed.
     * @return              Input handling result. */
    input_result_t (*input)(struct ui_window *window, uint16_t key);
} ui_window_type_t;

/** Window header structure. */
typedef struct ui_window {
    ui_window_type_t *type;             /**< Type of the window. */
    const char *title;                  /**< Title of the window. */
} ui_window_t;

/** Structure defining a UI list entry type. */
typedef struct ui_entry_type {
    /** Destroy the entry (optional).
     * @param entry         Entry to destroy. */
    void (*destroy)(struct ui_entry *entry);

    /** Render the entry.
     * @note                The draw region will set to where to render, cursor
     *                      will be positioned at (0, 0).
     * @param entry         Entry to render. */
    void (*render)(struct ui_entry *entry);

    /** Write the help text for the entry.
     * @note                The draw region will be set to the help area, cursor
     *                      will be positioned where to write.
     * @param entry         Entry to write for. */
    void (*help)(struct ui_entry *entry);

    /** Handle input on the entry.
     * @param entry         Entry input was performed on.
     * @param key           Key that was pressed.
     * @return              Input handling result. */
    input_result_t (*input)(struct ui_entry *entry, uint16_t key);
} ui_entry_type_t;

/** List entry header structure. */
typedef struct ui_entry {
    ui_entry_type_t *type;              /**< Type of the entry. */
} ui_entry_t;

extern console_t *ui_console;

/** Versions of printf() to use when rendering the UI. */
#define ui_vprintf(fmt, args) console_vprintf(ui_console, fmt, args)
#define ui_printf(fmt...) console_printf(ui_console, fmt)

extern void ui_print_action(uint16_t key, const char *name);

extern void ui_display(ui_window_t *window, console_t *console, unsigned timeout);

extern ui_window_t *ui_list_create(const char *title, bool exitable);
extern void ui_list_insert(ui_window_t *window, ui_entry_t *entry, bool selected);
extern bool ui_list_empty(ui_window_t *window);

extern ui_entry_t *ui_link_create(ui_window_t *window);
extern ui_entry_t *ui_entry_create(const char *label, value_t *value);
extern ui_entry_t *ui_checkbox_create(const char *label, value_t *value);
extern ui_entry_t *ui_textbox_create(const char *label, value_t *value);
extern ui_entry_t *ui_chooser_create(const char *label, value_t *value);
extern void ui_chooser_insert(ui_entry_t *entry, const value_t *value, char *label);

extern void ui_window_destroy(ui_window_t *window);
extern void ui_entry_destroy(ui_entry_t *entry);

#endif /* CONFIG_TARGET_HAS_UI */
#endif /* __UI_H */
