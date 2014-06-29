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
 * @brief             INT 15h interface definitions
 */

#ifndef __BIOS_MEMORY_H
#define __BIOS_MEMORY_H

#include <types.h>

// Value of EAX for the E820 call ('SMAP')
#define E820_SMAP		0x534d4150

// Memory map type values
#define E820_TYPE_FREE		1	    /**< Usable memory. */
#define E820_TYPE_RESERVED	2	    /**< Reserved memory. */
#define E820_TYPE_ACPI_RECLAIM	3	/**< ACPI reclaimable. */
#define E820_TYPE_ACPI_NVS	4	    /**< ACPI NVS. */
#define E820_TYPE_BAD		5	    /**< Bad memory. New in ACPI 3.0. */
#define E820_TYPE_DISABLED	6	    /**< Address range is disabled. New in ACPI 4.0. */

// Attribute bit values
#define E820_ATTR_ENABLED	(1<<0)	/**< Address range is enabled. New in 3.0. In ACPI 4.0+, must be 1. */
#define E820_ATTR_NVS		(1<<1)	/**< Address range is nonvolatile. */
#define E820_ATTR_SLOW		(1<<2)	/**< Access to this memory may be very slow. New in ACPI 4.0. */
#define E820_ATTR_ERRORLOG	(1<<3)	/**< Memory is used to log hardware errors. New in ACPI 4.0. */

// E820 memory map entry structure
typedef struct e820_entry {
	uint64_t start;			/**< Start of range. */
	uint64_t length;		/**< Length of range. */
	uint32_t type;			/**< Type of range. */
	uint32_t attr;			/**< Attributes of range. */
} __packed e820_entry_t;

#endif /* __BIOS_MEMORY_H */
