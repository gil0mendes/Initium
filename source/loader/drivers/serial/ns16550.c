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
 * @brief		16550 UART driver.
 */
#include <arch/io.h>

#include <drivers/serial/ns16550.h>

#include <console.h>
#include <loader.h>

/** Base of the 16550 registers. */
static ns16550_t ns16550_base;

#ifdef CONFIG_TARGET_NS16550_IO

/** Read a UART register.
 * @param reg		Register to read.
 * @return		Value read. */
static inline uint8_t ns16550_read(int reg) {
	return in8(ns16550_base + reg);
}

/** Write a UART register.
 * @param reg		Register to read.
 * @param value		Value to write. */
static inline void ns16550_write(int reg, uint8_t value) {
	out8(ns16550_base + reg, value);
}

#else /* CONFIG_TARGET_NS16550_IO */

#ifndef CONFIG_TARGET_NS16550_REG_SHIFT
# define CONFIG_TARGET_NS16550_REG_SHIFT	2
#endif

/** Read a UART register.
 * @param reg		Register to read.
 * @return		Value read. */
static inline uint8_t ns16550_read(int reg) {
	reg = reg << CONFIG_TARGET_NS16550_REG_SHIFT;
	return read8((void *)(ns16550_base + reg));
}

/** Write a UART register.
 * @param reg		Register to read.
 * @param value		Value to write. */
static inline void ns16550_write(int reg, uint8_t value) {
	reg = reg << CONFIG_TARGET_NS16550_REG_SHIFT;
	write8((void *)(ns16550_base + reg), value);
}

#endif /* CONFIG_TARGET_NS16550_IO */

/** Write a character to the serial console.
 * @param ch		Character to write. */
static void ns16550_console_putc(char ch) {
	if(ch == '\n')
		ns16550_console_putc('\r');

	ns16550_write(NS16550_REG_THR, ch);
	while(!(ns16550_read(NS16550_REG_LSR) & NS16550_LSR_THRE));
}

/** NS16550 UART debug console. */
static console_t ns16550_console = {
	.putc = ns16550_console_putc,
};

/**
 * Initialise the NS16550 UART as the debug console.
 *
 * Sets the NS16550 UART as the debug console. This function does not
 * reconfigure the UART, that is done by ns16550_config(). The reason for this
 * split is for platforms where we know the UART has already been configured
 * prior to the loader being executed, meaning we can keep that configuration.
 *
 * @param base		Base of NS16550 registers.
 */
void ns16550_init(ns16550_t base) {
	ns16550_base = base;

	/* Wait for the transmit buffer to empty. */
	while(!(ns16550_read(NS16550_REG_LSR) & NS16550_LSR_THRE));

	/* Set the debug console. */
	debug_console = &ns16550_console;
}

/** Reconfigure the NS16550 UART.
 * @param clock_rate	Base clock rate.
 * @param baud_rate	Baud rate to use. */
void ns16550_config(uint32_t clock_rate, unsigned baud_rate) {
	uint16_t divisor;

	/* Disable all interrupts, disable the UART while configuring. */
	ns16550_write(NS16550_REG_IER, 0);
	ns16550_write(NS16550_REG_FCR, 0);

	/* Set DLAB to enable access to divisor registers. */
	ns16550_write(NS16550_REG_LCR, NS16550_LCR_DLAB);

	/* Program the divisor to set the baud rate. */
	divisor = (clock_rate / 16) / baud_rate;
	ns16550_write(NS16550_REG_DLL, divisor & 0xff);
	ns16550_write(NS16550_REG_DLH, (divisor >> 8) & 0x3f);

	/* Switch to operational mode. */
	ns16550_write(NS16550_REG_LCR, NS16550_LCR_WLS_8);

	/* Clear and enable FIFOs. */
	ns16550_write(NS16550_REG_FCR, NS16550_FCR_FIFO_EN
		| NS16550_FCR_CLEAR_RX | NS16550_FCR_CLEAR_TX);

	/* Enable RTS/DTR. */
	ns16550_write(NS16550_REG_MCR, NS16550_MCR_DTR | NS16550_MCR_RTS);
}
