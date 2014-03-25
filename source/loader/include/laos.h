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
 * @brief		LAOS boot protocol definitions.
 */

#ifndef __LAOS_H
#define __LAOS_H

// Magic number passed to the entry point of a LAOS kernel.
#define LAOS_MAGIC			0xb007cafe

// Current LAOS version.
#define LAOS_VERSION			1

#ifndef __ASM__

#include <types.h>

// Type used to store a physical address.
typedef uint64_t laos_paddr_t;

// Type used to store a virtual address.
typedef uint64_t laos_vaddr_t;

///// Information tags.

// LAOS information tag header structure.
typedef struct laos_tag 
{
	uint32_t type;				// Type of the tag.
	uint32_t size;				// Total size of the tag data.
} laos_tag_t;

// Possible information tag types.
#define LAOS_TAG_NONE			0	// End of tag list.
#define LAOS_TAG_CORE			1	// Core information tag (always present).
#define LAOS_TAG_OPTION			2	// Kernel option.
#define LAOS_TAG_MEMORY			3	// Physical memory range.
#define LAOS_TAG_VMEM			4	// Virtual memory range.
#define LAOS_TAG_PAGETABLES		5	// Page table information (architecture-specific).
#define LAOS_TAG_MODULE			6	// Boot module.
#define LAOS_TAG_VIDEO			7	// Video mode information.
#define LAOS_TAG_BOOTDEV		8	// Boot device information.
#define LAOS_TAG_LOG			9	// Kernel log buffer.
#define LAOS_TAG_SECTIONS		10	// ELF section information.
#define LAOS_TAG_E820			11	// BIOS address range descriptor (PC-specific).

// Tag containing core information for the kernel.
typedef struct laos_tag_core 
{
	laos_tag_t header;				// Tag header.

	laos_paddr_t tags_phys;			// Physical address of the tag list.
	uint32_t tags_size;				// Total size of the tag list (rounded to 8 bytes).
	uint32_t _pad;

	laos_paddr_t kernel_phys;		// Physical address of the kernel image.

	laos_vaddr_t stack_base;		// Virtual address of the boot stack.
	laos_paddr_t stack_phys;		// Physical address of the boot stack.
	uint32_t stack_size;			// Size of the boot stack.
} laos_tag_core_t;

// Tag containing an option passed to the kernel.
typedef struct laos_tag_option 
{
	laos_tag_t header;			// Tag header.

	uint8_t type;				// Type of the option.
	uint32_t name_size;			// Length of name string, including null terminator.
	uint32_t value_size;		// Size of the option value, in bytes.
} laos_tag_option_t;

// Possible option types.
#define LAOS_OPTION_BOOLEAN		0	// Boolean.
#define LAOS_OPTION_STRING		1	// String.
#define LAOS_OPTION_INTEGER		2	// Integer.

// Tag describing a physical memory range.
typedef struct laos_tag_memory 
{
	laos_tag_t header;			// Tag header.

	laos_paddr_t start;			// Start of the memory range.
	laos_paddr_t size;			// Size of the memory range.
	uint8_t type;				// Type of the memory range.
} laos_tag_memory_t;

// Possible memory range types.
#define LAOS_MEMORY_FREE		0	// Free, usable memory.
#define LAOS_MEMORY_ALLOCATED	1	// Kernel image and other non-reclaimable data.
#define LAOS_MEMORY_RECLAIMABLE	2	// Memory reclaimable when boot information is no longer needed.
#define LAOS_MEMORY_PAGETABLES	3	// Temporary page tables for the kernel.
#define LAOS_MEMORY_STACK		4	// Stack set up for the kernel.
#define LAOS_MEMORY_MODULES		5	// Module data.

// Tag describing a virtual memory range.
typedef struct laos_tag_vmem 
{
	laos_tag_t header;			// Tag header.

	laos_vaddr_t start;			// Start of the virtual memory range.
	laos_vaddr_t size;			// Size of the virtual memory range.
	laos_paddr_t phys;			// Physical address that this range maps to.
} laos_tag_vmem_t;

