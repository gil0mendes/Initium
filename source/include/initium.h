/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2012-2015 Gil Mendes
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
 * Initium boot protocol definitions.
 */

#ifndef __INITIUM_H
#define __INITIUM_H

// Magic number passed to the entry point of a Initium kernel.
#define INITIUM_MAGIC			0xb007cafe

// Current Initium version.
#define INITIUM_VERSION			1

#ifndef __ASM__

#include <types.h>

// Type used to store a physical address.
typedef uint64_t initium_paddr_t;

// Type used to store a virtual address.
typedef uint64_t initium_vaddr_t;

///// Information tags.

// Initium information tag header structure.
typedef struct initium_tag
{
	uint32_t type;				// Type of the tag.
	uint32_t size;				// Total size of the tag data.
} initium_tag_t;

// Possible information tag types.
#define INITIUM_TAG_NONE			0	// End of tag list.
#define INITIUM_TAG_CORE			1	// Core information tag (always present).
#define INITIUM_TAG_OPTION			2	// Kernel option.
#define INITIUM_TAG_MEMORY			3	// Physical memory range.
#define INITIUM_TAG_VMEM			4	// Virtual memory range.
#define INITIUM_TAG_PAGETABLES		5	// Page table information (architecture-specific).
#define INITIUM_TAG_MODULE			6	// Boot module.
#define INITIUM_TAG_VIDEO			7	// Video mode information.
#define INITIUM_TAG_BOOTDEV		8	// Boot device information.
#define INITIUM_TAG_LOG			9	// Kernel log buffer.
#define INITIUM_TAG_SECTIONS		10	// ELF section information.
#define INITIUM_TAG_E820			11	// BIOS address range descriptor (PC-specific).

// Tag containing core information for the kernel.
typedef struct initium_tag_core
{
	initium_tag_t header;				// Tag header.

	initium_paddr_t tags_phys;			// Physical address of the tag list.
	uint32_t tags_size;				// Total size of the tag list (rounded to 8 bytes).
	uint32_t _pad;

	initium_paddr_t kernel_phys;		// Physical address of the kernel image.

	initium_vaddr_t stack_base;		// Virtual address of the boot stack.
	initium_paddr_t stack_phys;		// Physical address of the boot stack.
	uint32_t stack_size;			// Size of the boot stack.
} initium_tag_core_t;

// Tag containing an option passed to the kernel.
typedef struct initium_tag_option
{
	initium_tag_t header;			// Tag header.

	uint8_t type;					// Type of the option.
	uint32_t name_len;				// Length of name string, including null terminator.
	uint32_t value_len;			// Size of the option value, in bytes.
} initium_tag_option_t;

// Possible option types.
#define INITIUM_OPTION_BOOLEAN		0	// Boolean.
#define INITIUM_OPTION_STRING		1	// String.
#define INITIUM_OPTION_INTEGER		2	// Integer.

// Tag describing a physical memory range.
typedef struct initium_tag_memory
{
	initium_tag_t header;			// Tag header.

	initium_paddr_t start;			// Start of the memory range.
	initium_paddr_t size;			// Size of the memory range.
	uint8_t type;					// Type of the memory range.
} initium_tag_memory_t;

// Possible memory range types.
#define INITIUM_MEMORY_FREE		0	// Free, usable memory.
#define INITIUM_MEMORY_ALLOCATED	1	// Kernel image and other non-reclaimable data.
#define INITIUM_MEMORY_RECLAIMABLE	2	// Memory reclaimable when boot information is no longer needed.
#define INITIUM_MEMORY_PAGETABLES	3	// Temporary page tables for the kernel.
#define INITIUM_MEMORY_STACK		4	// Stack set up for the kernel.
#define INITIUM_MEMORY_MODULES		5	// Module data.

// Tag describing a virtual memory range.
typedef struct initium_tag_vmem
{
	initium_tag_t header;			// Tag header.

	initium_vaddr_t start;			// Start of the virtual memory range.
	initium_vaddr_t size;			// Size of the virtual memory range.
	initium_paddr_t phys;			// Physical address that this range maps to.
} initium_tag_vmem_t;

