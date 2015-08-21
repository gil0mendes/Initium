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
 * @file
 * @brief               Initium kernel loader.
 */

#include <lib/string.h>
#include <lib/utility.h>

#include <loader/initium.h>

#include <assert.h>
#include <config.h>
#include <device.h>
#include <disk.h>
#include <loader.h>
#include <memory.h>
#include <ui.h>

#include "initium_elf.h"

/**
 * Helper functions.
 */

/** Find a tag in the image tag list.
 * @param loader        Loader internal data.
 * @param type          Type of tag to find.
 * @return              Pointer to tag, or NULL if not found. */
void *initium_find_itag(initium_loader_t *loader, uint32_t type) {
    list_foreach(&loader->itags, iter) {
        initium_itag_t *itag = list_entry(iter, initium_itag_t, header);

        if (itag->type == type)
            return itag->data;
    }

    return NULL;
}

/** Get the next tag of the same type.
 * @param loader        Loader internal data.
 * @param data          Current tag.
 * @return              Pointer to next tag of same type, or NULL if none found. */
void *initium_next_itag(initium_loader_t *loader, void *data) {
    initium_itag_t *itag = container_of(data, initium_itag_t, data);
    uint32_t type = itag->type;

    while (itag != list_last(&loader->itags, initium_itag_t, header)) {
        itag = list_next(itag, header);
        if (itag->type == type)
            return itag->data;
    }

    return NULL;
}

/** Allocate a tag list entry.
 * @param loader        Loader internal data.
 * @param type          Type of the tag.
 * @param size          Size of the tag data.
 * @return              Pointer to allocated tag. Will be cleared to 0. */
void *initium_alloc_tag(initium_loader_t *loader, uint32_t type, size_t size) {
    initium_tag_t *ret;

    ret = (initium_tag_t *)phys_to_virt(loader->core->tags_phys + loader->core->tags_size);
    memset(ret, 0, size);
    ret->type = type;
    ret->size = size;

    loader->core->tags_size += round_up(size, 8);
    if (loader->core->tags_size > PAGE_SIZE)
        internal_error("Exceeded maximum tag list size");

    return ret;
}

/** Check whether a virtual mapping is valid.
 * @param loader        Loader internal data.
 * @param mapping       Mapping to check.
 * @param addr          Virtual address to map at.
 * @param phys          Physical address to map to (or ~0 for no mapping).
 * @param size          Size of the range.
 * @return              Whether mapping is valid. */
static bool check_mapping(initium_loader_t *loader, initium_vaddr_t addr, initium_paddr_t phys, initium_vaddr_t size) {
    if (!size || size % PAGE_SIZE)
        return false;

    if (addr != ~(initium_vaddr_t)0) {
        if (addr % PAGE_SIZE) {
            return false;
        } else if (addr + size - 1 < addr) {
            return false;
        } else if (loader->mode == LOAD_MODE_32BIT && addr + size - 1 >= 0x100000000ull) {
            return false;
        }
    }

    if (phys != ~(initium_paddr_t)0 && phys % PAGE_SIZE)
        return false;

    return true;
}

/** Insert a virtual address mapping.
 * @param loader        Loader internal data.
 * @param start         Virtual address of start of mapping.
 * @param size          Size of the mapping.
 * @param phys          Physical address. */
static void add_mapping(initium_loader_t *loader, load_ptr_t start, load_size_t size, phys_ptr_t phys) {
    initium_mapping_t *mapping;

    /* All virtual memory tags should be provided together in the tag list,
     * sorted in address order. To do this, we must maintain mapping info
     * separately in sorted order, then add it all to the tag list at once. */
    mapping = malloc(sizeof(*mapping));
    mapping->start = start;
    mapping->size = size;
    mapping->phys = (phys == ~(phys_ptr_t)0) ? ~(initium_paddr_t)0 : phys;

    list_init(&mapping->header);

    list_foreach(&loader->mappings, iter) {
        initium_mapping_t *other = list_entry(iter, initium_mapping_t, header);

        if (mapping->start <= other->start) {
            list_add_before(&other->header, &mapping->header);
            return;
        }
    }

    list_append(&loader->mappings, &mapping->header);
}