// Tag describing a boot module.
typedef struct laos_tag_module 
{
	laos_tag_t header;			// Tag header.

	laos_paddr_t addr;			// Address of the module.
	uint32_t size;				// Size of the module.
	uint32_t name_size;			// Length of name string, including null terminator.
} laos_tag_module_t;

// Structure describing an RGB colour.
typedef struct laos_colour 
{
	uint8_t red;				// Red value.
	uint8_t green;				// Green value.
	uint8_t blue;				// Blue value.
} laos_colour_t;

// Tag containing video mode information.
typedef struct laos_tag_video 
{
	laos_tag_t header;			// Tag header.

	uint32_t type;				// Type of the video mode set up.
	uint32_t _pad;

	union 
	{
		// VGA text mode information.
		struct 
		{
			uint8_t cols;			// Columns on the text display.
			uint8_t lines;			// Lines on the text display.
			uint8_t x;				// Cursor X position.
			uint8_t y;				// Cursor Y position.
			uint32_t _pad;
			laos_paddr_t mem_phys;	// Physical address of VGA memory.
			laos_vaddr_t mem_virt;	// Virtual address of VGA memory.
			uint32_t mem_size;		// Size of VGA memory mapping.
		} vga;

		// Linear framebuffer mode information.
		struct 
		{
			uint32_t flags;			// LFB properties.
			uint32_t width;			// Width of video mode, in pixels.
			uint32_t height;		// Height of video mode, in pixels.
			uint8_t bpp;			// Number of bits per pixel.
			uint32_t pitch;			// Number of bytes per line of the framebuffer.
			uint32_t _pad;
			laos_paddr_t fb_phys;	// Physical address of the framebuffer.
			laos_vaddr_t fb_virt;	// Virtual address of a mapping of the framebuffer.
			uint32_t fb_size;		// Size of the virtual mapping.
			uint8_t red_size;		// Size of red component of each pixel.
			uint8_t red_pos;		// Bit position of the red component of each pixel.
			uint8_t green_size;		// Size of green component of each pixel.
			uint8_t green_pos;		// Bit position of the green component of each pixel.
			uint8_t blue_size;		// Size of blue component of each pixel.
			uint8_t blue_pos;		// Bit position of the blue component of each pixel.
			uint16_t palette_size;	// For indexed modes, size of the colour palette.

			// For indexed modes, the colour palette set by the loader.
			laos_colour_t palette[0];
		} lfb;
	};
} laos_tag_video_t;

// Video mode types.
#define LAOS_VIDEO_VGA			(1<<0)	// VGA text mode.
#define LAOS_VIDEO_LFB			(1<<1)	// Linear framebuffer.

// Linear framebuffer flags.
#define LAOS_LFB_RGB			(1<<0)	// Direct RGB colour format.
#define LAOS_LFB_INDEXED		(1<<1)	// Indexed colour format.

// Type used to store a MAC address.
typedef uint8_t laos_mac_addr_t[16];

// Type used to store an IPv4 address.
typedef uint8_t laos_ipv4_addr_t[4];

// Type used to store an IPv6 address.
typedef uint8_t laos_ipv6_addr_t[16];

// Type used to store an IP address.
typedef union laos_ip_addr 
{
	laos_ipv4_addr_t v4;			// IPv4 address.
	laos_ipv6_addr_t v6;			// IPv6 address.
} laos_ip_addr_t;

