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
* @brief       x86 CPU initialisation functions
*/

#include <arch/io.h>

#include <x86/descriptor.h>

#include <lib/string.h>
#include <lib/utility.h>

#include <assert.h>
#include <loader.h>
#include <memory.h>
#include <time.h>

// Frequency of the PIT
#define PIT_FREQUENCY		1193182L

// Frequency of the booting CPU
static uint64_t cpu_frequency = 0;

// ---------------------------------------------------------------------------- [HELPER FUNCS]

// ============================================================================
// Read the Time Stamp Counter
//
// @return 	Value of the TSC
static inline uint64_t
readTimeStampCounter(void) {
	uint32_t high, low;
	__asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
	return ((uint64_t)high << 32) | low;
}

// ============================================================================
// Function to calculate the CPU frequency
//
// @return			Calculated frequency
static uint64_t
calculateCpuFrequency(void) {
	uint16_t shi, slo, ehi, elo, ticks;
	uint64_t start, end, cycles;

	// First set the PIT to rate generator mode
	out8(0x43, 0x34);
	out8(0x40, 0xFF);
	out8(0x40, 0xFF);

	// Wait for the cycle to begin
	do {
		out8(0x43, 0x00);
		slo = in8(0x40);
		shi = in8(0x40);
	} while(shi != 0xFF);

	// Get the start TSC value
	start = readTimeStampCounter();

	// Wait for the high byte to drop to 128
	do {
		out8(0x43, 0x00);
		elo = in8(0x40);
		ehi = in8(0x40);
	} while(ehi > 0x80);

	// Get the end TSC value
	end = readTimeStampCounter();

	// Calculate the differences between the values
	cycles = end - start;
	ticks = ((ehi << 8) | elo) - ((shi << 8) | slo);

	// Calculate frequency
	return (cycles * PIT_FREQUENCY) / ticks;
}

// ---------------------------------------------------------------------------- [REG FUNCS]

// ============================================================================
// spin for a certain amount of time
//
// @param us 		Microseconds to delay for
void
spin(timeout_t us) {
	// Work out where we will finish
	uint64_t target = readTimeStampCounter() + ((cpu_frequency / 1000000) * us);

	// Spin until the target is reached
	while(readTimeStampCounter() < target) {
		__asm__ volatile("pause");
	}
}

// ============================================================================
// Perform initialisation of the CPU
void
cpuInit(void) {
	// Calculate the CPU frequency
	cpu_frequency = calculateCpuFrequency();
}
