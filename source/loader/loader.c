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
 * @brief 			Loader entry point
 */

#include <fs.h>
#include <menu.h>
#include <config.h>
#include <loader.h>
#include <screen.h>
#include <assert.h>
#include <memory.h>
#include <console.h>

// Maximum number of pre-boot hooks
#define PREBOOT_HOOKS_MAX 8

// Array of pre-boot hooks
static preboot_hook_t preboot_hooks[PREBOOT_HOOKS_MAX];
static size_t preboot_hooks_count = 0;

/**
 * Add a pre-boot hook
 *
 * @param hook 		Hook to add
 */
void loader_register_preboot_hook(preboot_hook_t hook) {
	assert(preboot_hooks_count < PREBOOT_HOOKS_MAX);
	preboot_hooks[preboot_hooks_count++] = hook;
}

/**
 * Perform pre-boot tasks
 */
void load_preboot(void) {
	size_t i;

	for (i = 0; i < preboot_hooks_count; i++) {
		preboot_hooks[i]();
	}
}

/**
 * Main function for the Initium bootloader
 */
void loader_main(void) {
	// we must have a filesystem to boot from
	/*if(!boot_device || !boot_device->fs) {
		boot_error("Could not find boot fylesystem");
	}*/

	// Load the configuration file
	//config_init();

	#ifdef CONFIG_GUI_MODE
		// Setup screen
		setupScreen();
	#else
		// Show menu
		//current_environ = menuDisplay();
	#endif

	while(true);
}