// Tag containing boot device information.
typedef struct laos_tag_bootdev 
{
	laos_tag_t header;			// Tag header.

	uint32_t type;				// Boot device type.

	union 
	{
		// Disk device information.
		struct 
		{
			uint32_t flags;			// Behaviour flags.
			uint8_t uuid[64];		// UUID of the boot filesystem.
			uint8_t device;			// Device ID (platform-specific).
			uint8_t partition;		// Partition number.
			uint8_t sub_partition;	// Sub-partition number.
		} disk;

		// Network boot information.
		struct 
		{
			uint32_t flags;		// Behaviour flags.

			// Server IP address.
			laos_ip_addr_t server_ip;

			// UDP port number of TFTP server.
			uint16_t server_port;

			// Gateway IP address.
			laos_ip_addr_t gateway_ip;

			// IP used on this machine when communicating with server.
			laos_ip_addr_t client_ip;

			// MAC address of the boot network interface.
			laos_mac_addr_t client_mac;

			// Network interface type.
			uint8_t hw_type;

			// Hardware address length.
			uint8_t hw_addr_len;
		} net;
	};
} laos_tag_bootdev_t;

// Boot device types.
#define LAOS_BOOTDEV_NONE		0	// No boot device (e.g. boot image).
#define LAOS_BOOTDEV_DISK		1	// Booted from a disk device.
#define LAOS_BOOTDEV_NET		2	// Booted from the network.

// Network boot behaviour flags.
#define LAOS_NET_IPV6			(1<<0)	// Given addresses are IPv6 addresses.

// Tag describing the kernel log buffer.
typedef struct laos_tag_log 
{
	laos_tag_t header;			// Tag header.

	laos_vaddr_t log_virt;		// Virtual address of log buffer.
	laos_paddr_t log_phys;		// Physical address of log buffer.
	uint32_t log_size;			// Size of log buffer.
	uint32_t _pad;

	laos_paddr_t prev_phys;		// Physical address of previous log buffer.
	uint32_t prev_size;			// Size of previous log buffer.
} laos_tag_log_t;

// Structure of a log buffer.
typedef struct laos_log 
{
	uint32_t magic;				// Magic value used by loader (should not be overwritten).

	uint32_t start;				// Offset in the buffer of the start of the log.
	uint32_t length;			// Number of characters in the log buffer.

	uint32_t info[3];			// Fields for use by the kernel.
	uint8_t buffer[0];			// Log data.
} laos_log_t;

// Tag describing ELF section headers.
typedef struct laos_tag_sections 
{
	laos_tag_t header;			// Tag header.

	uint32_t num;				// Number of section headers.
	uint32_t entsize;			// Size of each section header.
	uint32_t shstrndx;			// Section name string table index.

	uint32_t _pad;

	uint8_t sections[0];		// Section data.
} laos_tag_sections_t;

// Tag containing an E820 address range descriptor (PC-specific).
typedef struct laos_tag_e820 
{
	laos_tag_t header;			// Tag header.

	uint64_t start;
	uint64_t length;
	uint32_t type;
	uint32_t attr;
} laos_tag_e820_t;

// Tag containing page table information.
typedef struct laos_tag_pagetables 
{
	laos_tag_t header;			// Tag header.

#if defined(__i386__)
	laos_paddr_t page_dir;		// Physical address of the page directory.
	laos_vaddr_t mapping;		// Virtual address of recursive mapping.
#elif defined(__x86_64__)
	laos_paddr_t pml4;			// Physical address of the PML4.
	laos_vaddr_t mapping;		// Virtual address of recursive mapping.
#elif defined(__arm__)
	laos_paddr_t l1;			// Physical address of the first level page table.
	laos_vaddr_t mapping;		// Virtual address of temporary mapping region.
#endif
} laos_tag_pagetables_t;

///// Image tags.

// LAOS ELF note name.
#define LAOS_NOTE_NAME			"LAOS"

// Section type definition.
#ifdef __arm__
# define LAOS_SECTION_TYPE		"%note"
#else
# define LAOS_SECTION_TYPE		"@note"
#endif

