/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Gil Mendes
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
 * @brief               EFI executable loader.
 */

#include <efi/console.h>
#include <efi/disk.h>
#include <efi/efi.h>
#include <efi/memory.h>
#include <efi/video.h>

#include <lib/string.h>

#include <assert.h>
#include <config.h>
#include <fs.h>
#include <loader.h>
#include <memory.h>
#include <ui.h>

/** EFI loader data. */
typedef struct efi_loader {
    fs_handle_t *handle;                 /**< Handle to EFI image. */
    value_t cmdline;                     /**< Command line to the image. */
    efi_device_path_t *path;             /**< EFI file path. */
} efi_loader_t;

/** Load an EFI executable.
 * @param _loader       Pointer to loader data. */
static __noreturn void efi_loader_load(void *_loader) {
    efi_loader_t *loader = _loader;
    phys_size_t buf_size;
    void *buf;
    efi_handle_t image_handle;
    efi_loaded_image_t *image;
    device_t *device;
    efi_char16_t *str;
    efi_uintn_t str_size;
    efi_status_t status;
    status_t ret;

    /* Allocate a buffer to read the image into. */
    buf_size = round_up(loader->handle->size, PAGE_SIZE);
    buf = memory_alloc(buf_size, 0, 0, 0, MEMORY_TYPE_INTERNAL, 0, NULL);

    /* Read it in. */
    ret = fs_read(loader->handle, buf, loader->handle->size, 0);
    if (ret != STATUS_SUCCESS)
	boot_error("Failed to read EFI image (%d)", ret);

    /* Ask the firmware to load the image. */
    status = efi_call(
        efi_boot_services->load_image,
        false, efi_image_handle, NULL, buf, loader->handle->size, &image_handle);
    if (status != EFI_SUCCESS)
	boot_error("Failed to load EFI image (0x%zx)", status);

    /* Get the loaded image protocol. */
    status = efi_get_loaded_image(image_handle, &image);
    if (status != EFI_SUCCESS)
	boot_error("Failed to get loaded image protocol (0x%zx)", status);

    /* Try to identify the handle of the device the image was on. */
    device = loader->handle->mount->device;
    image->device_handle = (device->type == DEVICE_TYPE_DISK)
                           ? efi_disk_get_handle((disk_device_t *)device)
			   : NULL;
    image->file_path = loader->path;

    fs_close(loader->handle);

    /* Convert the arguments to UTF-16. FIXME: UTF-8 internally? */
    str_size = (strlen(loader->cmdline.string) + 1) * sizeof(*str);
    str = malloc(str_size);
    for (size_t i = 0; i < str_size / sizeof(*str); i++)
	str[i] = loader->cmdline.string[i];

    image->load_options = str;
    image->load_options_size = str_size;

    /* Reset everything to default state. */
    efi_video_reset();
    efi_console_reset();
    efi_memory_cleanup();

    /* Start the image. */
    status = efi_call(efi_boot_services->start_image, image_handle, &str_size, &str);
    if (status != EFI_SUCCESS)
	dprintf("efi: loaded image returned status 0x%zx\n", status);

    /* We can't do anything here - the loaded image may have done things making
     * our internal state invalid. Just pass through the error to whatever
     * loaded us. */
    efi_exit(status, str, str_size);
}

/** Get a configuration window.
 * @param _loader       Pointer to loader data.
 * @param title         Title to give the window.
 * @return              Configuration window. */
static ui_window_t *efi_loader_configure(void *_loader, const char *title) {
    efi_loader_t *loader = _loader;
    ui_window_t *window;
    ui_entry_t *entry;

    window = ui_list_create(title, true);
    entry = ui_entry_create("Command line", &loader->cmdline);
    ui_list_insert(window, entry, false);
    return window;
}

/** EFI loader operations. */
static loader_ops_t efi_loader_ops = {
    .load = efi_loader_load,
    .configure = efi_loader_configure,
};

/** Create an EFI file path for a file path.
 * @param handle        Handle to file.
 * @param path          Path to file.
 * @return              Pointer to generated EFI path, NULL on failure. */
static efi_device_path_t *convert_file_path(fs_handle_t *handle, const char *path) {
    efi_device_path_file_t *efi_path;
    efi_device_path_t *end;
    size_t len;

    /* We need to generate an EFI path from the file path. Since we can't get
     * the full path from a relative path, only allow absolute ones (or relative
     * ones from the root). */
    if (*path == '(') {
	    while (*path && *path++ != ')')
		;

	    /* This should be true, fs_open() succeeded. */
	    assert(*path == '/');
	} else if (*path != '/') {
	    if (current_environ->directory && current_environ->directory != handle->mount->root) {
		    config_error("File path must be absolute or relative to root");
		    return NULL;
		}
	}

    while (*path == '/')
	path++;

    /* This allocation may overestimate, due to duplicate '/'s, but we will put
     * the correct size in the structure. Add 2 to length for leading '\' and
     * NULL terminator. */
    efi_path = malloc(sizeof(efi_device_path_file_t)
                      + ((strlen(path) + 2) * sizeof(efi_char16_t))
                      + sizeof(efi_device_path_t));
    efi_path->header.type = EFI_DEVICE_PATH_TYPE_MEDIA;
    efi_path->header.subtype = EFI_DEVICE_PATH_MEDIA_SUBTYPE_FILE;

    /* Always need a leading '\'. */
    len = 0;
    efi_path->path[len++] = '\\';

    /* Convert the path. FIXME: UTF-8 conversion. */
    while (*path) {
	    if (*path == '/') {
		    efi_path->path[len++] = '\\';
		    while (*path == '/')
			path++;
		} else {
		    efi_path->path[len++] = *path++;
		}
	}

    /* Add the null terminator. */
    efi_path->path[len++] = 0;

    efi_path->header.length = sizeof(efi_device_path_file_t) + (len * sizeof(efi_char16_t));

    /* Add a terminator entry. */
    end = (efi_device_path_t *)((char *)efi_path + efi_path->header.length);
    end->type = EFI_DEVICE_PATH_TYPE_END;
    end->subtype = EFI_DEVICE_PATH_END_SUBTYPE_WHOLE;
    return &efi_path->header;
}

/** Load an EFI executable.
 * @param args          Argument list.
 * @return              Whether successful. */
static bool config_cmd_efi(value_list_t *args) {
    efi_loader_t *loader;
    const char *path;
    status_t ret;

    if ((args->count != 1 && args->count != 2)
        || args->values[0].type != VALUE_TYPE_STRING
        || (args->count == 2 && args->values[1].type != VALUE_TYPE_STRING))
	{
	    config_error("Invalid arguments");
	    return false;
	}

    loader = malloc(sizeof(*loader));

    path = args->values[0].string;
    ret = fs_open(path, NULL, &loader->handle);
    if (ret != STATUS_SUCCESS) {
	    config_error("Error %d opening '%s'", ret, path);
	    goto err_free;
	} else if (loader->handle->directory) {
	    config_error("'%s' is a directory", path);
	    goto err_close;
	}

    loader->path = convert_file_path(loader->handle, path);
    if (!loader->path)
	goto err_close;

    /* Copy the command line. */
    if (args->count == 2) {
	    value_move(&args->values[1], &loader->cmdline);
	} else {
	    value_init(&loader->cmdline, VALUE_TYPE_STRING);
	}

    environ_set_loader(current_environ, &efi_loader_ops, loader);
    return true;

err_close:
    fs_close(loader->handle);
err_free:
    free(loader);
    return false;
}

BUILTIN_COMMAND("efi", config_cmd_efi);
