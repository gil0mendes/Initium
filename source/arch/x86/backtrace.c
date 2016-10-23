/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Gil Mendes <gil00mendes@gmail.com>
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
 * @brief               x86 backtrace function.
 */

#include <lib/backtrace.h>
#include <x86/cpu.h>

#include <loader.h>

/** Structure containing a stack frame. */
typedef struct stack_frame {
	struct stack_frame *next;       /**< Pointer to next stack frame. */
	ptr_t addr;                     /**< Function return address. */
} stack_frame_t;

/**
 * Print out a backtrace.
 *
 * @param func  Print function to use.
 */
void backtrace(printf_t func)
{
	stack_frame_t *frame;

  #ifdef __PIC__
	func("Backtrace (base = %p):\n", __start);
  #else
	func("Backtrace:\n");
  #endif

	frame = (stack_frame_t*)x86_read_bp();
	while (frame && frame->addr) {
    #ifdef __PIC__
		func(" %p (%p)\n", frame->addr, frame->addr - (ptr_t)__start);
	  #else
		func(" %p\n", frame->addr);
	  #endif

		frame = frame->next;
	}
}
