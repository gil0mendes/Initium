Initium Boot Protocol
==================

License
-------

Copyright &copy; 2014 Gil Mendes

This document is distributed under the terms of the [Creative Commons Attribution-ShareAlike 2.0 Unported](http://creativecommons.org/licenses/by-sa/3.0/) license.

Introduction
------------

The purpose of this document is o specify the Initium Boot Protocol, a method
for loading an operating systm kernel and providing information to it.

The following sections describe the format of the information provided to the
boot loader in the kernel image, the format of the information that boot
loader provides to the kernel, and the machine state upon entering the kernel.

Basic Definitions
-----------------

This specification uses C syntax to define the Initium structures. Structures are
assumed to be implicitly padded by the compiler to archive manual aligment
for each field. However, in some cases padding has been explicitly added to
structures to ensure that 64-bit fields are aligned to an 8 bytes bondary
regardless of architecture (e.g. on 32-bit x86, 64-bit fields need only be
aligned to 4 bytes). Only the standard C fixed-width integer types are used, to
addictional type aliases are defined:

	typedef uint64_t initium_paddr_t;
	typedef uint64_t initium_vaddr_t;

These types are used to store physical and virtual address, respectively.
They are fixed to 64 bits in order to keep the format of all structures the
same across architectures.

Kernel Image
------------

A Initium kernel is an ELF32 or ELF64 image annotaded with ELF note sections to
specify to the boot loader how it wishes to be loaded, options that can be
edited by the user to control kernel behavior, among otehr things. These
sections are referred to as "image tags". All integer values contained within
the image tags are stored in the byte order specified by the ELF header.

ELF notes have the following format:

* _Name Size_: 32-bit integer. Size of the Name field, in bytes.
* _Desc Size_: 32-bit integer. Size of the Desc field, in bytes.
* _Type_: 32-bit integer. Type of the note.
* _Name_: String identifying the vendor who defined the format of the note.
	Padded to a 4 byte bondary.
* _Dec_: Data contained in the note. Padded to a 4 byte boundary.

For a Initium image tag, the Name field is set to `"INITIUM\0"`, the Type field is
set to the identifier of the image tag type, and the Desc field contains the
tag data.

The Initium header file included with the Initium boot loader defines macros for
creating all image tag types that expand to an inline ASM code chunk that
correctly defines the note section.

The following image tag types are defined (given as "Name (type ID)"):

### `INITIUM_ITAG_IMAGE`(`0`)

This image tag is the only tag that is required to be present in a kernel image.
All other tags are optional. It is used to identify the image as a Initium kernel.
There must only be one `INITIUM_ITAG_IMAGE` tag a kernel image.

	Typedef struct initium_itag_image
	{
		uint32_t version;
		uint32_t flags;
	} initium_itag_image_t;

Fields:

* `version`: Number defining the Initium protocol version that the kernel is
	using. The version number can be used by a boot loader to determine whether
	additions in later version of this specification are present. The current
	version number is 1.
* `flags`: Flags controlling whether certain optional features should be
	enabled. The following flags are currently defined:
	- `INITIUM_IMAGE_SECTIONS` (bit 0): Load additional ELF sections and pass
		section header table to the kernel (see `INITIUM_TAG_SECTIONS`).
	- `INITIUM_IMAGE_LOG` (bit 1): Enable the kernel log facility (see
		`INITIUM_TAG_LOG`).

### `INITIUM_ITAG_LOAD` (`1`)

This tag can be used for more control over the physical and virtual memory
layout set up by boot loader. there must be at most one `INITIUM_ITAG_LOAD`
tag in a kernel image.

	typedef struct load_itag_load
	{
		uint32_t		flags;
		uint32_t		_pad;
		initium_paddr_t 	aligment;
		initium_paddr_t 	min_aligment;
		initium_vaddr_t	virt_map_base;
		initium_vaddr_t	virt_map_size;
	} initium_itag_laod_t;

Fields:

* `flags`: Flags controlling load behaviour. The following flags are currently
	defined:
	- `LOAD_LOAD_FIXED` (bit 0): When this bit is set, the loader will load
		each segment in the ELF image at the exact physical address specified by
		the `p_paddr` field in the program headers. The `alignment` and
		`min_alignment` fields will be ignoed. When unset, the kernel image will
		be loaded at physical address allocated by the boot loader.
* `aligment`: Requested aligment of the kernel image in physical memory. If
	this is set to 0, the alignment will be chosen automatically by the boot
	loader. Otherwise, it must be a power of 2 greater that or equal to the
	machine's page size.
* `min_alignment`: Minimum physical alignment. If non-zero and less than the
	`alignement` value, if the loader is unable to allocate at `alignment` it
	will repeatedly try to allocate at unable to allocate at `alignment` it
	will repeatedly try alocate at decreasing powers of 2 to this value
	until it is able to make allocat at decresing powers of 2 down to this value
	minimum alignment.
* `virt_map_base`/`virt_map_size`: The boot loader allocates regions of the
	virtual address space to map certain things into, such as the tag list for
	the kernel and the video framebuffer. These fields specify the region of
	virtual address space that these mappings at any location in the virtual address
	space.

If this tag is not present in the kernel image, all fields will default to 0.

### `INITIUM_ITAG_OPTION` (`2`)

Multiple option tags can be included in a kernel image. Each describes an
option for the kernel that can be configured by the user via the boot loader.
The boot loader can use the information given by these tags, for example, to
display a configuration menu.

	typedef struct initium_itag_option
	{
		uint8_t		type;
		uint32_t	name_len;
		uint32_t	desc_len;
		uint32_t	default_len;
	} initium_itag_option_t;

This structure is followed by 3 variable length fields: `name`, `desc` and
`default`.

Fields:

* `type`: Type of the option. What is in the `default` field depends on the
   option type. The following option types are defined:
   - `INITIUM_OPTION_BOOLEAN` (0): Boolean, 1 byte, 0 or 1.
   - `INITIUM_OPTION_STRING` (1): String, variable length, null-terminated string
     (length includes null terminator).
   - `INITIUM_OPTION_INTEGER` (2): Integer, 8 bytes, 64-bit integer.
 * `name_len`: Length of the `name` field, including null terminator.
 * `desc_len`: Length of the `desc` field, including null terminator.
 * `default_len`: Length of the `default` field. Dependent on `type`, see above.
 * `name`: String name of the option. Must not contain spaces, or any of the
   following characters: `" '`. Names of the format `option_name` are
   recommended.
 * `desc`: Human readable description for the option, for use in configuration
   menus in the boot loader.
 * `default`: Default value of the option. Dependent on `type`, see above.

### `INITIUM_ITAG_MAPPING` (`3`)

This tag specifies a range of physical memory to map in to the kernel virtual
address space.

    typedef struct initium_itag_mapping
	{
     	initium_vaddr_t virt;
    	initium_paddr_t phys;
    	initium_vaddr_t size;
    } initium_itag_mapping_t;

Fields:

 * `virt`: The virtual address to map at. If this is set to the special value
   `0xffffffffffffffff`, then the boot loader will allocate an address to map
   at. The allocated address can be found in the kernel by iterating over the
   provided virtual address space map to find an entry with matching physical
   address and size. Otherwise, the exact address specified (must be aligned to
   the page size) is used.
 * `phys`: The physical address to map to (must be aligned to the page size).
 * `size`: The size of the range to map (must be a multiple of the page size).

For more information on the procedure used to build the virtual address space
for the kernel, see the _Kernel Environment_ section.

### `INITIUM_ITAG_VIDEO` (`4`)

This tag is used to specify to the boot loader what video types are supported
and what display mode it should attempt to set. There must be at most one
`INITIUM_ITAG_VIDEO` tag in a kernel image.

    typedef struct initium_itag_video
	{
    	uint32_t types;
    	uint32_t width;
    	uint32_t height;
    	uint8_t  bpp;
    } initium_itag_video_t;

Fields:

 * `types`: Bitfield of supported video types. The following video types are
   currently defined:
    - `INITIUM_VIDEO_VGA` (bit 0): VGA text mode.
    - `INITIUM_VIDEO_LFB` (bit 1): Linear framebuffer.
 * `width`/`height`/`bpp`: Preferred mode for `INITIUM_VIDEO_LFB`. If this mode
   is available it will be set, otherwise one as close as possible will be set.
   If all fields are 0, a mode will be chosen automatically.

What types are supported, the default video setup if this tag is not present,
and the precedence of types all depend on the platform. Furthermore, some
platforms may not support video mode setting at all. On such platforms the
kernel should use other means of output, such as a UART. See the _Platform
Specifics_ section for details of the video support on each platform.

Kernel Environment
------------------

The kernel entry point takes 2 arguments:

    void kmain(uint32_t magic, initium_tag_t *tags);

The first argument is the magic value, `0xb007cafe`, which can be used by the
kernel to ensure that it has been loaded by a Initium boot loader. The second
argument is a pointer to the first tag in the information tag list, described
in the _Kernel Information_ section.

The kernel is entered with the MMU enabled. A virtual address space is set up
by the boot loader which maps the kernel and certain other things, as well as
an architecture-dependent method to be able to manipulate the virtual address
space (see below). The following procedure is used to build the address space:

 * Map the kernel image.
 * Map ranges specified by `INITIUM_ITAG_MAPPING` tags.
 * Allocate space for other mappings (e.g. tag list, framebuffer, log buffer)
   as well as the stack. Address space for these mappings is allocated within
   the address range specified by the `INITIUM_ITAG_LOAD` tag. They are all
   allocated contiguously, however padding is permitted between mappings only
   as necessary to satisfy any alignment requirements.

A map of the virtual address space is provided in the tag list, see
`INITIUM_TAG_VMEM`. Note that the address space set up by the boot loader is
intended to only be temporary. The kernel should build its own address space
using the information provided by Initium as soon as possible. The physical
memory used by the page tables created by boot loader is marked as
`INITIUM_MEMORY_PAGETABLES` in the memory map, allowing it to be identified so
that it can be made free for use once the kernel has switched to its own page
tables.

On all architectures, the kernel will be entered with a valid stack set up,
mapped in the virtual address space. Details of the stack mapping are contained
in the `INITIUM_TAG_CORE` information tag. The physical memory used by the stack
is marked in the memory map as `INITIUM_MEMORY_STACK`; if the kernel switches away
from this stack as part of its initialization it should be sure to free the
physical memory used by it.

The following sections describe the environment upon entering the kernel for
each supported architecture.

### IA32

The arguments to the kernel entry point are passed on the stack. The machine
state upon entry to the kernel is as follows:

 * In 32-bit protected mode, paging enabled. A20 line is enabled.
 * CS is set to a flat 32-bit read/execute code segment. The exact value is
   undefined.
 * DS/ES/FS/GS/SS are set to a flat 32-bit read/write data segment. The exact
   values are undefined.
 * All bits in EFLAGS are clear except for bit 1, which must always be 1.
 * EBP is 0.
 * ESP points to a valid stack mapped in the virtual address space set up by
   the boot loader.
    - Initium magic value at offset 4.
    - Tag list pointer at offset 8.

Other machine state is not defined. In particular, the state of GDTR and IDTR
may not be valid, the kernel should set up its own GDT and IDT.

The boot loader may make use of large (4MB) pages within a single virtual
mapping when constructing the virtual address space, however separate mappings
should not be mapped together on a single large page. If large pages have been
used then the kernel will be entered with CR4.PSE set to 1, however this is not
guaranteed to be set and the kernel should explicitly enable it should it wish
to use large pages.

The address space set up by the boot loader will _not_ have the global flag set
on any mappings.

To allow the kernel to manipulate the virtual address space, the page directory
is recursively mapped. A 4MB (sized and aligned) region of address space is
allocated, and the page directory entry for that range is set to point to
itself. This region is allocated as high as possible _outside_ of the virtual
map range specified by the `INITIUM_ITAG_LOAD` tag and any mappings specified by
`INITIUM_ITAG_MAPPING` tags. The range is additionally not included in any
`INITIUM_TAG_VMEM` tags passed to the kernel. This region is kept separate like
this because it is intended to be temporary to assist the kernel in setting up
its own page tables, and should be hidden from architecture-independent kernel
code.

The format of the `INITIUM_TAG_PAGETABLES` tag for IA32 is as follows:

    typedef struct initium_tag_pagetables
	{
    	initium_tag_t 		header;

    	initium_paddr_t 	page_dir;
    	initium_vaddr_t 	mapping;
    } initium_tag_pagetables_t;

Fields:

 * `page_dir`: Physical address of the page directory.
 * `mapping`: Virtual address of the recursive page directory mapping.

### AMD64

The arguments to the kernel entry point are passed as per the AMD64 ABI. The
machine state upon entry to the kernel is as follows:

 * RDI contains the Initium magic value.
 * RSI contains the tag list pointer.
 * In 64-bit long mode, paging enabled, A20 line is enabled.
 * CS is set to a flat 64-bit read/execute code segment. The exact value is
   undefined.
 * DS/ES/FS/GS/SS are set to 0.
 * All bits in RFLAGS are clear except for bit 1, which must always be 1.
 * RBP is 0.
 * RSP points to a valid stack mapped in the virtual address space set up by
   the boot loader.

Other machine state is not defined. In particular, the state of GDTR and IDTR
may not be valid, the kernel should set up its own GDT and IDT.

The boot loader may make use of large (2MB) pages within a single virtual
mapping when constructing the virtual address space, however separate mappings
should not be mapped together on a single large page.

The address space set up by the boot loader will _not_ have the global flag set
on any mappings.

To allow the kernel to manipulate the virtual address space, the PML4 is
recursively mapped. A 512GB (sized and aligned) region of the virtual address
space is allocated, and the PML4 entry for that range is set to point to itself.
The same rules for allocation of this region as for IA32 apply here.

The format of the `INITIUM_TAG_PAGETABLES` tag for AMD64 is as follows:

    typedef struct initium_tag_pagetables
	{
    	initium_tag_t   header;

    	initium_paddr_t pml4;
    	initium_vaddr_t mapping;
    } initium_tag_pagetables_t;

Fields:

 * `pml4`: Physical address of the PML4.
 * `mapping`: Virtual address of the recursive PML4 mapping.

### ARM

The arguments to the kernel are passed as per the ARM Procedure Call Standard.
The machine state upon entry to the kernel is as follows:

 * R0 contains the Initium magic value.
 * R1 contains the tag list pointer.
 * Both IRQs and FIQs are disabled.
 * MMU is enabled (TTBR0 = address of first level table, TTBCR.N = 0).
 * Instruction and data caches are enabled.
 * R11 is 0.
 * R13 (SP) points to a valid stack mapped in the virtual address space set up
   by the boot loader.

Other machine state is not defined.

The boot loader may make use of sections (1MB page mappings) within a single
virtual mapping when constructing the virtual address space, however separate
mappings should not be mapped together on a single section.

Unlike IA32 and AMD64, on ARM it is not possible to recursively map the paging
structures as the first and second level tables have different formats.
Instead, a 1MB (sized and aligned) region of the virtual address space is
allocated, and the last page within it is mapped to the second level table that
covers that region. The kernel can then set up temporary mappings within this
region by modifying the page table. The same rules for allocation of this region
as for IA32 apply here.

The format of the `INITIUM_TAG_PAGETABLES` tag for ARM is as follows:

    typedef struct initium_tag_pagetables
	{
    	initium_tag_t   header;

    	initium_paddr_t l1;
    	initium_vaddr_t mapping;
    } initium_tag_pagetables_t;

Fields:

 * `l1`: Physical address of the first level translation table.
 * `mapping`: Virtual address of the 1MB temporary mapping region.

Kernel Information
------------------

Once the kernel has been loaded, the boot loader provides a set of information
to it describing the environment that it is running in, options that were set by
the user, modules that were loaded, etc. This information is provided as a list
of "information tags". The tag list is a contiguous list of structures, each
with an identical header identifying the type and size of that tag. The start
of the tag list is aligned to the page size, and the start of each tag is
aligned on an 8 byte boundary after the end of the previous tag. The tag list
is terminated with an `INITIUM_TAG_NONE` tag. The tag list is mapped in a virtual
memory range, and the physical memory containing it is marked in the memory map
as `INITIUM_MEMORY_RECLAIMABLE`. Multiple tags of the same type are guaranteed to
be grouped together in the tag list.

The header of each tag is in the following format:

    typedef struct initium_tag
	{
    	uint32_t type;
    	uint32_t size;
    } initium_tag_t;

Fields:

 * `type`: Type ID for this tag.
 * `size`: Total size of the tag data, including the header. The location of
   the next tag is given by `ROUND_UP(current_tag + current_tag->size, 8)`.

The following sections define all of the tag types that can appear in the tag
list (given as "Name (type ID)").

### `INITIUM_TAG_NONE` (`0`)

This tag signals the end of the tag list. It contains no extra data other than
the header.

### `INITIUM_TAG_CORE` (`1`)

This tag is always present in the tag list, and it is always the first tag in
the list.

    typedef struct initium_tag_core
	{
    	initium_tag_t   header;

    	initium_paddr_t tags_phys;
    	uint32_t      tags_size;
    	uint32_t      _pad;

    	initium_paddr_t kernel_phys;

    	initium_vaddr_t stack_base;
    	initium_paddr_t stack_phys;
    	uint32_t      stack_size;
    } initium_tag_core_t;

Fields:

 * `tags_phys`: Physical address of the tag list.
 * `tags_size`: Total size of the tag data in bytes, rounded up to an 8 byte
   boundary.
 * `kernel_phys`: The physical load address of the kernel image. This field is
   only valid when the `INITIUM_ITAG_LOAD` tag does not have `INITIUM_LOAD_FIXED`
   set.
 * `stack_base`: The virtual address of the base of the boot stack.
 * `stack_phys`: The physical address of the base of the boot stack.
 * `stack_size`: The size of the boot stack in bytes.

### `INITIUM_TAG_OPTION` (`2`)

This tag gives details of the value of a kernel option. For each option defined
by a `INITIUM_ITAG_OPTION` image tag, a `INITIUM_TAG_OPTION` will be present to
give the option value.

    typedef struct initium_tag_option
	{
    	initium_tag_t header;

    	uint8_t     type;
    	uint32_t    name_len;
    	uint32_t    value_len;
    } initium_tag_option_t;

This structure is followed by 2 variable length fields, `name` and `value`.

Fields:

 * `type`: Type of the option (see `INITIUM_ITAG_OPTION`).
 * `name_len`: Length of the name string, including null terminator.
 * `value_len`: Size of the option value, in bytes.
 * `name`: Name of the option. The start of this field is the end of the tag
   structure, aligned up to an 8 byte boundary.
 * `value`: Value of the option, dependent on the type (see `INITIUM_ITAG_OPTION`).
   The start of this field is the end of the name string, aligned up to an 8
   byte boundary.

### `INITIUM_TAG_MEMORY` (`3`)

The set of `INITIUM_TAG_MEMORY` tags in the tag list describe the physical memory
available for use by the kernel. This memory map is derived from a platform-
specific memory map and also details which ranges of memory contain the kernel,
modules, Initium data, etc. All ranges are page-aligned, and no ranges will
overlap. The minimum set of ranges possible are given, i.e. adjacent ranges of
the same type are coalesced into a single range. Note that this memory map does
_not_ describe platform-specific memory regions such as memory used for ACPI
data. It only contains information about usable RAM. Depending on the platform,
additional information may be included in the tag list about such regions. See
the _Platform Specifics_ section for more information. All memory tags are
sorted in the tag list by lowest start address first.

    typedef struct initium_tag_memory
	{
    	initium_tag_t   header;

    	initium_paddr_t start;
    	initium_paddr_t size;
    	uint8_t       type;
    } initium_tag_memory_t;

Fields:

 * `start`: Start address of the range. Aligned to the page size.
 * `size`: Size of the range. Multiple of the page size.
 * `type`: Type of the range. The following types are currently defined:
    - `INITIUM_MEMORY_FREE` (0): Free memory available for the kernel to use.
    - `INITIUM_MEMORY_ALLOCATED` (1): Allocated memory not available for use by
      the kernel. Such ranges contain the kernel image and the log buffer (if
      any).
    - `INITIUM_MEMORY_RECLAIMABLE` (2): Memory that currently contains Initium data
      (e.g. the tag list) but can be freed after kernel initialization is
      complete.
    - `INITIUM_MEMORY_PAGETABLES` (3): Memory containing the page tables set up
      by the boot loader.
    - `INITIUM_MEMORY_STACK` (4): Memory containing the stack set up by the boot
      loader.
    - `INITIUM_MEMORY_MODULES` (5): Memory containing module data.

### `INITIUM_TAG_VMEM` (`4`)

The set of `INITIUM_TAG_VMEM` tags describe all virtual memory mappings that
exist in the kernel virtual address space. All virtual memory tags are sorted
in the tag list by lowest start address first.

    typedef struct initium_tag_vmem
	{
    	initium_tag_t   header;

    	initium_vaddr_t start;
    	initium_vaddr_t size;
    	initium_paddr_t phys;
    } initium_tag_vmem_t;

Fields:

 * `start`: Start address of the virtual memory range. Aligned to the page size.
 * `size`: Size of the virtual memory range. Multiple of the page size.
 * `phys`: Start of the physical memory range that this range maps to.

### `INITIUM_TAG_PAGETABLES` (`5`)

This tag contains information required to allow the kernel to manipulate virtual
address mappings. The content of this tag is architecture-specific. For more
information see the _Kernel Environment_ section.

### `INITIUM_TAG_MODULE` (`6`)

Initium allows "modules" to be loaded and passed to the kernel. As far as Initium
and the boot loader is aware, a module is simply a regular file that is loaded
into memory and passed to the kernel. Interpretation of the loaded data is up
to the kernel. Possible uses are loading drivers necessary for the kernel to
be able to access the boot filesystem, or loading a file system image to run
the OS from. Each instance of this tag describes one of the modules that has
been loaded.

    typedef struct initium_tag_module
	{
    	initium_tag_t   header;

    	initium_paddr_t addr;
    	uint32_t      size;
    	uint32_t      name_len;
    } initium_tag_module_t;

This structure is followed by a variable length `name` field.

Fields:

 * `addr`: Physical address at which the module data is located, aligned to the
   page size. The data is not mapped into the virtual address space. The memory
   containing module data is marked as `INITIUM_MEMORY_MODULES`.
 * `size`: Size of the module data, in bytes.
 * `name_len`: Length of the name string, including null terminator.
 * `name`: Name of the module. This is the base name of the file that the module
   was loaded from.

### `INITIUM_TAG_VIDEO` (`7`)

This tag describes the current video mode. This tag may not always be present,
whether it is is platform dependent. For more details, see `INITIUM_ITAG_VIDEO`
and the _Platform Specifics_ section.

    typedef struct initium_colour
	{
    	uint8_t red;
    	uint8_t green;
    	uint8_t blue;
    } initium_colour_t;

    typedef struct initium_tag_video
	{
    	initium_tag_t header;

    	uint32_t     type;
    	uint32_t    _pad;

    	union {
    		struct {
    			uint8_t        cols;
    			uint8_t        lines;
    			uint8_t        x;
    			uint8_t        y;
    			uint32_t       _pad;
    			initium_paddr_t  mem_phys;
    			initium_vaddr_t  mem_virt;
    			uint32_t       mem_size;
    		} vga;

    		struct {
    			uint32_t       flags;
    			uint32_t       width;
    			uint32_t       height;
    			uint8_t        bpp;
    			uint32_t       pitch;
    			uint32_t       _pad;
    			initium_paddr_t  fb_phys;
    			initium_vaddr_t  fb_virt;
    			uint32_t       fb_size;
    			uint8_t        red_size;
    			uint8_t        red_pos;
    			uint8_t        green_size;
    			uint8_t        green_pos;
    			uint8_t        blue_size;
    			uint8_t        blue_pos;
    			uint16_t       palette_size;
    			initium_colour_t palette[0];
    		} lfb;
    	};
    } initium_tag_video_t;

Fields:

 * `type`: One of the available video mode type bits defined in the
   `INITIUM_ITAG_VIDEO` section will be set in this field to indicate the type
   of the video mode that has been set. The remainder of the structure
   content is dependent on the video mode type.

Fields (`INITIUM_VIDEO_VGA`):

 * `cols`: Number of columns on the text display (in characters).
 * `lines`: Number of lines on the text display (in characters).
 * `x`: Current X position of the cursor.
 * `y`: Current Y position of the cursor.
 * `mem_phys`: Physical address of VGA memory (0xb8000 on PC).
 * `mem_virt`: Virtual address of a mapping of the VGA memory.
 * `mem_size`: Size of the virtual mapping (multiple of page size). The size of
   the mapping will be sufficient to access the entire screen (i.e. at least
   `cols * rows * 2`).

Fields (`INITIUM_VIDEO_LFB`);

 * `flags`: Flags indicating properties of the framebuffer. The lower 8 bits
   contain the colour The following flags
   are currently defined:
    - `INITIUM_LFB_RGB` (bit 0): The framebuffer is in direct RGB colour format.
    - `INITIUM_LFB_INDEXED` (bit 1): The framebuffer is in indexed colour format.
   The `INITIUM_LFB_RGB` and `INITIUM_LFB_INDEXED` flags are mutually exclusive.
 * `width`: Width of the video mode, in pixels.
 * `height`: Height of the video mode, in pixels.
 * `bpp`: Bits per pixel.
 * `pitch`: Number of bytes per line of the framebuffer.
 * `fb_phys`: Physical address of the framebuffer.
 * `fb_virt`: Virtual address of a mapping of the framebuffer.
 * `fb_size`: Size of the virtual mapping (multiple of the page size). The size
   of the mapping will be sufficient to access the entire screen (i.e. at least
   `pitch * height`).
 * `red_size`/`green_size`/`blue_size`: For `INITIUM_LFB_RGB` modes, these fields
   give the size, in bits, of the red, green and blue components of each pixel.
 * `red_pos`/`green_pos`/`blue_pos`: For `INITIUM_LFB_RGB` modes, these fields
   give the bit position within each pixel of the least significant bits of the
   red, green and blue components.
 * `palette_size`: For `INITIUM_LFB_INDEXED`, the number of colours in the given
   palette.
 * `palette`: For `INITIUM_LFB_INDEXED`, a variable length colour palette that
   has been set up by the boot loader.

### `INITIUM_TAG_BOOTDEV` (`8`)

This tag describes the device that the system was booted from.

    typedef uint8_t initium_mac_addr_t[16];
    typedef uint8_t initium_ipv4_addr_t[4];
    typedef uint8_t initium_ipv6_addr_t[16];
    typedef union initium_ip_addr {
    	initium_ipv4_addr_t v4;
    	initium_ipv6_addr_t v6;
    } initium_ip_addr_t;

The above types are used to store network address information. A MAC address is
stored in the `initium_mac_addr_t` type, formatted according to the information
provided in the tag. IP addresses are stored in the `initium_ip_addr_t` union,
in network byte order.

    typedef struct initium_tag_bootdev
	{
    	initium_tag_t header;

    	uint32_t type;

    	union {
    		struct {
    			uint32_t         flags;
    			uint8_t          uuid[64];
    			uint8_t          device;
    			uint8_t          partition;
    			uint8_t          sub_partition;
    		} disk;

    		struct {
    			uint32_t         flags;
    			initium_ip_addr_t  server_ip;
    			uint16_t         server_port;
    			initium_ip_addr_t  gateway_ip;
    			initium_ip_addr_t  client_ip;
    			initium_mac_addr_t client_mac;
    			uint8_t          hw_type;
    			uint8_t          hw_addr_len;
    		} net;
    	};
    } initium_tag_bootdev_t;

Fields:

 * `type`: Specifies the type of the boot device. The following device types are
   currently defined:
    - `INITIUM_BOOTDEV_NONE` (0): No boot device. This is the case when booted
      from a boot image.
    - `INITIUM_BOOTDEV_DISK` (1): Booted from a disk device.
    - `INITIUM_BOOTDEV_NET` (2): Booted from the network.
	- `INITIUM_BOOTDEV_OTHER` (3): Booted from some other device, specified by a
	  string, the meaning of which is OS-defined.

   The remainder of the structure is dependent on the device type.

Fields (`INITIUM_BOOTDEV_DISK`):

 * `flags`: Behaviour flags. No flags are currently defined.
 * `uuid`: If not zero-length, the UUID of the boot filesystem. The UUID is
   given as a string. It is recommended that this is used if present to identify
   the boot FS, as it is the most reliable method. See the section
   _Filesystem UUIDs_ for more information about this field.
 * `device`: Boot device number. This field is platform-specific. On the PC
   platform it contains the BIOS device number. It is currently undefined for
   other platforms.
 * `partition`: Partition number on the boot device. Partitions are numebered
   starting from 0. Logical partitions in DOS extended partitions start from 4.
   If not booted from a partition, will be set to `0xff`.
 * `sub_partition`: Sub-partition number within the partition, e.g. for BSD
   disklabels. If not booted from a sub-partition, will be set to `0xff`.

Fields (`INITIUM_BOOTDEV_NET`):

 * `flags`: Behaviour flags. The following flags are currently defined:
    - `INITIUM_NET_IPV6` (bit 0): The given IP addresses are IPv6 addresses,
      rather than IPv4 addresses.
 * `server_ip`: IP address of the server that was booted from.
 * `server_port`: UDP port number of the TFTP server.
 * `gateway_ip`: Gateway IP address.
 * `client_ip`: IP address that was used on this machine when communicating
   with the server.
 * `client_mac`: MAC address of the boot network interface.
 * `hw_type`: Type of the network interface, as specified by the _Hardware Type_
   section of RFC 1700.
 * `hw_addr_len`: Length of the client MAC address.

Fields (`INITIUM_BOOTDEV_OTHER`):

 * `str_len`: Length of the device specifier string following the structure,
   including null terminator.

### `INITIUM_TAG_LOG` (`9`)

This tag is part of an optional feature that enables the boot loader to display
a log from the kernel in the event of a crash. As long as the memory that
contains the log buffer is not cleared when the machine is reset, it can be
recovered by the boot loader. Whether this is supported is platform-dependent
(currently PC only). It is a rather rudimentary system, however it is useful to
assist in debugging, particularly on real hardware if no other mechanism is
available to obtain output from a kernel before a crash. A kernel indicates
that it wishes to use this feature by setting the `INITIUM_IMAGE_LOG` flag in the
`INITIUM_ITAG_IMAGE` tag. If the boot loader supports this feature, this tag type
will be included in the tag list.

    typedef struct initium_tag_log
	{
    	initium_tag_t   header;

    	initium_vaddr_t log_virt;
    	initium_paddr_t log_phys;
    	uint32_t      log_size;
    	uint32_t      _pad;

    	initium_paddr_t prev_phys;
    	uint32_t      prev_size;
    } initium_tag_log_t;

Fields:

 * `log_virt`: Virtual address of the log buffer (aligned to page size).
 * `log_phys`: Physical address of the log buffer.
 * `log_size`: Total size of the log buffer (multiple of page size).
 * `prev_phys`: Physical address of the log buffer from the previous session.
 * `prev_size`: Total size of previous session log buffer. If 0, no previous
   log buffer was available.

The main log buffer will be contained in physical memory marked as
`INITIUM_MEMORY_ALLOCATED`, while the previous log buffer, if any, will be
contained in physical memory marked as `INITIUM_MEMORY_RECLAIMABLE`.

The log buffer is a circular buffer. It includes a header at the start, any
remaining space contains the log itself. The header has the following format:

    typedef struct initium_log
	{
    	uint32_t magic;

    	uint32_t start;
    	uint32_t length;

    	uint32_t info[3];
    	uint8_t  buffer[0];
    } initium_log_t;

Fields:

 * `magic`: This field is used by the boot loader to identify a log buffer. Its
   exact value is not specified, it should not be modified by the kernel.
 * `start`: Offset in the buffer of the start of the log. A new log buffer will
   have this field initialized to 0. Must not exceed
   `(tag->log_size - sizeof(initium_log_t))`.
 * `length`: Number of characters in the log buffer. A new log buffer will have
   this field initialized to 0. Must not exceed
   `(tag->log_size - sizeof(initium_log_t))`.
 * `info`: These fields are free for use by the kernel. A new log buffer will
   have them initialized to 0, the previous session log buffer will contain
   the values written by the previous kernel.

The kernel should modify the `start` and `length` fields as it writes into the
buffer. An algorithm for writing to the buffer is shown below:

    log_size = tag->log_size - sizeof(initium_log_t);
    log->buffer[(log->start + log->length) % log_size] = ch;
    if(log->length < log_size)
	{
    	log->length++;
    } else
	{
    	log->start = (log->start + 1) % log_size;
    }

A point to bear in mind is that when the CPU is reset, the CPU cache may not be
written back to memory. Therefore, should you wish to ensure that a set of data
written to the log can be seen by the boot loader, invalidate the CPU cache
after writing.

### `INITIUM_TAG_SECTIONS` (`10`)

If the `INITIUM_IMAGE_SECTIONS` flag is specified in the `INITIUM_ITAG_IMAGE` image
tag, additional loadable sections not normally loaded (`SHT_PROGBITS`,
`SHT_NOBITS`, `SHT_SYMTAB` and `SHT_STRTAB` sections without the `SHF_ALLOC`
flag set, e.g. the symbol table and debug information) will be loaded, and this
tag will be passed containing the section header table. The additional sections
will be loaded in physical memory marked as `INITIUM_MEMORY_ALLOCATED`, and each
will have its header modified to contain the allocated physical address in the
`sh_addr` field.

    typedef struct initium_tag_sections
	{
    	initium_tag_t header;

    	uint32_t    num;
    	uint32_t    entsize;
    	uint32_t    shstrndx;

    	uint32_t    _pad;

    	uint8_t     sections[0];
    } initium_tag_sections_t;

Fields:

 * `num`/`entsize`/`shstrndx`: These fields correspond to the fields with the
   same name in the ELF executable header.
 * `sections`: Array of section headers, each `entsize` bytes long.

Platform Specifics
------------------

### PC

If a video mode image tag is not included, VGA text mode will be used. If both
`INITIUM_VIDEO_LFB` and `INITIUM_VIDEO_VGA` are set, the boot loader will attempt
to set a framebuffer mode and fall back on VGA. It is recommended that a
kernel supports VGA text mode rather than exclusively using an LFB mode, as it
may not always be possible to set a LFB mode.

There are some information tags that are specific to the PC platform. These are
detailed below.

#### `INITIUM_TAG_E820` (`11`)

The PC BIOS provides more memory information than is included in the Initium
physical memory map. Some of this information is required to support ACPI,
therefore Initium will provide an unmodified copy of the BIOS E820 memory map
to the kernel. The tag list will contain an E820 tag for each address range
descriptor returned by INT 15h function E820h. All E820 tags will be placed in
the tag list in the order in which they were returned by the BIOS. Each tag's
data will contain the tag header and then the data returned by the BIOS. The
format of each descriptor as of ACPI 5.0 is as follows:

    typedef struct initium_tag_e820
	{
    	initium_tag_t header;

    	uint64_t    start;
    	uint64_t    length;
    	uint32_t    type;
    	uint32_t    attr;
    } initium_tag_e820_t;

The `start`, `length` and `type` fields are guaranteed to be present, however
any fields following them may not be present. The kernel should check the tag
size before accessing them.

Filesystem UUIDs
----------------

Some filesystems, such as ext2/3/4, have their own UUID. For these, the UUID
field of the `INITIUM_TAG_BOOTDEV` tag contains a string representation of the
filesystem's UUID. There are others that do not, however for some it is
possible to give a reasonably unique identifier based on other information.
This section describes what a boot loader should give as a UUID for certain
filesystems. These should follow the behaviour of the `libblkid` library.

### ISO9660

If the filesystem has a modification date set, that is used, else the creation
date is used, and formatted into the following:

    yyyy-mm-dd-hh-mm-ss-cc