// LAOS image tag types (used as ELF note type field).
#define LAOS_ITAG_IMAGE		0	// Basic image information (required).
#define LAOS_ITAG_LOAD		1	// Memory layout options.
#define LAOS_ITAG_OPTION	2	// Option description.
#define LAOS_ITAG_MAPPING	3	// Virtual memory mapping description.
#define LAOS_ITAG_VIDEO		4	// Requested video mode.

// Image tag containing basic image information.
typedef struct laos_itag_image 
{
	uint32_t version;			// LAOS version that the image is using.
	uint32_t flags;				// Flags for the image.
} laos_itag_image_t;

// Flags controlling optional features.
#define LAOS_IMAGE_SECTIONS		(1<<0)	// Load ELF sections and pass a sections tag.
#define LAOS_IMAGE_LOG			(1<<1)	// Enable the kernel log facility.

// Macro to declare an image itag.
#define LAOS_IMAGE(flags) \
	__asm__( \
		"   .pushsection \".note.laos.image\", \"a\", " LAOS_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 3f - 2f\n" \
		"   .long " XSTRINGIFY(LAOS_ITAG_IMAGE) "\n" \
		"0: .asciz \"LAOS\"\n" \
		"1: .p2align 2\n" \
		"2: .long " XSTRINGIFY(LAOS_VERSION) "\n" \
		"   .long " STRINGIFY(flags) "\n" \
		"3: .p2align 2\n" \
		"   .popsection\n")

// Image tag specifying loading parameters.
typedef struct laos_itag_load 
{
	uint32_t flags;					// Flags controlling load behaviour.
	uint32_t _pad;
	laos_paddr_t alignment;			// Requested physical alignment of kernel image.
	laos_paddr_t min_alignment;		// Minimum physical alignment of kernel image.
	laos_vaddr_t virt_map_base;		// Base of virtual mapping range.
	laos_vaddr_t virt_map_size;		// Size of virtual mapping range.
} laos_itag_load_t;

// Flags controlling load behaviour.
#define LAOS_LOAD_FIXED		(1<<0)	// Load at a fixed physical address.

// Macro to declare a load itag.
#define LAOS_LOAD(flags, alignment, min_alignment, virt_map_base, virt_map_size) \
	__asm__( \
		"   .pushsection \".note.laos.load\", \"a\", " LAOS_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 3f - 2f\n" \
		"   .long " XSTRINGIFY(LAOS_ITAG_LOAD) "\n" \
		"0: .asciz \"LAOS\"\n" \
		"1: .p2align 2\n" \
		"2: .long " STRINGIFY(flags) "\n" \
		"   .long 0\n" \
		"   .quad " STRINGIFY(alignment) "\n" \
		"   .quad " STRINGIFY(min_alignment) "\n" \
		"   .quad " STRINGIFY(virt_map_base) "\n" \
		"   .quad " STRINGIFY(virt_map_size) "\n" \
		"3: .p2align 2\n" \
		"   .popsection\n")

// Image tag containing an option description.
typedef struct laos_itag_option 
{
	uint8_t type;				// Type of the option.
	uint32_t name_len;			// Length of the option name.
	uint32_t desc_len;			// Length of the option description.
	uint32_t default_len;		// Length of the default value.
} laos_itag_option_t;

// Macro to declare a boolean option itag.
#define LAOS_BOOLEAN_OPTION(name, desc, default) \
	__asm__( \
		"   .pushsection \".note.laos.option." name "\", \"a\", " LAOS_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 6f - 2f\n" \
		"   .long " XSTRINGIFY(LAOS_ITAG_OPTION) "\n" \
		"0: .asciz \"LAOS\"\n" \
		"1: .p2align 2\n" \
		"2: .byte " XSTRINGIFY(LAOS_OPTION_BOOLEAN) "\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .long 4f - 3f\n" \
		"   .long 5f - 4f\n" \
		"   .long 1\n" \
		"3: .asciz \"" name "\"\n" \
		"4: .asciz \"" desc "\"\n" \
		"5: .byte " STRINGIFY(default) "\n" \
		"6: .p2align 2\n" \
		"   .popsection\n")

