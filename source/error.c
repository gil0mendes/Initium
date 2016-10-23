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
 * @brief       Boot error handling functions
 */

#include <lib/backtrace.h>
#include <lib/printf.h>

#include <console.h>
#include <loader.h>
#include <memory.h>
#include <shell.h>
#include <ui.h>

/** Boot error message. */
static const char *boot_error_format;
static va_list boot_error_args;

/**
 * Helper for error_printf().
 */
static void error_printf_helper(char ch, void *data, int *total)
{
	if (debug_console != current_console) {
		console_putc(debug_console, ch);
	}

	// print out a character
	console_putc(current_console, ch);

	*total = *total + 1;
}

/**
 * Formatted print to both consoles.
 */
static int error_printf(const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = do_vprintf(error_printf_helper, NULL, fmt, args);
	va_end(args);

	return ret;
}

/**
 * Helper for error_dprintf().
 */
static void error_dprintf_helper(char ch, void *data, int *total) {
  console_putc(debug_console, ch);

  *total = *total + 1;
}

/**
 * Formatted print to debug console bypassing the log.
 */
static int error_dprintf(const char *fmt, ...) {
  va_list args;
  int ret;

  va_start(args, fmt);
  ret = do_vprintf(error_dprintf_helper, NULL, fmt, args);
  va_end(args);

  return ret;
}

/**
 * Raise an internal error.
 *
 * @param fmt   Error format string.
 * @param ...   Values to substitute into format.
 */
void __noreturn internal_error(const char *fmt, ...)
{
	va_list args;

	// end console UI mode
	if (current_console && current_console->out && current_console->out->in_ui) {
		console_end_ui(current_console);
	}

	error_printf("\nInternal Error: ");

	va_start(args, fmt);
	do_vprintf(error_printf_helper, NULL, fmt, args);
	va_end(args);

	error_printf("\n\n");
	error_printf("Please report this error to https://github.com/gil0mendes/Initium/issues\n");

	backtrace(error_printf);

	target_halt();
}

/**
 * Display the boot error message.
 *
 * @param console Console to print to.
 */
static void boot_error_message(console_t *console)
{
	do_vprintf(error_printf_helper, console, boot_error_format, boot_error_args);

	console_printf(console, "\n\n");
	console_printf(console, "Ensure that you have enough memory available, that you do not have any\n");
	console_printf(console, "malfunctioning hardware and that your computer meets the minimum system\n");
	console_printf(console, "requirements for the operating system.\n\n");
}

#ifdef CONFIG_TARGET_HAS_UI

/**
 * Render the boot error window.
 *
 * @param  window Window to render.
 */
static void boot_error_render(ui_window_t *window)
{
	boot_error_message(current_console);
}

/**
 * Write the help text for the boot error.
 *
 * @param window Window to write for.
 */
static void boot_error_help(ui_window_t *window)
{
	ui_print_action('\e', "Reboot");

	if (shell_enabled) {
		ui_print_action(CONSOLE_KEY_F2, "Shell");
	}

	ui_print_action(CONSOLE_KEY_F10, "Debug Log");
}

/**
 * Handle input on the boot error window.
 *
 * @param  window Window input was performed on.
 * @param  key    Key that was pressed.
 * @return        Input handling result.
 */
static input_result_t boot_error_input(ui_window_t *window, uint16_t key)
{
	switch (key) {
	case '\e':
		target_reboot();
		return INPUT_HANDLED;
	case CONSOLE_KEY_F2:
		/* We start the shell in boot_error() upon return. */
		return (shell_enabled) ? INPUT_CLOSE : INPUT_HANDLED;
	case CONSOLE_KEY_F10:
		debug_log_display();
		return INPUT_RENDER_WINDOW;
	default:
		return INPUT_HANDLED;
	}
}

/** Boot error window type. */
static ui_window_type_t boot_error_window_type = {
	.render = boot_error_render,
	.help	= boot_error_help,
	.input	= boot_error_input,
};

#endif // CONFIG_TARGET_HAS_UI.

/**
 * Display details of a boot error.
 *
 * @param fmt   Error format string.
 * @param ...   Values to substitute into format.
 */
void __noreturn boot_error(const char *fmt, ...)
{
	// Save the format string and arguments for boot_error_message()
	boot_error_format = fmt;
	va_start(boot_error_args, fmt);

  // Print the message out to the debug console, along with a backtrace.
  console_printf(debug_console, "\nBoot Error: ");
  boot_error_message(debug_console);
  backtrace(error_dprintf);

#ifdef CONFIG_TARGET_HAS_UI
	if (console_has_caps(current_console, CONSOLE_CAP_UI)) {
		ui_window_t *window;

		window = malloc(sizeof(*window));
		window->type = &boot_error_window_type;
		window->title = "Boot Error";

		ui_display(window, 0);
		ui_window_destroy(window);

		// jump into the shell (only get here if it is enabled)
		shell_main();
	}
#endif  // CONFIG_TARGET_HAS_UI

	if (current_console != debug_console) {
    console_printf(current_console, "\nBoot Error: ");
    boot_error_message(current_console);
  }

	// jump into the shell
	if (shell_enabled) {
		shell_main();
	} else {
		target_halt();
	}
}
