/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2015 Gil Mendes
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
 * @brief		Core definitions.
 */

#ifndef __LOADER_H
#define __LOADER_H

#include <arch/loader.h>

#include <platform/loader.h>

#include <console.h>
#include <status.h>
#include <types.h>

struct ui_window;

// Structure defining an OS loader type
typedef struct loader_type {
	/**
	 * Load the operating system.
	 * @note		Should not return.
	 */
	void (*load)(void) __noreturn;

	#if CONFIG_UI
	/**
	 * Return a window for configuring the OS.
	 *
	 * @return		Pointer to configuration window.
	 */
	struct ui_window *(*configure)(void);
	#endif
} loader_type_t;

/** Builtin object definition structure. */
typedef struct builtin {
	/** Type of the builtin */
	enum {
		BUILTIN_TYPE_PARTITION,
		BUILTIN_TYPE_FS,
		BUILTIN_TYPE_COMMAND,
	} type;

	/** Pointer to object */
	void *object;
} builtin_t;

// Type of a hook function to call before booting an OS
typedef void (*preboot_hook_t)(void);

extern char __start[], __end[];
extern builtin_t __builtins_start[], __builtins_end[];

// Define a builtin object
#define DEFINE_BUILTIN(type, object) \
	static builtin_t __builtin_##object __section(".builtins") __used = { \
		type, \
		&object \
	}

// Iterate over builtin objects
#define builtin_foreach(builtin_type, object_type, var) \
	int __iter_##var = 0; \
	for (object_type *var = (object_type *)__builtins_start[0].object; \
	    __iter_##var < (__builtins_end - __builtins_start); \
	    var = (object_type *)__builtins_start[++__iter_##var].object) \
		if (__builtins_start[__iter_##var].type == builtin_type)

/**
 * Offset to apply to a physical address to get a virtual address.
 *
 * To handle platforms where the loader runs from the virtual address space
 * and physical memory is not identity mapped, this value is added on to any
 * physical address used to obtain a virtual address that maps it. If it is
 * not specified by the architecture, it is assumed that physical addresses
 * can be used directly without modification.
 */
#ifndef TARGET_VIRT_OFFSET
# define TARGET_VIRT_OFFSET    0
#endif

/**
 * Highest physical address accessible to the loader.
 *
 * Specifies the highest physical address which the loader can access. If this
 * is not specified by the architecture, it is assumed that the loader can
 * access the low 4GB of the physical address space.
 */
#ifndef TARGET_PHYS_MAX
# define TARGET_PHYS_MAX       0xffffffff
#endif

/**
 * Convert a virtual address to a physical address.
 *
 * @param addr         Address to convert.
 * @return             Converted physical address.
 */
static inline phys_ptr_t virt_to_phys(ptr_t addr) {
       return (addr - TARGET_VIRT_OFFSET);
}

/**
 * Convert a physical address to a virtual address.
 *
 * @param addr         Address to convert.
 * @return             Converted virtual address.
 */
static inline ptr_t phys_to_virt(phys_ptr_t addr) {
       return (addr + TARGET_VIRT_OFFSET);
}

/** Operating modes for a loaded OS. */
typedef enum load_mode {
    LOAD_MODE_32BIT,                    /**< 32-bit. */
    LOAD_MODE_64BIT,                    /**< 64-bit. */
} load_mode_t;

/** Structure defining operations for an OS loader. */
typedef struct loader_ops {
    /** Load the operating system.
     * @param private       Loader private data. */
    void (*load)(void *private) __noreturn;

	#ifdef CONFIG_TARGET_HAS_UI
	/**
	 * Get a configuration window for the OS.
	 *
	 * @param private		Loader private data.
	 * @param title			Title to give the window.
	 * @return              Window for configuring the OS.
	 */
	struct ui_window *(*configure)(void *private, const char *title);
	#endif
} loader_ops_t;

extern void target_reboot(void) __noreturn;

#ifdef TARGET_HAS_EXIT
extern void target_exit(void) __noreturn;
#else
static inline __noreturn void target_exit(void) { target_reboot(); }
#endif

extern void boot_error(const char *fmt, ...) __printf(1, 2) __noreturn;
extern void internal_error(const char *fmt, ...) __printf(1, 2) __noreturn;

extern void backtrace(int (*print)(const char *fmt, ...));

extern void loader_register_preboot_hook(preboot_hook_t hook);
extern void loader_preboot(void);

extern void loader_main(void);

#endif /* __LOADER_H */
