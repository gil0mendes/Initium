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
 * @brief   Linux kernel loader.
 */

 #ifndef __LOADER_LINUX_H
 #define __LOADER_LINUX_H

 #include <config.h>
 #include <fs.h>
 #include <video.h>

/** Linux loader internal data. */
typedef struct linux_loader {
	fs_handle_t *kernel;                    /**< Kernel image handle. */
	list_t initrds;                         /**< Initrd file list. */
	offset_t initrd_size;                   /**< Combined initrd size. */
	char *cmdline;                          /**< Kernel command line (path + arguments). */
	char *path;                             /**< Separated path string. */
	value_t args;                           /**< Value for editing kernel arguments. */
	video_mode_t *video;                    /**< Video mode set by linux_video_set(). */
} linux_loader_t;

/** Linux initrd structure. */
typedef struct linux_initrd {
	list_t header;                          /**< Link to initrd list. */
	fs_handle_t *handle;                    /**< Handle to initrd. */
} linux_initrd_t;

extern bool linux_arch_check(linux_loader_t *loader);
extern void linux_arch_load(linux_loader_t *loader) __noreturn;

extern void linux_initrd_load(linux_loader_t *loader, void *addr);

 #ifdef CONFIG_TARGET_HAS_VIDEO

/** Set the video mode for a Linux kernel.
 * @param loader            Loader internal data. */
static inline void linux_video_set(linux_loader_t *loader)
{
	loader->video = video_env_set(current_environ, "video_mode");
}

 #endif /* CONFIG_TARGET_HAS_VIDEO */

 #endif /* __LOADER_LINUX_H */