/** Allocate virtual address space.
 * @param loader        Loader internal data.
 * @param phys          Physical address to map to (or ~0 for no mapping).
 * @param size          Size of the range.
 * @return              Virtual address of mapping. */
initium_vaddr_t initium_alloc_virtual(initium_loader_t *loader, initium_paddr_t phys, initium_vaddr_t size) {
    load_ptr_t addr;

    if (!check_mapping(loader, ~(initium_vaddr_t)0, phys, size))
        boot_error("Invalid virtual mapping (physical 0x%" PRIx64 ")", phys);

    if (!allocator_alloc(&loader->allocator, size, 0, &addr))
        boot_error("Insufficient address space available (allocating %" PRIuLOAD " bytes)", size);

    if (phys != ~(initium_paddr_t)0) {
        /* Architecture code does extra validation. */
        if (!mmu_map(loader->mmu, addr, phys, size))
            boot_error("Invalid virtual mapping (physical 0x%" PRIx64 ")", phys);
    }

    add_mapping(loader, addr, size, phys);
    return addr;
}

/** Map at a location in the virtual address space.
 * @param loader        Loader internal data.
 * @param addr          Virtual address to map at.
 * @param phys          Physical address to map to (or ~0 for no mapping).
 * @param size          Size of the range. */
void initium_map_virtual(initium_loader_t *loader, initium_vaddr_t addr, initium_paddr_t phys, initium_vaddr_t size) {
    if (!check_mapping(loader, addr, phys, size))
        boot_error("Invalid virtual mapping (virtual 0x%" PRIx64 ")", addr);

    if (!allocator_insert(&loader->allocator, addr, size))
        boot_error("Mapping 0x%" PRIxLOAD " conflicts with another", addr);

    if (phys != ~(initium_paddr_t)0) {
        if (!mmu_map(loader->mmu, addr, phys, size))
            boot_error("Invalid virtual mapping (virtual 0x%" PRIx64 ")", addr);
    }

    add_mapping(loader, addr, size, phys);
}

/**
 * Loader implementation.
 */

/** Allocate the tag list.
 * @param loader        Loader internal data. */
static void alloc_tag_list(initium_loader_t *loader) {
    initium_tag_core_t *core;
    phys_ptr_t phys;

    /* For now, assume that the tag list never exceeds a page. */
    core = memory_alloc(PAGE_SIZE, 0, 0, 0, MEMORY_TYPE_RECLAIMABLE, MEMORY_ALLOC_HIGH, &phys);
    memset(core, 0, sizeof(*core));
    core->header.type = INITIUM_TAG_CORE;
    core->header.size = sizeof(*core);
    core->tags_phys = phys;
    core->tags_size = round_up(sizeof(*core), 8);

    /* Will be mapped into the virtual address space later, as we cannot yet
     * perform virtual allocations. */
    loader->core = core;
}

/** Check whether alignment parameters are valid.
 * @param load          Load parameters.
 * @return              Whether parameters are valid. */
static bool check_alignment_params(initium_itag_load_t *load) {
    if (load->alignment) {
        if (load->alignment < PAGE_SIZE) {
            return false;
        } else if (!is_pow2(load->alignment)) {
            return false;
        }
    }

    if (load->min_alignment) {
        if (load->min_alignment < PAGE_SIZE || load->min_alignment > load->alignment) {
            return false;
        } else if (!is_pow2(load->min_alignment)) {
            return false;
        }
    } else {
        load->min_alignment = load->alignment;
    }

    return true;
}

/** Check whether virtual map parameters are valid.
 * @param loader        Loader internal data.
 * @param load          Load parameters.
 * @return              Whether parameters are valid. */
static bool check_virt_map_params(initium_loader_t *loader, initium_itag_load_t *load) {
    if (load->virt_map_base % PAGE_SIZE || load->virt_map_size % PAGE_SIZE) {
        return false;
    } else if (load->virt_map_base && !load->virt_map_size) {
        return false;
    } else if ((load->virt_map_base + load->virt_map_size - 1) < load->virt_map_base) {
        return false;
    }

    if (loader->mode == LOAD_MODE_32BIT) {
        if (!load->virt_map_base && !load->virt_map_size) {
            load->virt_map_size = 0x100000000ull;
        } else if (load->virt_map_base + load->virt_map_size > 0x100000000ull) {
            return false;
        }
    }

    return true;
}