// Tag describing a boot module.
typedef struct initium_tag_module
{
	initium_tag_t header;			// Tag header.

	initium_paddr_t addr;			// Address of the module.
	uint32_t size;				// Size of the module.
	uint32_t name_len;			// Length of name string, including null terminator.
} initium_tag_module_t;

// Structure describing an RGB colour.
typedef struct initium_colour
{
	uint8_t red;				// Red value.
	uint8_t green;				// Green value.
	uint8_t blue;				// Blue value.
} initium_colour_t;

// Tag containing video mode information.
typedef struct initium_tag_video
{
	initium_tag_t header;			// Tag header.

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
			initium_paddr_t mem_phys;	// Physical address of VGA memory.
			initium_vaddr_t mem_virt;	// Virtual address of VGA memory.
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
			initium_paddr_t fb_phys;	// Physical address of the framebuffer.
			initium_vaddr_t fb_virt;	// Virtual address of a mapping of the framebuffer.
			uint32_t fb_size;		// Size of the virtual mapping.
			uint8_t red_size;		// Size of red component of each pixel.
			uint8_t red_pos;		// Bit position of the red component of each pixel.
			uint8_t green_size;		// Size of green component of each pixel.
			uint8_t green_pos;		// Bit position of the green component of each pixel.
			uint8_t blue_size;		// Size of blue component of each pixel.
			uint8_t blue_pos;		// Bit position of the blue component of each pixel.
			uint16_t palette_size;	// For indexed modes, size of the colour palette.

			// For indexed modes, the colour palette set by the loader.
			initium_colour_t palette[0];
		} lfb;
	};
} initium_tag_video_t;

// Video mode types.
#define INITIUM_VIDEO_VGA			(1<<0)	// VGA text mode.
#define INITIUM_VIDEO_LFB			(1<<1)	// Linear framebuffer.

// Linear framebuffer flags.
#define INITIUM_LFB_RGB			(1<<0)	// Direct RGB colour format.
#define INITIUM_LFB_INDEXED		(1<<1)	// Indexed colour format.

// Type used to store a MAC address.
typedef uint8_t initium_mac_addr_t[16];

// Type used to store an IPv4 address.
typedef uint8_t initium_ipv4_addr_t[4];

// Type used to store an IPv6 address.
typedef uint8_t initium_ipv6_addr_t[16];

// Type used to store an IP address.
typedef union initium_ip_addr
{
	initium_ipv4_addr_t v4;			// IPv4 address.
	initium_ipv6_addr_t v6;			// IPv6 address.
} initium_ip_addr_t;

// Tag containing boot device information.
typedef struct initium_tag_bootdev
{
	initium_tag_t header;			// Tag header.

	uint32_t type;				// Boot device type.

	union
	{
		// Disk device information.
		struct
		{
			uint32_t flags;			// Behaviour flags.
			uint8_t uuid[64];		// UUID of the boot filesystem.
		} disk;

		// Network boot information.
		struct
		{
			uint32_t flags;		// Behaviour flags.

			// Server IP address.
			initium_ip_addr_t server_ip;

			// UDP port number of TFTP server.
			uint16_t server_port;

			// Gateway IP address.
			initium_ip_addr_t gateway_ip;

			// IP used on this machine when communicating with server.
			initium_ip_addr_t client_ip;

			// MAC address of the boot network interface.
			initium_mac_addr_t client_mac;

			// Network interface type.
			uint8_t hw_type;

			// Hardware address length.
			uint8_t hw_addr_len;
		} net;

		/** Other device information. */
		struct {
			uint32_t str_len;		/**< Length of device identification string. */
		} other;
	};
} initium_tag_bootdev_t;

// Boot device types.
#define INITIUM_BOOTDEV_NONE		0	/**> No boot device (e.g. boot image). */
#define INITIUM_BOOTDEV_DISK		1	/**> Booted from a disk device. */
#define INITIUM_BOOTDEV_NET			2	/**> Booted from the network. */
#define INITIUM_BOOTDEV_OTHER		3	/**< Other device (specified by string). */

