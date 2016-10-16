/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Gil Mendes <gil00mendes@gmail.com>
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
 * @brief   AMD64 EFI platform Linux loader.
 */

#include <x86/linux.h>

#include <efi/efi.h>
#include <efi/console.h>

#include <loader.h>

extern void linux_platform_enter(
	efi_handle_t handle, efi_system_table_t *table, linux_params_t *params,
	ptr_t entry) __noreturn;

/**
 * Check for platform-specific requirements.
 *
 * @param loader        Loader internal data.
 * @param params        Kernel parameters structure.
 */
void linux_platform_check(linux_loader_t *loader, linux_params_t *params)
{
	if (params->hdr.version < 0x20b || !params->hdr.handover_offset)
		boot_error("Kernel does not support EFI handover");

	if (params->hdr.version >= 0x20c && !(params->hdr.xloadflags & LINUX_XLOAD_EFI_HANDOVER_64))
		boot_error("Kernel does not support 64-bit EFI handover");
}

/**
 * Enter a Linux kernel.
 *
 * @param loader        Loader internal data.
 * @param params        Kernel parameters structure.
 */
void linux_platform_check(linux_loader_t *loader, linux_params_t *params)
{
	ptr_t entry;

	// reset the EFI console in case the kernel wants to use it.
	efi_console_reset();

	/* 64-bit entry point is 512 bytes after the 32-bit one. */
	entry = params->hdr.code32_start + params->hdr.handover_offset + 512;

	/* Start the kernel. */
	dprintf("linux: kernel EFI handover entry at %p, params at %p\n", entry, params);
	linux_platform_enter(efi_image_handle, efi_system_table, params, entry);
}