/** Load kernel modules.
 * @param loader        Loader internal data. */
static void load_modules(initium_loader_t *loader) {
    list_foreach(&loader->modules, iter) {
        initium_module_t *module = list_entry(iter, initium_module_t, header);
        void *dest;
        phys_ptr_t phys;
        size_t size, name_size;
        initium_tag_module_t *tag;
        status_t ret;

        /* Allocate a chunk of memory to load to. */
        size = round_up(module->handle->size, PAGE_SIZE);
        dest = memory_alloc(size, 0, 0, 0, MEMORY_TYPE_MODULES, MEMORY_ALLOC_HIGH, &phys);

        dprintf(
            "initium: loading module '%s' to 0x%" PRIxPHYS " (size: %" PRIu64 ")\n",
            module->name, phys, module->handle->size);

        ret = fs_read(module->handle, dest, module->handle->size, 0);
        if (ret != STATUS_SUCCESS)
            boot_error("Error %d reading module '%s'", ret, module->name);

        name_size = strlen(module->name) + 1;

        tag = initium_alloc_tag(loader, INITIUM_TAG_MODULE, round_up(sizeof(*tag), 8) + name_size);
        tag->addr = phys;
        tag->size = module->handle->size;
        tag->name_size = name_size;

        memcpy((char *)tag + round_up(sizeof(*tag), 8), module->name, name_size);
    }
}

/** Set up the trampoline for the kernel.
 * @param loader        Loader internal data. */
static void setup_trampoline(initium_loader_t *loader) {
    ptr_t loader_start, loader_size;
    phys_ptr_t loader_phys;

    /*
     * Here we have the interesting task of setting things up so that we can
     * enter the kernel. It is not always possible to identity map the boot
     * loader: it is possible that something has been mapped into the virtual
     * address space at the identity mapped location. So, the procedure we use
     * to enter the kernel is as follows:
     *
     *  - Allocate a page and map this into the kernel's virtual address space,
     *    ensuring it does not conflict with the address range the loader is
     *    running at.
     *  - Construct a temporary address space that identity maps the loader and
     *    the allocated page.
     *  - Architecture entry code copies a piece of trampoline code to the page,
     *    then enables the MMU and switches to the target operating mode using
     *    the temporary address space.
     *  - Jump to the trampoline code which switches to the real address space
     *    and then jumps to the kernel.
     *
     * All allocated page tables for the temporary address space are marked as
     * internal so the kernel won't see them as in use at all.
     */

    /* Avoid the loader's address range. */
    loader_start = round_down((ptr_t)__start, PAGE_SIZE);
    loader_size = round_up((ptr_t)__end, PAGE_SIZE) - loader_start;
    allocator_reserve(&loader->allocator, loader_start, loader_size);

    /* Allocate a page and map it. */
    memory_alloc(PAGE_SIZE, 0, 0, 0, MEMORY_TYPE_INTERNAL, MEMORY_ALLOC_HIGH, &loader->trampoline_phys);
    loader->trampoline_virt = initium_alloc_virtual(loader, loader->trampoline_phys, PAGE_SIZE);

    /* Create an MMU context which maps the loader and the trampoline page. */
    loader->trampoline_mmu = mmu_context_create(loader->mode, MEMORY_TYPE_INTERNAL);
    loader_phys = virt_to_phys(loader_start);
    mmu_map(loader->trampoline_mmu, loader_start, loader_phys, loader_size);
    mmu_map(loader->trampoline_mmu, loader->trampoline_virt, loader->trampoline_phys, PAGE_SIZE);
}

/** Pass options to the kernel.
 * @param loader        Loader internal data. */
