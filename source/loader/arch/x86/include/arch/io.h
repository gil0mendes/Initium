/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 <author>
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
 * @brief		x86 I/O functions.
 */

#ifndef __ARCH_IO_H
#define __ARCH_IO_H

#include <types.h>

/**
 * Port I/O functions.
 */

/** Read an 8 bit value from a port.
 * @param port		Port to read from.
 * @return		Value read. */
static inline uint8_t in8(uint16_t port) {
	uint8_t ret;

	__asm__ __volatile__("inb %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}

/** Write an 8 bit value to a port.
 * @param port		Port to write to.
 * @param val		Value to write. */
static inline void out8(uint16_t port, uint8_t val) {
	__asm__ __volatile__("outb %1, %0" :: "dN"(port), "a"(val));
}

/** Read a 16 bit value from a port.
 * @param port		Port to read from.
 * @return		Value read. */
static inline uint16_t in16(uint16_t port) {
	uint16_t ret;

	__asm__ __volatile__("inw %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}

/** Write a 16 bit value to a port.
 * @param port		Port to write to.
 * @param val		Value to write. */
static inline void out16(uint16_t port, uint16_t val) {
	__asm__ __volatile__("outw %1, %0" :: "dN"(port), "a"(val));
}

/** Read a 32 bit value from a port.
 * @param port		Port to read from.
 * @return		Value read. */
static inline uint32_t in32(uint16_t port) {
	uint32_t ret;

	__asm__ __volatile__("inl %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}

/** Write a 32 bit value to a port.
 * @param port		Port to write to.
 * @param val		Value to write. */
static inline void out32(uint16_t port, uint32_t val) {
	__asm__ __volatile__("outl %1, %0" :: "dN"(port), "a"(val));
}

/**
 * Memory mapped I/O functions.
 */

/** Read an 8 bit value from a memory mapped register.
 * @param addr		Address to read from.
 * @return		Value read. */
static inline uint8_t read8(const volatile uint8_t *addr) {
	uint8_t ret;

	__asm__ __volatile__("movb %1, %0" : "=q"(ret) : "m"(*addr) : "memory");
	return ret;
}

/** Write an 8 bit value to a memory mapped register.
 * @param addr		Address to write to.
 * @param val		Value to write. */
static inline void write8(volatile uint8_t *addr, uint8_t val) {
	__asm__ __volatile__("movb %0, %1" :: "q"(val), "m"(*addr) : "memory");
}

/** Read a 16 bit value from a memory mapped register.
 * @param addr		Address to read from.
 * @return		Value read. */
static inline uint16_t read16(const volatile uint16_t *addr) {
	uint16_t ret;

	__asm__ __volatile__("movw %1, %0" : "=r"(ret) : "m"(*addr) : "memory");
	return ret;
}

/** Write a 16 bit value to a memory mapped register.
 * @param addr		Address to write to.
 * @param val		Value to write. */
static inline void write16(volatile uint16_t *addr, uint16_t val) {
	__asm__ __volatile__("movw %0, %1" :: "r"(val), "m"(*addr) : "memory");
}

/** Read a 32 bit value from a memory mapped register.
 * @param addr		Address to read from.
 * @return		Value read. */
static inline uint32_t read32(const volatile uint32_t *addr) {
	uint32_t ret;

	__asm__ __volatile__("movl %1, %0" : "=r"(ret) : "m"(*addr) : "memory");
	return ret;
}

/** Write a 32 bit value to a memory mapped register.
 * @param addr		Address to write to.
 * @param val		Value to write. */
static inline void write32(volatile uint32_t *addr, uint32_t val) {
	__asm__ __volatile__("movl %0, %1" :: "r"(val), "m"(*addr) : "memory");
}

#endif /* __ARCH_IO_H */
