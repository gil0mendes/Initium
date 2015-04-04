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
* @brief			PXE loader wrapper
*/

#include <platform/loader.h>

#include <x86/asm.h>

.section .text
.code16

// ============================================================================
// Main function of the PXE boot sector
FUNCTION_START(_start)
	// Set the boot drive ID to 0x7F for PXE
	movb	$0x7F, %dl
	xorl	%ebx, %ebx
	xorl	%ecx, %ecx

	// Jump to the loader entry point
	ljmp	$(LOADER_LOAD_ADDR >> 4), $0
FUNCTION_END(_start)

// Pad the file to the loader load address
. = _start + LOADER_LOAD_ADDR - 0x7C00 - 1;
.byte 0