static void add_option_tags(initium_loader_t *loader) {
    initium_itag_foreach(loader, INITIUM_ITAG_OPTION, initium_itag_option_t, option) {
        char *name = (char *)option + sizeof(*option);
        value_t *value;
        void *data;
        size_t name_size, data_size, size;
        initium_tag_option_t *tag;

        /* All options are added to the environment by config_cmd_initium(). */
        value = environ_lookup(current_environ, name);
        assert(value);

        switch (option->type) {
        case INITIUM_OPTION_BOOLEAN:
            assert(value->type == VALUE_TYPE_BOOLEAN);

            data = &value->boolean;
            data_size = sizeof(value->boolean);
            break;
        case INITIUM_OPTION_STRING:
            assert(value->type == VALUE_TYPE_STRING);

            data = value->string;
            data_size = strlen(value->string) + 1;
            break;
        case INITIUM_OPTION_INTEGER:
            assert(value->type == VALUE_TYPE_STRING);

            data = &value->integer;
            data_size = sizeof(value->integer);
            break;
        default:
            unreachable();
        }

        name_size = strlen(name) + 1;
        size = round_up(sizeof(*tag), 8) + round_up(name_size, 8) + data_size;

        tag = initium_alloc_tag(loader, INITIUM_TAG_OPTION, size);
        tag->type = option->type;
        tag->name_size = name_size;
        tag->value_size = data_size;

        memcpy((char *)tag + round_up(sizeof(*tag), 8), name, name_size);
        memcpy((char *)tag + round_up(sizeof(*tag), 8) + round_up(name_size, 8), data, data_size);
    }
}

/**
 * Add a file system boot device tag.
 *
 * @param loader        Loader internal data.
 * @param uuid          UUID string.
 */
static void add_fs_bootdev_tag(initium_loader_t *loader, const char *uuid) {
    initium_tag_bootdev_t *tag = initium_alloc_tag(loader, INITIUM_TAG_BOOTDEV, sizeof(*tag));

    tag->type = INITIUM_BOOTDEV_FS;
    tag->fs.flags = 0;

    if (uuid) {
        strncpy((char *)tag->fs.uuid, uuid, sizeof(tag->fs.uuid));
        tag->fs.uuid[sizeof(tag->fs.uuid) - 1] = 0;
    } else {
        tag->fs.uuid[0] = 0;
    }
}

/** Add a tag for a device specifier string.
 * @param loader        Loader internal data.
 * @param str           Device specifier string. */
static void add_other_bootdev_tag(initium_loader_t *loader, const char *str) {
    initium_tag_bootdev_t *tag;
    size_t len;

    len = strlen(str) + 1;

    tag = initium_alloc_tag(loader, INITIUM_TAG_BOOTDEV, sizeof(*tag));
    tag->type = INITIUM_BOOTDEV_OTHER;
    tag->other.str_len = len;

    memcpy((char *)tag + round_up(sizeof(*tag), 8), str, len);
}

/** Add boot device information to the tag list.
 * @param loader        Loader internal data. */
static void add_bootdev_tag(initium_loader_t *loader) {
    device_t *device;
    const value_t *value;
    initium_tag_bootdev_t *tag;

    value = environ_lookup(current_environ, "root_device");
    if (value) {
        assert(value->type == VALUE_TYPE_STRING);

        if (strncmp(value->string, "other:", 6) == 0) {
            add_other_bootdev_tag(loader, &value->string[6]);
            return;
        } else if (strncmp(value->string, "uuid:", 5) == 0) {
            add_fs_bootdev_tag(loader, &value->string[5]);
            return;
        }

        device = device_lookup(value->string);
        assert(device);
    } else {
        device = loader->handle->mount->device;
    }

    if (device->mount && device->mount->uuid) {
        add_fs_bootdev_tag(loader, device->mount->uuid);
        return;
    }

    /* Nothing usable. TODO: network */
    tag = initium_alloc_tag(loader, INITIUM_TAG_BOOTDEV, sizeof(*tag));
    tag->type = INITIUM_TAG_NONE;
}

/** Add physical memory information to the tag list.
 * @param loader        Loader internal data. */