// Macro to declare an integer option itag.
#define LAOS_INTEGER_OPTION(name, desc, default) \
	__asm__( \
		"   .pushsection \".note.laos.option." name "\", \"a\", " LAOS_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 6f - 2f\n" \
		"   .long " XSTRINGIFY(LAOS_ITAG_OPTION) "\n" \
		"0: .asciz \"LAOS\"\n" \
		"1: .p2align 2\n" \
		"2: .byte " XSTRINGIFY(LAOS_OPTION_INTEGER) "\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .long 4f - 3f\n" \
		"   .long 5f - 4f\n" \
		"   .long 8\n" \
		"3: .asciz \"" name "\"\n" \
		"4: .asciz \"" desc "\"\n" \
		"5: .quad " STRINGIFY(default) "\n" \
		"6: .p2align 2\n" \
		"   .popsection\n")

// Macro to declare an string option itag.
#define LAOS_STRING_OPTION(name, desc, default) \
	__asm__( \
		"   .pushsection \".note.laos.option." name "\", \"a\", " LAOS_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 6f - 2f\n" \
		"   .long " XSTRINGIFY(LAOS_ITAG_OPTION) "\n" \
		"0: .asciz \"LAOS\"\n" \
		"1: .p2align 2\n" \
		"2: .byte " XSTRINGIFY(LAOS_OPTION_STRING) "\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .long 4f - 3f\n" \
		"   .long 5f - 4f\n" \
		"   .long 6f - 5f\n" \
		"3: .asciz \"" name "\"\n" \
		"4: .asciz \"" desc "\"\n" \
		"5: .asciz \"" default "\"\n" \
		"6: .p2align 2\n" \
		"   .popsection\n")

// Image tag containing a virtual memory mapping description.
typedef struct laos_itag_mapping 
{
	laos_vaddr_t virt;			// Virtual address to map.
	laos_paddr_t phys;			// Physical address to map to.
	laos_vaddr_t size;			// Size of mapping to make.
} laos_itag_mapping_t;

// Macro to declare a virtual memory mapping itag.
#define LAOS_MAPPING(virt, phys, size) \
	__asm__( \
		"   .pushsection \".note.laos.mapping.b" # virt "\", \"a\", " LAOS_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 3f - 2f\n" \
		"   .long " XSTRINGIFY(LAOS_ITAG_MAPPING) "\n" \
		"0: .asciz \"LAOS\"\n" \
		"1: .p2align 2\n" \
		"2: .quad " STRINGIFY(virt) "\n" \
		"   .quad " STRINGIFY(phys) "\n" \
		"   .quad " STRINGIFY(size) "\n" \
		"3: .p2align 2\n" \
		"   .popsection\n")

// Image tag specifying the kernel's requested video mode.
typedef struct laos_itag_video 
{
	uint32_t types;				// Supported video mode types.
	uint32_t width;				// Preferred LFB width.
	uint32_t height;			// Preferred LFB height.
	uint8_t bpp;				// Preferred LFB bits per pixel.
} laos_itag_video_t;

// Macro to declare a video mode itag.
#define LAOS_VIDEO(types, width, height, bpp) \
	__asm__( \
		"   .pushsection \".note.laos.video\", \"a\", " LAOS_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 3f - 2f\n" \
		"   .long " XSTRINGIFY(LAOS_ITAG_VIDEO) "\n" \
		"0: .asciz \"LAOS\"\n" \
		"1: .p2align 2\n" \
		"2: .long " STRINGIFY(types) "\n" \
		"   .long " STRINGIFY(width) "\n" \
		"   .long " STRINGIFY(height) "\n" \
		"   .byte " STRINGIFY(bpp) "\n" \
		"3: .p2align 2\n" \
		"   .popsection\n")

#endif // __ASM__
#endif // __LAOS_H
