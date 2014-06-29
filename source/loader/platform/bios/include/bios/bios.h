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
 * @brief		BIOS platform main definitions.
 */

#ifndef __BIOS_BIOS_H
#define __BIOS_BIOS_H

#include <lib/string.h>

#include <x86/cpu.h>

/** Memory area to use when passing data to BIOS interrupts (56KB).
 * @note		Area is actually 60KB, but the last 4KB are used for
 *			the stack. */
#define BIOS_MEM_BASE		0x1000
#define BIOS_MEM_SIZE		0xe000

/** Convert a segment + offset pair to a linear address. */
#define SEGOFF2LIN(segoff) \
	(ptr_t)((((segoff) & 0xffff0000) >> 12) + ((segoff) & 0xffff))

/** Convert a linear address to a segment + offset pair. */
#define LIN2SEGOFF(lin) \
	(uint32_t)(((lin & 0xfffffff0) << 12) + (lin & 0xf))

/** Structure describing registers to pass to a BIOS interrupt. */
typedef struct bios_regs {
	union {
		struct {
			uint32_t eflags;
			uint32_t eax;
			uint32_t ebx;
			uint32_t ecx;
			uint32_t edx;
			uint32_t edi;
			uint32_t esi;
			uint32_t ebp;
			uint32_t _es;
		};
		struct {
			uint16_t flags, _hflags;
			uint16_t ax, _hax;
			uint16_t bx, _hbx;
			uint16_t cx, _hcx;
			uint16_t dx, _hdx;
			uint16_t di, _hdi;
			uint16_t si, _hsi;
			uint16_t bp, _hbp;
			uint16_t es, _hes;
		};
	};
} bios_regs_t;

/** Initialise a BIOS registers structure.
 * @param regs		Structure to initialise. */
static inline void bios_regs_init(bios_regs_t *regs) {
	memset(regs, 0, sizeof(bios_regs_t));
}

extern void bios_call(uint8_t num, bios_regs_t *regs);
extern void bios_console_init(void);
extern void bios_memory_init(void);
extern void platform_init(void);
extern void platform_reboot(void);

#endif /* __BIOS_BIOS_H */