static void add_memory_tags(initium_loader_t *loader) {
    list_t memory_map;

    /* Reclaim all memory used internally. */
    memory_finalize(&memory_map);

    /* Dump the memory map to the debug console. */
    dprintf("initium: final physical memory map:\n");
    memory_map_dump(&memory_map);

    /* Add tags for each range. */
    list_foreach(&memory_map, iter) {
        memory_range_t *range = list_entry(iter, memory_range_t, header);
        initium_tag_memory_t *tag = initium_alloc_tag(loader, INITIUM_TAG_MEMORY, sizeof(*tag));

        tag->start = range->start;
        tag->size = range->size;
        tag->type = range->type;
    }
}

/** Add virtual memory information to the tag list.
 * @param loader        Loader internal data. */
static void add_vmem_tags(initium_loader_t *loader) {
    dprintf("initium: final virtual memory map:\n");

    list_foreach(&loader->mappings, iter) {
        initium_mapping_t *mapping = list_entry(iter, initium_mapping_t, header);
        initium_tag_vmem_t *tag = initium_alloc_tag(loader, INITIUM_TAG_VMEM, sizeof(*tag));

        tag->start = mapping->start;
        tag->size = mapping->size;
        tag->phys = mapping->phys;

        dprintf(" 0x%" PRIx64 "-0x%" PRIx64 " -> 0x%" PRIx64 "\n", tag->start, tag->start + tag->size, tag->phys);
    }
}

/** Load a KBoot kernel.
 * @param _loader       Pointer to loader internal data. */
static __noreturn void initium_loader_load(void *_loader) {
    initium_loader_t *loader = _loader;
    phys_ptr_t phys;

    dprintf("initium: version %" PRIu32 " image, flags 0x%" PRIx32 "\n", loader->image->version, loader->image->flags);

    /* Check whether the kernel is supported (CPU feature requirements, etc). */
    initium_arch_check_kernel(loader);

    /* Allocate the tag list. */
    alloc_tag_list(loader);

    /* Validate load parameters. */
    loader->load = initium_find_itag(loader, INITIUM_ITAG_LOAD);
    if (loader->load) {
        if (!check_alignment_params(loader->load))
            boot_error("Invalid kernel alignment parameters");
        if (!check_virt_map_params(loader, loader->load))
            boot_error("Invalid kernel virtual map range");
    } else {
        /* No load tag, create one and initialize everything to zero. */
        loader->load = malloc(sizeof(*loader->load));
        memset(loader->load, 0, sizeof(*loader->load));
    }

    /* Have the architecture do its own validation and fill in defaults. */
    initium_arch_check_load_params(loader, loader->load);

    /* Create the virtual address space and address allocator. */
    loader->mmu = mmu_context_create(loader->mode, MEMORY_TYPE_PAGETABLES);
    allocator_init(&loader->allocator, loader->load->virt_map_base, loader->load->virt_map_size);

    /* Ensure that we never allocate virtual address 0. */
    allocator_reserve(&loader->allocator, 0, PAGE_SIZE);

    /* Load the kernel image. */
    initium_elf_load_kernel(loader);

    /* Perform all mappings specified by the kernel image. */
    initium_itag_foreach(loader, INITIUM_ITAG_MAPPING, initium_itag_mapping_t, mapping) {
        if (mapping->virt == ~(initium_vaddr_t)0) {
            initium_alloc_virtual(loader, mapping->phys, mapping->size);
        } else {
            initium_map_virtual(loader, mapping->virt, mapping->phys, mapping->size);
        }
    }

    /* Perform architecture setup. */
    initium_arch_setup(loader);

    /* Now we can allocate a virtual mapping for the tag list. */
    loader->tags_virt = initium_alloc_virtual(loader, loader->core->tags_phys, PAGE_SIZE);

    /* Load additional sections if requested. */
    if (loader->image->flags & INITIUM_IMAGE_SECTIONS)
        initium_elf_load_sections(loader);

    /* Load modules. */
    load_modules(loader);

    /* Allocate the stack. */
    memory_alloc(PAGE_SIZE, 0, 0, 0, MEMORY_TYPE_STACK, MEMORY_ALLOC_HIGH, &phys);
    loader->core->stack_base = initium_alloc_virtual(loader, phys, PAGE_SIZE);
    loader->core->stack_phys = phys;
    loader->core->stack_size = PAGE_SIZE;

    /* Set up the kernel entry trampoline. */
    setup_trampoline(loader);

    /* Add other information tags. All memory allocation is done at this point. */
    add_option_tags(loader);
    add_bootdev_tag(loader);
    add_memory_tags(loader);
    add_vmem_tags(loader);

    /* End the tag list. */
    initium_alloc_tag(loader, INITIUM_TAG_NONE, sizeof(initium_tag_t));

    /* Start the kernel. */
    dprintf(
        "initium: entering kernel at 0x%" PRIx64 " (trampoline_phys: 0x%" PRIxPHYS ", trampoline_virt: 0x%" PRIx64 ")\n",
        loader->entry, loader->trampoline_phys, loader->trampoline_virt);

    initium_arch_enter(loader);
}