// Network boot behaviour flags.
#define INITIUM_NET_IPV6			(1<<0)	// Given addresses are IPv6 addresses.

// Tag describing the kernel log buffer.
typedef struct initium_tag_log
{
	initium_tag_t header;			// Tag header.

	initium_vaddr_t log_virt;		// Virtual address of log buffer.
	initium_paddr_t log_phys;		// Physical address of log buffer.
	uint32_t log_size;			// Size of log buffer.
	uint32_t _pad;

	initium_paddr_t prev_phys;		// Physical address of previous log buffer.
	uint32_t prev_size;			// Size of previous log buffer.
} initium_tag_log_t;

// Structure of a log buffer.
typedef struct initium_log
{
	uint32_t magic;				// Magic value used by loader (should not be overwritten).

	uint32_t start;				// Offset in the buffer of the start of the log.
	uint32_t length;			// Number of characters in the log buffer.

	uint32_t info[3];			// Fields for use by the kernel.
	uint8_t buffer[0];			// Log data.
} initium_log_t;

// Tag describing ELF section headers.
typedef struct initium_tag_sections
{
	initium_tag_t header;			// Tag header.

	uint32_t num;				// Number of section headers.
	uint32_t entsize;			// Size of each section header.
	uint32_t shstrndx;			// Section name string table index.

	uint32_t _pad;

	uint8_t sections[0];		// Section data.
} initium_tag_sections_t;

// Tag containing an E820 address range descriptor (PC-specific).
typedef struct initium_tag_e820
{
	initium_tag_t header;			// Tag header.

	uint64_t start;
	uint64_t length;
	uint32_t type;
	uint32_t attr;
} initium_tag_e820_t;

/** Tag containing page table information (IA32). */
typedef struct initium_tag_pagetables_ia32 {
    initium_tag_t header;                     /**< Tag header. */

    initium_paddr_t page_dir;                 /**< Physical address of the page directory. */
    initium_vaddr_t mapping;                  /**< Virtual address of recursive mapping. */
} initium_tag_pagetables_ia32_t;

/** Tag containing page table information (AMD64). */
typedef struct initium_tag_pagetables_amd64 {
    initium_tag_t header;                     /**< Tag header. */

    initium_paddr_t pml4;                     /**< Physical address of the page directory. */
    initium_vaddr_t mapping;                  /**< Virtual address of recursive mapping. */
} initium_tag_pagetables_amd64_t;

/** Tag containing page table information (ARM). */
typedef struct initium_tag_pagetables_arm {
    initium_tag_t header;                     /**< Tag header. */

    initium_paddr_t l1;                       /**< Physical address of the first level page table. */
    initium_vaddr_t mapping;                  /**< Virtual address of temporary mapping region. */
} initium_tag_pagetables_arm_t;

/** Tag containing page table information. */
#if defined(__i386__)
    typedef initium_tag_pagetables_ia32_t initium_tag_pagetables_t;
#elif defined(__x86_64__)
    typedef initium_tag_pagetables_amd64_t initium_tag_pagetables_t;
#elif defined(__arm__)
    typedef initium_tag_pagetables_arm_t initium_tag_pagetables_t;
#endif

///// Image tags.

// Initium ELF note name.
#define INITIUM_NOTE_NAME			"INITIUM"

// Section type definition.
#ifdef __arm__
# define INITIUM_SECTION_TYPE		"%note"
#else
# define INITIUM_SECTION_TYPE		"@note"
#endif

// Initium image tag types (used as ELF note type field).
#define INITIUM_ITAG_IMAGE		0	// Basic image information (required).
#define INITIUM_ITAG_LOAD		1	// Memory layout options.
#define INITIUM_ITAG_OPTION	2	// Option description.
#define INITIUM_ITAG_MAPPING	3	// Virtual memory mapping description.
#define INITIUM_ITAG_VIDEO		4	// Requested video mode.

