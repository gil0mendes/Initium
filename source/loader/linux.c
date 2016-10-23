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

#include <lib/string.h>

#include <loader/linux.h>

#include <ui.h>
#include <memory.h>
#include <loader.h>

// video mode types to support (will only get VGA if platform supports.)
#define LINUX_VIDEO_TYPES (VIDEO_MODE_VGA | VIDEO_MODE_LFB)

static __noreturn void linux_loader_load(void *_loader)
{
	linux_loader_t *loader = _loader;
	size_t size;

	// combine the path string and arguments back into a single string.
	size = strlen("BOOT_IMAGE=") + strlen(loader->path) + strlen(loader->args.string) + 2;
	loader->cmdline = malloc(size);
	strcpy(loader->cmdline, "BOOT_IMAGE=");
	strcat(loader->cmdline, loader->path);
	strcat(loader->cmdline, " ");
	strcat(loader->cmdline, loader->args.string);

	// architecture code does all the work
	linux_arch_load(loader);
}

#ifdef CONFIG_TARGET_HAS_UI

static ui_window_t *linux_loader_configure(void *_loader, const char *title)
{
	linux_loader_t *loader = _loader;
	ui_window_t *window;
	ui_entry_t *entry;

	window = ui_list_create(title, true);
	entry = ui_entry_create("Command line", &loader->args);
	ui_list_insert(window, entry, false);

 #ifdef CONFIG_TARGET_HAS_VIDEO
	entry = video_env_chooser(current_environ, "video_mode", LINUX_VIDEO_TYPES);
	ui_list_insert(window, entry, false);
 #endif // CONFIG_TARGET_HAS_VIDEO

	return window;
}

#endif // CONFIG_TARGET_HAS_UI

/** Linux loader operations. */
static loader_ops_t linux_loader_ops = {
	.load		= linux_loader_load,
    #ifdef CONFIG_TARGET_HAS_UI
	.configure	= linux_loader_configure,
    #endif
};

/** Load Linux kernel initrd data.
 * @param loader        Loader internal data.
 * @param addr          Allocated address to load to. */
void linux_initrd_load(linux_loader_t *loader, void *addr)
{
	list_foreach(&loader->initrds, iter) {
		linux_initrd_t *initrd = list_entry(iter, linux_initrd_t, header);
		status_t ret;

		ret = fs_read(initrd->handle, addr, initrd->handle->size, 0);
		if (ret != STATUS_SUCCESS)
			boot_error("Error loading initrd: %pS", ret);

		addr += initrd->handle->size;
	}
}

/** Add an initrd file.
 * @param loader        Linux loader internal data.
 * @param path          Path to initrd.
 * @return              Whether successful. */
static bool add_initrd(linux_loader_t *loader, const char *path)
{
	linux_initrd_t *initrd;
	status_t ret;

	initrd = malloc(sizeof(*initrd));
	list_init(&initrd->header);

	ret = fs_open(path, NULL, FILE_TYPE_REGULAR, &initrd->handle);
	if (ret != STATUS_SUCCESS) {
		config_error("Error opening '%s': %pS", path, ret);
		free(initrd);
		return false;
	}

	loader->initrd_size += initrd->handle->size;
	list_append(&loader->initrds, &initrd->header);
	return true;
}

/** Load a Linux kernel.
 * @param args          Argument list.
 * @return              Whether successful. */
static bool config_cmd_linux(value_list_t *args)
{
	linux_loader_t *loader;
	status_t ret;

	if (args->count < 1 || args->count > 2 || args->values[0].type != VALUE_TYPE_STRING) {
		config_error("Invalid arguments");
		return false;
	}

	loader = malloc(sizeof(*loader));
	list_init(&loader->initrds);
	loader->initrd_size = 0;

	loader->args.type = VALUE_TYPE_STRING;
	split_cmdline(args->values[0].string, &loader->path, &loader->args.string);

	ret = fs_open(loader->path, NULL, FILE_TYPE_REGULAR, &loader->kernel);
	if (ret != STATUS_SUCCESS) {
		config_error("Error opening '%s': %pS", loader->path, ret);
		goto err_free;
	}

	if (args->count == 2) {
		if (args->values[1].type == VALUE_TYPE_STRING) {
			if (!add_initrd(loader, args->values[1].string))
				goto err_close;
		} else if (args->values[1].type == VALUE_TYPE_LIST) {
			value_list_t *list = args->values[1].list;

			for (size_t i = 0; i < list->count; i++) {
				if (list->values[i].type != VALUE_TYPE_STRING) {
					config_error("Invalid arguments");
					goto err_initrd;
				} else if (!add_initrd(loader, list->values[i].string))
					goto err_initrd;
			}
		} else {
			config_error("Invalid arguments");
			goto err_close;
		}
	}

	// check whther the kernel image is valid
	if (!linux_arch_check(loader)) {
		goto err_initrd;
	}

  #ifdef CONFIG_TARGET_HAS_VIDEO
	video_env_init(current_environ, "video_mode", LINUX_VIDEO_TYPES, NULL);
  #endif

	environ_set_loader(current_environ, &linux_loader_ops, loader);
	return true;

err_initrd:
	while (!list_empty(&loader->initrds)) {
		linux_initrd_t *initrd = list_first(&loader->initrds, linux_initrd_t, header);

		list_remove(&initrd->header);
		fs_close(initrd->handle);
		free(initrd);
	}

err_close:
	fs_close(loader->kernel);

err_free:
	value_destroy(&loader->args);
	free(loader->path);
	free(loader);
	return false;
}

BUILTIN_COMMAND("linux", "Load a Linux kernel", config_cmd_linux);