/** Get a configuration window.
 * @param _loader       Pointer to loader internal data.
 * @param title         Title to give the window.
 * @return              Configuration window. */
static ui_window_t *initium_loader_configure(void *_loader, const char *title) {
    initium_loader_t *loader = _loader;
    ui_window_t *window;

    window = ui_list_create(title, true);

    // TODO: video mode

    /* Add entries for each option. */
    initium_itag_foreach(loader, INITIUM_ITAG_OPTION, initium_itag_option_t, option) {
        char *name = (char *)option + sizeof(*option);
        char *desc = (char *)option + sizeof(*option) + option->name_size;
        value_t *value;
        ui_entry_t *entry;

        /* All entries should be added and the correct type at this point. */
        value = environ_lookup(current_environ, name);
        assert(value);
        entry = ui_entry_create(desc, value);
        ui_list_insert(window, entry, false);
    }

    return window;
}

/** Initium loader operations. */
static loader_ops_t initium_loader_ops = {
    .load = initium_loader_load,
    .configure = initium_loader_configure,
};

/**
 * Configuration command.
 */

/** Check whether the command arguments are valid.
 * @param args          Arguments to check.
 * @return              Whether arguments are valid. */
static bool check_args(value_list_t *args) {
    if (args->count != 1 && args->count != 2)
        return false;

    if (args->values[0].type != VALUE_TYPE_STRING)
        return false;

    if (args->count == 2) {
        if (args->values[1].type == VALUE_TYPE_LIST) {
            value_list_t *list = args->values[1].list;

            for (size_t i = 0; i < list->count; i++) {
                if (list->values[i].type != VALUE_TYPE_STRING)
                    return false;
            }
        } else if (args->values[1].type != VALUE_TYPE_STRING) {
            return false;
        }
    }

    return true;
}

/** Add an image tag from a Initium kernel.
 * @param loader        Loader internal data.
 * @param note          Note header.
 * @param desc          Note data.
 * @return              Whether to continue iteration. */
static bool add_image_tag(initium_loader_t *loader, elf_note_t *note, void *desc) {
    size_t size;
    bool can_duplicate;
    initium_itag_t *tag;

    loader->success = false;

    switch (note->n_type) {
    case INITIUM_ITAG_IMAGE:
        size = sizeof(initium_itag_image_t);
        can_duplicate = false;
        break;
    case INITIUM_ITAG_LOAD:
        size = sizeof(initium_itag_load_t);
        can_duplicate = false;
        break;
    case INITIUM_ITAG_VIDEO:
        size = sizeof(initium_itag_video_t);
        can_duplicate = false;
        break;
    case INITIUM_ITAG_OPTION:
        size = sizeof(initium_itag_option_t);
        can_duplicate = true;
        break;
    case INITIUM_ITAG_MAPPING:
        size = sizeof(initium_itag_mapping_t);
        can_duplicate = true;
        break;
    default:
        config_error("initium: '%s' has unrecognized image tag type %" PRIu32, loader->path, note->n_type);
        return false;
    }

    if (note->n_descsz < size) {
        config_error("initium: '%s' has undersized tag type %" PRIu32, loader->path, note->n_type);
        return false;
    } else if (!can_duplicate && initium_find_itag(loader, note->n_type)) {
        config_error("initium: '%s' has multiple tags of type %" PRIu32, loader->path);
        return false;
    }

    /* May be extra data following the tag header. */
    size = max(size, note->n_descsz);

    tag = malloc(sizeof(initium_itag_t) + size);
    tag->type = note->n_type;
    memcpy(tag->data, desc, size);

    list_init(&tag->header);
    list_append(&loader->itags, &tag->header);

    loader->success = true;
    return true;
}