// Image tag containing basic image information.
typedef struct initium_itag_image
{
	uint32_t version;			// Initium version that the image is using.
	uint32_t flags;				// Flags for the image.
} initium_itag_image_t;

// Flags controlling optional features.
#define INITIUM_IMAGE_SECTIONS		(1<<0)	// Load ELF sections and pass a sections tag.
#define INITIUM_IMAGE_LOG			(1<<1)	// Enable the kernel log facility.

// Macro to declare an image itag.
#define INITIUM_IMAGE(flags) \
	__asm__( \
		"   .pushsection \".note.initium.image\", \"a\", " INITIUM_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 3f - 2f\n" \
		"   .long " XSTRINGIFY(INITIUM_ITAG_IMAGE) "\n" \
		"0: .asciz \"" INITIUM_NOTE_NAME "\"\n" \
		"1: .p2align 2\n" \
		"2: .long " XSTRINGIFY(INITIUM_VERSION) "\n" \
		"   .long " STRINGIFY(flags) "\n" \
		"   .p2align 2\n" \
		"3: .popsection\n")

// Image tag specifying loading parameters.
typedef struct initium_itag_load
{
	uint32_t flags;					// Flags controlling load behaviour.
	uint32_t _pad;
	initium_paddr_t alignment;			// Requested physical alignment of kernel image.
	initium_paddr_t min_alignment;		// Minimum physical alignment of kernel image.
	initium_vaddr_t virt_map_base;		// Base of virtual mapping range.
	initium_vaddr_t virt_map_size;		// Size of virtual mapping range.
} initium_itag_load_t;

// Flags controlling load behaviour.
#define INITIUM_LOAD_FIXED		(1<<0)	// Load at a fixed physical address.

// Macro to declare a load itag.
#define INITIUM_LOAD(flags, alignment, min_alignment, virt_map_base, virt_map_size) \
	__asm__( \
		"   .pushsection \".note.initium.load\", \"a\", " INITIUM_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 3f - 2f\n" \
		"   .long " XSTRINGIFY(INITIUM_ITAG_LOAD) "\n" \
		"0: .asciz \"" INITIUM_NOTE_NAME "\"\n" \
		"1: .p2align 2\n" \
		"2: .long " STRINGIFY(flags) "\n" \
		"   .long 0\n" \
		"   .quad " STRINGIFY(alignment) "\n" \
		"   .quad " STRINGIFY(min_alignment) "\n" \
		"   .quad " STRINGIFY(virt_map_base) "\n" \
		"   .quad " STRINGIFY(virt_map_size) "\n" \
		"   .p2align 2\n" \
		"3: .popsection\n")

// Image tag containing an option description.
typedef struct initium_itag_option
{
	uint8_t type;				// Type of the option.
	uint32_t name_len;			// Length of the option name.
	uint32_t desc_len;			// Length of the option description.
	uint32_t default_len;		// Length of the default value.
} initium_itag_option_t;

// Macro to declare a boolean option itag.
#define INITIUM_BOOLEAN_OPTION(name, desc, default) \
	__asm__( \
		"   .pushsection \".note.initium.option." name "\", \"a\", " INITIUM_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 6f - 2f\n" \
		"   .long " XSTRINGIFY(INITIUM_ITAG_OPTION) "\n" \
		"0: .asciz \"" INITIUM_NOTE_NAME "\"\n" \
		"1: .p2align 2\n" \
		"2: .byte " XSTRINGIFY(INITIUM_OPTION_BOOLEAN) "\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .long 4f - 3f\n" \
		"   .long 5f - 4f\n" \
		"   .long 1\n" \
		"3: .asciz \"" name "\"\n" \
		"4: .asciz \"" desc "\"\n" \
		"5: .byte " STRINGIFY(default) "\n" \
		"   .p2align 2\n" \
		"6: .popsection\n")

