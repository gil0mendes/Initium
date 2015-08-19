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
 * @brief       x86 exception handling functions
 */

#include <x86/cpu.h>
#include <x86/descriptor.h>
#include <x86/exception.h>

#include <loader.h>

// Structure containing a stack frame
typedef struct stack_frame {
	struct stack_frame *next;   // Pointer to next stack frame
	ptr_t addr;                 // Function return address
} stack_frame_t;

/**
 *Print out a backtrace.
 *
 * @param print		Print function to use.
 */
void backtrace(int (*print)(const char *fmt, ...)) {
	stack_frame_t *frame;

	frame = (stack_frame_t *)x86_read_bp();
	while(frame) {
#ifdef __PIC__
		print(" %p (%p)\n", frame->addr, frame->addr - (ptr_t)__start);
#else
    	print(" %p\n", frame->addr);
#endif
		frame = frame->next;
	}
}

#ifdef __LP64__

/** Handle an exception.
 *
 * @param frame		Interrupt frame.
 */
void x86_exception_handler(exception_frame_t *frame) {
	internal_error("Exception %lu (error code 0x%lx)\n"
		"cs: 0x%04lx  ss: 0x%04lx\n"
		"rip: 0x%016lx  rsp: 0x%016lx  rflags: 0x%08lx\n"
		"rax: 0x%016lx  rbx: 0x%016lx  rcx: 0x%016lx\n"
		"rdx: 0x%016lx  rdi: 0x%016lx  rsi: 0x%016lx\n"
		"rbp: 0x%016lx  r8:  0x%016lx  r9:  0x%016lx\n"
		"r10: 0x%016lx  r11: 0x%016lx  r12: 0x%016lx\n"
		"r13: 0x%016lx  r14: 0x%016lx  r15: 0x%016lx",
		frame->num, frame->err_code, frame->cs, frame->ss, frame->ip,
		frame->sp, frame->flags, frame->ax, frame->bx, frame->cx,
		frame->dx, frame->di, frame->si, frame->bp, frame->r8,
		frame->r9, frame->r10, frame->r11, frame->r12, frame->r13,
		frame->r14, frame->r15);
}

#else /* __LP64__ */

/** Handle an exception.
 *
 * @param frame		Interrupt frame.
 */
void x86_exception_handler(exception_frame_t *frame) {
	internal_error("Exception %lu (error code 0x%lx)\n"
		"cs: 0x%04lx  ds: 0x%04lx  es: 0x%04lx  fs: 0x%04lx  gs: 0x%04lx\n"
		"eip: 0x%08lx  esp: 0x%08lx  eflags: 0x%08lx\n"
		"eax: 0x%08lx  ebx: 0x%08lx  ecx: 0x%08lx  edx: 0x%08lx\n"
		"edi: 0x%08lx  esi: 0x%08lx  ebp: 0x%08lx",
		frame->num, frame->err_code, frame->cs, frame->ds, frame->es,
		frame->fs, frame->gs, frame->ip, frame->sp, frame->flags,
		frame->ax, frame->bx, frame->cx, frame->dx, frame->di,
		frame->si, frame->bp);
}

#endif /* __LP64__ */