/** Add options to the environment.
 * @param loader        Loader internal data.
 * @return              Whether successful. */
static bool add_options(initium_loader_t *loader) {
    initium_itag_foreach(loader, INITIUM_ITAG_OPTION, initium_itag_option_t, option) {
        char *name = (char *)option + sizeof(*option);
        void *initial = (char *)option + sizeof(*option) + option->name_size + option->desc_size;
        const value_t *exist;
        value_t value;

        switch (option->type) {
        case INITIUM_OPTION_BOOLEAN:
            value.type = VALUE_TYPE_BOOLEAN;
            value.boolean = *(bool *)initial;
            break;
        case INITIUM_OPTION_STRING:
            value.type = VALUE_TYPE_STRING;
            value.string = initial;
            break;
        case INITIUM_OPTION_INTEGER:
            value.type = VALUE_TYPE_INTEGER;
            value.integer = *(uint64_t *)initial;
            break;
        default:
            config_error("initium: '%s' has invalid option type %" PRIu32 " ('%s')", loader->path, option->type, name);
            return false;
        }

        /* Don't overwrite an existing value. */
        exist = environ_lookup(current_environ, name);
        if (exist) {
            if (exist->type != value.type) {
                config_error("initium: Invalid value type set for option '%s'", name);
                return false;
            }
        } else {
            environ_insert(current_environ, name, &value);
        }
    }

    return true;
}

/** Add a module list
 * @param loader        Loader internal data.
 * @param list          List of modules to add.
 * @return              Whether successful. */
static bool add_module_list(initium_loader_t *loader, const value_list_t *list) {
    for (size_t i = 0; i < list->count; i++) {
        const char *path = list->values[i].string;
        initium_module_t *module;
        char *name;
        status_t ret;

        module = malloc(sizeof(*module));

        ret = fs_open(path, NULL, &module->handle);
        if (ret != STATUS_SUCCESS) {
            config_error("initium: Error %d opening module '%s'", ret, path);
            free(module);
            return false;
        } else if (module->handle->directory) {
            config_error("initium: '%s' is a directory", path);
            fs_close(module->handle);
            free(module);
            return false;
        }

        name = strrchr(path, '/');
        if (name) {
            module->name = strdup(name + 1);
        } else {
            module->name = list->values[i].string;
            list->values[i].string = NULL;
        }

        list_init(&module->header);
        list_append(&loader->modules, &module->header);
    }

    return true;
}

/** Directory iteration callback to add a module.
 * @param entry         Details of the entry that was found.
 * @param _loader       Pointer to loader data.
 * @return              Whether to continue iteration. */
static bool add_module_dir_cb(const fs_entry_t *entry, void *_loader) {
    initium_loader_t *loader = _loader;
    initium_module_t *module;
    status_t ret;

    module = malloc(sizeof(*module));

    ret = fs_open_entry(entry, &module->handle);
    if (ret != STATUS_SUCCESS) {
        config_error("initium: Error %d opening module '%s'", ret, entry->name);
        free(module);
        loader->success = false;
        return false;
    } else if (module->handle->directory) {
        /* Ignore it. */
        fs_close(module->handle);
        free(module);
        return true;
    }

    module->name = strdup(entry->name);

    list_init(&module->header);
    list_append(&loader->modules, &module->header);

    return true;
}

/** Add modules from a directory.
 * @param loader        Loader internal data.
 * @param path          Path to module directory.
 * @return              Whether successful. */
static bool add_module_dir(initium_loader_t *loader, const char *path) {
    fs_handle_t *handle;
    status_t ret;

    ret = fs_open(path, NULL, &handle);
    if (ret != STATUS_SUCCESS) {
        config_error("initium: Error %d opening '%s'", ret, path);
        return false;
    } else if (!handle->directory) {
        config_error("initium: '%s' is not a directory", path);
        fs_close(handle);
        return false;
    }

    loader->success = true;

    ret = fs_iterate(handle, add_module_dir_cb, loader);
    fs_close(handle);
    if (ret != STATUS_SUCCESS) {
        config_error("initium: Error %d iterating '%s'", ret, path);
        return false;
    }

    return loader->success;
}

