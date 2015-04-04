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
 * @brief       Boot error handling functions
 */

#include <lib/printf.h>

#include <console.h>
#include <loader.h>

/**
 * Helper for printing error messages.
 *
 * @param ch		Character to display.
 * @param data		Ignored.
 * @param total		Pointer to total character count.
 */
static void error_printf_helper(char ch, void *data, int *total) {
	if(debug_console.out) {
		debug_console.out->putc(ch);
	}

	if(main_console.out) {
		main_console.out->putc(ch);
	}

	*total = *total + 1;
}

/**
 * Formatted print function for error functions.
 */
static int error_printf(const char *fmt, ...) {
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = do_vprintf(error_printf_helper, NULL, fmt, args);
	va_end(args);

	return ret;
}

/**
 * Raise an internal error.
 *
 * @param fmt		Error format string.
 * @param ...		Values to substitute into format.
 */
void __noreturn internal_error(const char *fmt, ...) {
	va_list args;

	if(main_console.out) {
		main_console.out->reset();
	}

	error_printf("\nAn internal error has occurred:\n\n");

	va_start(args, fmt);
	do_vprintf(error_printf_helper, NULL, fmt, args);
	va_end(args);

	error_printf("\n\n");
	error_printf("Please report this error to https://github.com/gil0mendes/Initium/issues\n");
#ifdef __PIC__
	error_printf("Backtrace (base = %p):\n", __start);
#else
	error_printf("Backtrace:\n");
#endif
	backtrace(error_printf);

	system_halt();
}

/**
 * Display details of a boot error.
 *
 * @param fmt		Error format string.
 * @param ...		Values to substitute into format.
 */
void __noreturn boot_error(const char *fmt, ...) {
	va_list args;

	/* TODO: This eventually needs to go in a UI window and should let you
	 * reboot the machine. */

	if(main_console.out) {
		main_console.out->reset();
	}

	error_printf("\nAn error has occurred during boot:\n\n");

	va_start(args, fmt);
	do_vprintf(error_printf_helper, NULL, fmt, args);
	va_end(args);

	error_printf("\n\n");
	error_printf("Ensure that you have enough memory available, that you do not have any\n");
	error_printf("malfunctioning hardware and that your computer meets the minimum system\n");
	error_printf("requirements for the operating system.\n");

	system_halt();
}
