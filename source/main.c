/**
* The MIT License (MIT)
*
* Copyright (c) 2014-2016 Gil Mendes <gil00mendes@gmail.com>
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
 * @brief               Loader main function.
 */

#include <menu.h>
#include <shell.h>
#include <assert.h>
#include <config.h>
#include <device.h>
#include <loader.h>
#include <memory.h>

/** Maximum number of pre-boot hooks */
#define PREBOOT_HOOKS_MAX   8

/** Array of pre-boot hooks */
static preboot_hook_t preboot_hooks[PREBOOT_HOOKS_MAX];
static size_t preboot_hook_count = 0;

/**
 * Add a pre-boot hook.
 *
 * @param hook Hook to add.
 */
void loader_register_preboot_hook(preboot_hook_t hook) {
  assert(preboot_hook_count < PREBOOT_HOOKS_MAX);
  preboot_hooks[preboot_hook_count++] = hook;
}

/**
 * Perform pre-boot tasks.
 */
void loader_preboot(void) {
  for (size_t i = 0; i < preboot_hook_count; i++) { preboot_hooks[i](); }
}

/**
 * Main function of the loader.
 */
void loader_main(void) {
  environ_t *env;

  // initialize config
  config_init();

  // initialize memory
  memory_init();

  // initialize devices
  device_init();

  // load the configuration file
  config_load();

  // display the menu
  env = menu_display();

  // and finnaly boot the OS
  if (env->loader) {
    environ_boot(env);
  } else {
    boot_error("No operating system to boot");
  }
}