// Macro to declare an integer option itag.
#define INITIUM_INTEGER_OPTION(name, desc, default) \
	__asm__( \
		"   .pushsection \".note.initium.option." name "\", \"a\", " INITIUM_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 6f - 2f\n" \
		"   .long " XSTRINGIFY(INITIUM_ITAG_OPTION) "\n" \
		"0: .asciz \"" INITIUM_NOTE_NAME "\"\n" \
		"1: .p2align 2\n" \
		"2: .byte " XSTRINGIFY(INITIUM_OPTION_INTEGER) "\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .long 4f - 3f\n" \
		"   .long 5f - 4f\n" \
		"   .long 8\n" \
		"3: .asciz \"" name "\"\n" \
		"4: .asciz \"" desc "\"\n" \
		"5: .quad " STRINGIFY(default) "\n" \
		"   .p2align 2\n" \
		"6: .popsection\n")

// Macro to declare an string option itag.
#define INITIUM_STRING_OPTION(name, desc, default) \
	__asm__( \
		"   .pushsection \".note.initium.option." name "\", \"a\", " INITIUM_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 6f - 2f\n" \
		"   .long " XSTRINGIFY(INITIUM_ITAG_OPTION) "\n" \
		"0: .asciz \"" INITIUM_NOTE_NAME "\"\n" \
		"1: .p2align 2\n" \
		"2: .byte " XSTRINGIFY(INITIUM_OPTION_STRING) "\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .byte 0\n" \
		"   .long 4f - 3f\n" \
		"   .long 5f - 4f\n" \
		"   .long 6f - 5f\n" \
		"3: .asciz \"" name "\"\n" \
		"4: .asciz \"" desc "\"\n" \
		"5: .asciz \"" default "\"\n" \
		"   .p2align 2\n" \
		"6: .popsection\n")

// Image tag containing a virtual memory mapping description.
typedef struct initium_itag_mapping
{
	initium_vaddr_t virt;			// Virtual address to map.
	initium_paddr_t phys;			// Physical address to map to.
	initium_vaddr_t size;			// Size of mapping to make.
} initium_itag_mapping_t;

// Macro to declare a virtual memory mapping itag.
#define INITIUM_MAPPING(virt, phys, size) \
	__asm__( \
		"   .pushsection \".note.initium.mapping.b" # virt "\", \"a\", " INITIUM_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 3f - 2f\n" \
		"   .long " XSTRINGIFY(INITIUM_ITAG_MAPPING) "\n" \
		"0: .asciz \"" INITIUM_NOTE_NAME "\"\n" \
		"1: .p2align 2\n" \
		"2: .quad " STRINGIFY(virt) "\n" \
		"   .quad " STRINGIFY(phys) "\n" \
		"   .quad " STRINGIFY(size) "\n" \
		"   .p2align 2\n" \
		"3: .popsection\n")

// Image tag specifying the kernel's requested video mode.
typedef struct initium_itag_video
{
	uint32_t types;				// Supported video mode types.
	uint32_t width;				// Preferred LFB width.
	uint32_t height;			// Preferred LFB height.
	uint8_t bpp;				// Preferred LFB bits per pixel.
} initium_itag_video_t;

// Macro to declare a video mode itag.
#define INITIUM_VIDEO(types, width, height, bpp) \
	__asm__( \
		"   .pushsection \".note.initium.video\", \"a\", " INITIUM_SECTION_TYPE "\n" \
		"   .long 1f - 0f\n" \
		"   .long 3f - 2f\n" \
		"   .long " XSTRINGIFY(INITIUM_ITAG_VIDEO) "\n" \
		"0: .asciz \"" INITIUM_NOTE_NAME "\"\n" \
		"1: .p2align 2\n" \
		"2: .long " STRINGIFY(types) "\n" \
		"   .long " STRINGIFY(width) "\n" \
		"   .long " STRINGIFY(height) "\n" \
		"   .byte " STRINGIFY(bpp) "\n" \
		"   .p2align 2\n" \
		"3: .popsection\n")

#endif // __ASM__
#endif // __INITIUM_H
