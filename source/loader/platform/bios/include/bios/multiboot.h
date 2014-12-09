/**
* The MIT License (MIT)
*
* Copyright (c) 2014 Gil Mendes <gil00mendes@gmail.com>
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
* Multiboot header
*/

#ifndef __PLATFORM_MULTIBOOT_H
#define __PLATFORM_MULTIBOOT_H

// Magic value passed by the bootloader
#define MB_LOADER_MAGIC			0x2BADB002

// Magic value in the Multiboot header
#define MB_HEADER_MAGIC			0x1BADB002

// Flags for the Multiboot header
#define MB_HEADER_MODALIGN		(1<<0)	// Align loaded modules on page boundaries
#define MB_HEADER_MEMINFO		(1<<1)	// Kernel wants a memory map
#define MB_HEADER_KLUDGE		(1<<16)	// Use a.out kludge

/** Flags passed by the bootloader. */
#define	MB_INFO_MEMINFO			(1<<0)	// Bootloader provided memory info
#define MB_INFO_BOOTDEV			(1<<1)	// Bootloader provided boot device
#define MB_INFO_CMDLINE			(1<<2)	// Bootloader provided command line
#define MB_INFO_MODULES			(1<<3)	// Bootloader provided module info
#define MB_INFO_AOUTSYMS		(1<<4)	// Bootloader provided a.out symbols
#define MB_INFO_ELFSYMS			(1<<5)	// Bootloader provided ELF symbols
#define MB_INFO_MMAP			(1<<6)	// Bootloader provided memory map
#define MB_INFO_DRIVES			(1<<7)	// Bootloader provided drive information
#define MB_INFO_CONFTABLE		(1<<8)	// Bootloader provided config table
#define MB_INFO_LDRNAME			(1<<9)	// Bootloader provided its name
#define MB_INFO_APMTABLE		(1<<10)	// Bootloader provided APM table
#define MB_INFO_VBEINFO			(1<<11)	// Bootloader provided VBE info

// Size of the Multiboot structures
#define MB_INFO_SIZE			88
#define MB_MODULE_SIZE			16

// Offsets into the info structure required in assembly code
#define MB_INFO_OFFSET_BOOT_DEVICE	12	// Offset of the boot device field
#define MB_INFO_OFFSET_CMDLINE		16	// Offset of the command line field
#define MB_INFO_OFFSET_MODS_COUNT	20	// Offset of the module count field
#define MB_INFO_OFFSET_MODS_ADDR	24	// Offset of the module address field

// Offsets into the module structure required in assembly code
#define MB_MODULE_OFFSET_MOD_START	0	// Offset of the module start field
#define MB_MODULE_OFFSET_MOD_END	4	// Offset of the module end field
#define MB_MODULE_OFFSET_CMDLINE	8	// Offset of the command line field

#ifndef __ASM__

#include <types.h>

// Multiboot information structure
typedef struct multiboot_info {
    uint32_t flags;				    // Flags
    uint32_t mem_lower;			    // Bytes of lower memory
    uint32_t mem_upper;			    // Bytes of upper memory
    uint32_t boot_device;			// Boot device
    uint32_t cmdline;			    // Address of kernel command line
    uint32_t mods_count;			// Module count
    uint32_t mods_addr;			    // Address of module structures
    uint32_t elf_sec[4];			// ELF section headers
    uint32_t mmap_length;			// Memory map length
    uint32_t mmap_addr;			    // Address of memory map
    uint32_t drives_length;			// Drive information length
    uint32_t drives_addr;			// Address of drive information
    uint32_t config_table;			// Configuration table
    uint32_t boot_loader_name;		// Boot loader name
    uint32_t apm_table;			    // APM table
    uint32_t vbe_control_info;		// VBE control information
    uint32_t vbe_mode_info;			// VBE mode information
    uint16_t vbe_mode;			    // VBE mode
    uint16_t vbe_interface_seg;		// VBE interface segment
    uint16_t vbe_interface_off;		// VBE interface offset
    uint16_t vbe_interface_len;		// VBE interface length
} __packed multiboot_info_t;

// Multiboot module information structure
typedef struct multiboot_module {
    uint32_t mod_start;			// Module start address
    uint32_t mod_end;			// Module end address
    uint32_t cmdline;			// Module command line
    uint32_t pad;
} __packed multiboot_module_t;

extern uint32_t multiboot_magic;
extern multiboot_info_t multiboot_info;

extern void multiboot_init(void);

#endif // __ASM__
#endif // __PLATFORM_MULTIBOOT_H