/** Load a Initium kernel.
 * @param args          Argument list.
 * @return              Whether successful. */
static bool config_cmd_initium(value_list_t *args) {
    initium_loader_t *loader;
    const value_t *value;
    status_t ret;

    if (!check_args(args)) {
        config_error("initium: Invalid arguments");
        return false;
    }

    loader = malloc(sizeof(*loader));
    list_init(&loader->modules);
    list_init(&loader->itags);
    list_init(&loader->mappings);
    loader->path = args->values[0].string;

    /* Open the kernel image. */
    ret = fs_open(loader->path, NULL, &loader->handle);
    if (ret != STATUS_SUCCESS) {
        config_error("initium: Error %d opening '%s'", ret, loader->path);
        goto err_free;
    } else if (loader->handle->directory) {
        config_error("initium: '%s' is a directory", loader->path);
        goto err_close;
    }

    /* Check if the image is a valid ELF image. */
    ret = initium_elf_identify(loader);
    if (ret != STATUS_SUCCESS) {
        if (ret == STATUS_UNKNOWN_IMAGE) {
            config_error("initium: '%s' is not a supported ELF image", loader->path);
        } else {
            config_error("initium: Error %d reading '%s'", ret, loader->path);
        }

        goto err_close;
    }

    /* Search all image tags. */
    loader->success = true;
    ret = initium_elf_iterate_notes(loader, add_image_tag);
    if (ret != STATUS_SUCCESS) {
        config_error("initium: Error %d while loading image tags from '%s'", ret, loader->path);
        goto err_itags;
    } else if (!loader->success) {
        goto err_itags;
    }

    /* Check if we have a valid image tag. */
    loader->image = initium_find_itag(loader, INITIUM_ITAG_IMAGE);
    if (!loader->image) {
        config_error("initium: '%s' is not a Initium kernel", loader->path);
        goto err_itags;
    } else if (loader->image->version != INITIUM_VERSION) {
        config_error("initium: '%s' has unsupported KBoot version %" PRIu32, loader->path, loader->image->version);
        goto err_itags;
    }

    /* Add options to the environment. */
    if (!add_options(loader))
        goto err_itags;

    /* Look for a root device option. */
    value = environ_lookup(current_environ, "root_device");
    if (value) {
        if (value->type != VALUE_TYPE_STRING) {
            config_error("initium: 'root_device' option should be a string");
            goto err_itags;
        }

        /* We can pass a UUID to the kernel without knowing the actual device.
         * TODO: Add label support as well? */
        if (strncmp(value->string, "other:", 6) != 0 && strncmp(value->string, "uuid:", 5) != 0) {
            if (!device_lookup(value->string)) {
                config_error("initium: Root device '%s' not found", value->string);
                goto err_itags;
            }
        }
    }

    /* Open all specified modules. Argument types already checked here. */
    if (args->count >= 2) {
        if (args->values[1].type == VALUE_TYPE_LIST) {
            if (!add_module_list(loader, args->values[1].list))
                goto err_modules;
        } else {
            if (!add_module_dir(loader, args->values[1].string))
                goto err_modules;
        }
    }

    environ_set_loader(current_environ, &initium_loader_ops, loader);
    return true;

err_modules:
    while (!list_empty(&loader->modules)) {
        initium_module_t *module = list_first(&loader->modules, initium_module_t, header);

        list_remove(&module->header);
        fs_close(module->handle);
        free(module->name);
        free(module);
    }

err_itags:
    while (!list_empty(&loader->itags)) {
        initium_itag_t *itag = list_first(&loader->itags, initium_itag_t, header);

        list_remove(&itag->header);
        free(itag);
    }

    free(loader->phdrs);
    free(loader->ehdr);

err_close:
    fs_close(loader->handle);

err_free:
    free(loader);
    return false;
}

BUILTIN_COMMAND("initium", config_cmd_initium);
