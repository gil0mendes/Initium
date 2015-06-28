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
 * @brief		x86 ELF definitions.
 */

#ifndef __ARCH_ELF_H
#define __ARCH_ELF_H

// Definitions of native ELF machine type, endianness, etc
#ifdef CONFIG_64BIT
  #define ELF_MACHINE	ELF_EM_X86_64	  // ELF machine (x86_64)
  #define ELF_CLASS	  ELFCLASS64	    // ELF class (64-bit)
#else
  #define ELF_MACHINE	ELF_EM_386	    // ELF machine (i386)
  #define ELF_CLASS	  ELFCLASS32	    // ELF class (32-bit)
#endif

#define ELF_ENDIAN	  ELFDATA2LSB	    // ELF endianness (little-endian)

#endif /* __ARCH_ELF_H */