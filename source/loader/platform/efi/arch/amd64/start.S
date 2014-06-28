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
 * @brief      AMD64 EFI startup code
 */

#include <arch/page.h>

#include <x86/asm.h>

.section .text, "ax", @progbits

// EFI entry point
FUNCTION_START(_start)
	mov	$0x3f8, %dx
	mov	$'H', %al
	out	%al, (%dx)
	mov	$'e', %al
	out	%al, (%dx)
	mov	$'l', %al
	out	%al, (%dx)
	mov	$'l', %al
	out	%al, (%dx)
	mov	$'o', %al
	out	%al, (%dx)
	mov	$'\r', %al
	out	%al, (%dx)
	mov	$'\n', %al
	out	%al, (%dx)
1:	jmp	1b
FUNCTION_END(_start)

// Dummy PE relocation so that the EFI loader recognizes us as relocatable.

.section .data, "aw", @progbits

__dummy:
	.long   0

.section .reloc, "aw", @progbits

__dummy_reloc:
        .long   __dummy - __dummy_reloc
        .long   10
        .word   0
