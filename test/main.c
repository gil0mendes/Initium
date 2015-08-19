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
* FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

/**
 * @file
 * @brief               Initium test kernel.
 */

#include <lib/utility.h>

#include "test.h"

INITIUM_IMAGE(INITIUM_IMAGE_SECTIONS | INITIUM_IMAGE_LOG);
INITIUM_VIDEO(INITIUM_VIDEO_LFB | INITIUM_VIDEO_VGA, 0, 0, 0);
INITIUM_BOOLEAN_OPTION("bool_option", "Boolean Option", true);
INITIUM_STRING_OPTION("string_option", "String Option", "Default Value");
INITIUM_LOAD(0, 0, 0, VIRT_MAP_BASE, VIRT_MAP_SIZE);
INITIUM_MAPPING(PHYS_MAP_BASE, PHYS_MAP_OFFSET, PHYS_MAP_SIZE);

/** Dump a core tag. */
static void dump_core_tag(initium_tag_core_t *tag) {
    printf("INITIUM_TAG_CORE:\n");
    printf("  tags_phys   = 0x%" PRIx64 "\n", tag->tags_phys);
    printf("  tags_size   = %" PRIu32 "\n", tag->tags_size);
    printf("  kernel_phys = 0x%" PRIx64 "\n", tag->kernel_phys);
    printf("  stack_base  = 0x%" PRIx64 "\n", tag->stack_base);
    printf("  stack_phys  = 0x%" PRIx64 "\n", tag->stack_phys);
    printf("  stack_size  = %" PRIu32 "\n", tag->stack_size);
}

/** Dump an option tag. */
static void dump_option_tag(initium_tag_option_t *tag) {
    const char *name;
    void *value;

    printf("INITIUM_TAG_OPTION:\n");
    printf("  type       = %" PRIu8 "\n", tag->type);
    printf("  name_size  = %" PRIu32 "\n", tag->name_size);
    printf("  value_size = %" PRIu32 "\n", tag->value_size);

    name = (const char *)round_up((ptr_t)tag + sizeof(initium_tag_option_t), 8);
    printf("  name       = `%s'\n", name);

    value = (void *)round_up((ptr_t)name + tag->name_size, 8);
    switch (tag->type) {
    case INITIUM_OPTION_BOOLEAN:
        printf("  value      = boolean: %d\n", *(bool *)value);
        break;
    case INITIUM_OPTION_STRING:
        printf("  value      = string: `%s'\n", (const char *)value);
        break;
    case INITIUM_OPTION_INTEGER:
        printf("  value      = integer: %" PRIu64 "\n", *(uint64_t *)value);
        break;
    default:
        printf("  <unknown type>\n");
        break;
    }
}

/** Get a memory range tag type. */
static const char *memory_tag_type(uint32_t type) {
    switch (type) {
    case INITIUM_MEMORY_FREE:
        return "Free";
    case INITIUM_MEMORY_ALLOCATED:
        return "Allocated";
    case INITIUM_MEMORY_RECLAIMABLE:
        return "Reclaimable";
    case INITIUM_MEMORY_PAGETABLES:
        return "Pagetables";
    case INITIUM_MEMORY_STACK:
        return "Stack";
    case INITIUM_MEMORY_MODULES:
        return "Modules";
    default:
        return "???";
    }
}

/** Dump a memory tag. */
static void dump_memory_tag(initium_tag_memory_t *tag) {
    printf("INITIUM_TAG_MEMORY:\n");
    printf("  start = 0x%" PRIx64 "\n", tag->start);
    printf("  size  = 0x%" PRIx64 "\n", tag->size);
    printf("  end   = 0x%" PRIx64 "\n", tag->start + tag->size);
    printf("  type  = %u (%s)\n", tag->type, memory_tag_type(tag->type));
}

/** Dump a virtual memory tag. */
static void dump_vmem_tag(initium_tag_vmem_t *tag) {
    printf("INITIUM_TAG_VMEM:\n");
    printf("  start = 0x%" PRIx64 "\n", tag->start);
    printf("  size  = 0x%" PRIx64 "\n", tag->size);
    printf("  end   = 0x%" PRIx64 "\n", tag->start + tag->size);
    printf("  phys  = 0x%" PRIx64 "\n", tag->phys);
}

/** Dump a pagetables tag. */
static void dump_pagetables_tag(initium_tag_pagetables_t *tag) {
    printf("INITIUM_TAG_PAGETABLES:\n");
    #ifdef __x86_64__
        printf("  pml4    = 0x%" PRIx64 "\n", tag->pml4);
        printf("  mapping = 0x%" PRIx64 "\n", tag->mapping);
    #elif defined(__i386__)
        printf("  page_dir = 0x%" PRIx64 "\n", tag->page_dir);
        printf("  mapping  = 0x%" PRIx64 "\n", tag->mapping);
    #elif defined(__arm__)
        printf("  l1      = 0x%" PRIx64 "\n", tag->l1);
        printf("  mapping = 0x%" PRIx64 "\n", tag->mapping);
    #endif
}

/** Dump a module tag. */
static void dump_module_tag(initium_tag_module_t *tag) {
    const char *name;
    printf("INITIUM_TAG_MODULE:\n");
    printf("  addr      = 0x%" PRIx64 "\n", tag->addr);
    printf("  size      = %" PRIu32 "\n", tag->size);
    printf("  name_size = %" PRIu32 "\n", tag->name_size);

    name = (const char *)round_up((ptr_t)tag + sizeof(initium_tag_module_t), 8);
    printf("  name      = `%s'\n", name);
}

/** Dump a video tag. */
static void dump_video_tag(initium_tag_video_t *tag) {
    printf("INITIUM_TAG_VIDEO:\n");

    switch (tag->type) {
    case INITIUM_VIDEO_VGA:
        printf("  type     = %u (INITIUM_VIDEO_VGA)\n", tag->type);
        printf("  cols     = %u\n", tag->vga.cols);
        printf("  lines    = %u\n", tag->vga.lines);
        printf("  x        = %u\n", tag->vga.x);
        printf("  y        = %u\n", tag->vga.y);
        printf("  mem_phys = 0x%" PRIx64 "\n", tag->vga.mem_phys);
        printf("  mem_virt = 0x%" PRIx64 "\n", tag->vga.mem_virt);
        printf("  mem_size = 0x%" PRIx32 "\n", tag->vga.mem_size);
        break;
    case INITIUM_VIDEO_LFB:
        printf("  type       = %u (INITIUM_VIDEO_LFB)\n", tag->type);
        printf("  flags      = 0x%" PRIx32 "\n", tag->lfb.flags);
        if (tag->lfb.flags & INITIUM_LFB_RGB)
            printf("    INITIUM_LFB_RGB\n");
        if (tag->lfb.flags & INITIUM_LFB_INDEXED)
            printf("    INITIUM_LFB_INDEXED\n");
        printf("  width      = %" PRIu32 "\n", tag->lfb.width);
        printf("  height     = %" PRIu32 "\n", tag->lfb.height);
        printf("  bpp        = %" PRIu8 "\n", tag->lfb.bpp);
        printf("  pitch      = %" PRIu32 "\n", tag->lfb.pitch);
        printf("  fb_phys    = 0x%" PRIx64 "\n", tag->lfb.fb_phys);
        printf("  fb_virt    = 0x%" PRIx64 "\n", tag->lfb.fb_virt);
        printf("  fb_size    = 0x%" PRIx32 "\n", tag->lfb.fb_size);

        if (tag->lfb.flags & INITIUM_LFB_RGB) {
            printf("  red_size   = %" PRIu8 "\n", tag->lfb.red_size);
            printf("  red_pos    = %" PRIu8 "\n", tag->lfb.red_pos);
            printf("  green_size = %" PRIu8 "\n", tag->lfb.green_size);
            printf("  green_pos  = %" PRIu8 "\n", tag->lfb.green_pos);
            printf("  blue_size  = %" PRIu8 "\n", tag->lfb.blue_size);
            printf("  blue_pos   = %" PRIu8 "\n", tag->lfb.blue_pos);
        } else if (tag->lfb.flags & INITIUM_LFB_INDEXED) {
            printf("  palette (%" PRIu16 " entries):\n", tag->lfb.palette_size);
            for (uint16_t i = 0; i < tag->lfb.palette_size; i++) {
                printf("    r = %-3u, g = %-3u, b = %-3u\n",
                    tag->lfb.palette[i].red, tag->lfb.palette[i].green,
                    tag->lfb.palette[i].blue);
            }
        }

        break;
    default:
        printf("  type = %u (unknown)\n", tag->type);
        break;
    }
}

/** Print an IP address.
 * @param addr      Address to print.
 * @param flags     Behaviour flags. */
static void print_ip_addr(initium_ip_addr_t *addr, uint32_t flags) {
    if (flags & INITIUM_NET_IPV6) {
        printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
            addr->v6[0], addr->v6[1], addr->v6[2], addr->v6[3],
            addr->v6[4], addr->v6[5], addr->v6[6], addr->v6[7],
            addr->v6[8], addr->v6[9], addr->v6[10], addr->v6[11],
            addr->v6[12], addr->v6[13], addr->v6[14], addr->v6[15]);
    } else {
        printf("%u.%u.%u.%u\n", addr->v4[0], addr->v4[1], addr->v4[2], addr->v4[3]);
    }
}

/** Dump a boot device tag. */
static void dump_bootdev_tag(initium_tag_bootdev_t *tag) {
    printf("INITIUM_TAG_BOOTDEV:\n");

    switch (tag->type) {
    case INITIUM_BOOTDEV_NONE:
        printf("  type = %" PRIu32 " (INITIUM_BOOTDEV_NONE)\n", tag->type);
        break;
    case INITIUM_BOOTDEV_DISK:
        printf("  type          = %" PRIu32 " (INITIUM_BOOTDEV_DISK)\n", tag->type);
        printf("  flags         = 0x%" PRIx32 "\n", tag->disk.flags);
        printf("  uuid          = `%s'\n", tag->disk.uuid);
        printf("  device        = 0x%x\n", tag->disk.device);
        printf("  partition     = 0x%x\n", tag->disk.partition);
        printf("  sub_partition = 0x%x\n", tag->disk.sub_partition);
        break;
    case INITIUM_BOOTDEV_NET:
        printf("  type        = %" PRIu32 " (INITIUM_BOOTDEV_NET)\n", tag->type);
        printf("  flags       = 0x%" PRIx32 "\n", tag->net.flags);
        if (tag->net.flags & INITIUM_NET_IPV6)
            printf("    INITIUM_NET_IPV6\n");
        printf("  server_ip   = "); print_ip_addr(&tag->net.server_ip, tag->net.flags);
        printf("  server_port = %" PRIu16 "\n", tag->net.server_port);
        printf("  gateway_ip  = "); print_ip_addr(&tag->net.gateway_ip, tag->net.flags);
        printf("  client_ip   = "); print_ip_addr(&tag->net.client_ip, tag->net.flags);
        printf("  client_mac  = %02x:%02x:%02x:%02x:%02x:%02x\n",
            tag->net.client_mac[0], tag->net.client_mac[1],
            tag->net.client_mac[2], tag->net.client_mac[3],
            tag->net.client_mac[4], tag->net.client_mac[5]);
        printf("  hw_addr_len = %u\n", tag->net.hw_addr_len);
        printf("  hw_type     = %u\n", tag->net.hw_type);
        break;
    default:
        printf("  type = %" PRIu32 " (unknown)\n", tag->type);
        break;
    }
}

/** Dump a log tag. */
static void dump_log_tag(initium_tag_log_t *tag) {
    initium_log_t *log;

    printf("INITIUM_TAG_LOG:\n");
    printf("  log_virt  = 0x%" PRIx64 "\n", tag->log_virt);
    printf("  log_phys  = 0x%" PRIx64 "\n", tag->log_phys);
    printf("  log_size  = %" PRIu32 "\n", tag->log_size);
    printf("  prev_phys = 0x%" PRIx64 "\n", tag->prev_phys);
    printf("  prev_size = %" PRIu32 "\n", tag->prev_size);

    log = (initium_log_t *)((ptr_t)tag->log_virt);
    printf("  magic     = 0x%" PRIx32 "\n", log->magic);
}

/** Get a section by index.
 * @param tag           Tag to get from.
 * @param index         Index to get. */
static elf_shdr_t *find_elf_section(initium_tag_sections_t *tag, uint32_t index) {
    return (elf_shdr_t *)&tag->sections[index * tag->entsize];
}

/** Dump a sections tag. */
static void dump_sections_tag(initium_tag_sections_t *tag) {
    const char *strtab;
    elf_shdr_t *shdr;

    printf("INITIUM_TAG_SECTIONS:\n");
    printf("  num      = %" PRIu32 "\n", tag->num);
    printf("  entsize  = %" PRIu32 "\n", tag->entsize);
    printf("  shstrndx = %" PRIu32 "\n", tag->shstrndx);

    shdr = find_elf_section(tag, tag->shstrndx);
    strtab = (const char *)phys_to_virt(shdr->sh_addr);
    printf("  shstrtab = 0x%lx (%p)\n", shdr->sh_addr, strtab);

    for (uint32_t i = 0; i < tag->num; i++) {
        shdr = find_elf_section(tag, i);

        printf("  section %u (`%s'):\n", i, (shdr->sh_name) ? strtab + shdr->sh_name : "");
        printf("    sh_type  = %" PRIu32 "\n", shdr->sh_type);
        printf("    sh_flags = 0x%" PRIx32 "\n", shdr->sh_flags);
        printf("    sh_addr  = %p\n", shdr->sh_addr);
        printf("    sh_size  = %" PRIu32 "\n", shdr->sh_size);
    }
}

/** Get an E820 tag type. */
static const char *e820_tag_type(uint32_t type) {
    switch (type) {
    case 1:
        return "Free";
    case 2:
        return "Reserved";
    case 3:
        return "ACPI reclaimable";
    case 4:
        return "ACPI NVS";
    case 5:
        return "Bad";
    case 6:
        return "Disabled";
    default:
        return "???";
    }
}

/** Dump an E820 tag. */
static void dump_e820_tag(initium_tag_e820_t *tag) {
    printf("INITIUM_TAG_E820:\n");
    printf("  start  = 0x%" PRIx64 "\n", tag->start);
    printf("  length = 0x%" PRIx64 "\n", tag->length);
    printf("  type   = %" PRIu32 " (%s)\n", tag->type, e820_tag_type(tag->type));
}

/** Entry point of the test kernel.
 * @param magic         KBoot magic number.
 * @param tags          Tag list pointer. */
void kmain(uint32_t magic, initium_tag_t *tags) {
    if (magic != INITIUM_MAGIC) {
        while (true)
            arch_pause();
    }

    console_init(tags);
    log_init(tags);

    printf("Test kernel loaded: magic: 0x%x, tags: %p\n", magic, tags);

    while (tags->type != INITIUM_TAG_NONE) {
        switch (tags->type) {
        case INITIUM_TAG_CORE:
            dump_core_tag((initium_tag_core_t *)tags);
            break;
        case INITIUM_TAG_OPTION:
            dump_option_tag((initium_tag_option_t *)tags);
            break;
        case INITIUM_TAG_MEMORY:
            dump_memory_tag((initium_tag_memory_t *)tags);
            break;
        case INITIUM_TAG_VMEM:
            dump_vmem_tag((initium_tag_vmem_t *)tags);
            break;
        case INITIUM_TAG_PAGETABLES:
            dump_pagetables_tag((initium_tag_pagetables_t *)tags);
            break;
        case INITIUM_TAG_MODULE:
            dump_module_tag((initium_tag_module_t *)tags);
            break;
        case INITIUM_TAG_VIDEO:
            dump_video_tag((initium_tag_video_t *)tags);
            break;
        case INITIUM_TAG_BOOTDEV:
            dump_bootdev_tag((initium_tag_bootdev_t *)tags);
            break;
        case INITIUM_TAG_LOG:
            dump_log_tag((initium_tag_log_t *)tags);
            break;
        case INITIUM_TAG_SECTIONS:
            dump_sections_tag((initium_tag_sections_t *)tags);
            break;
        case INITIUM_TAG_E820:
            dump_e820_tag((initium_tag_e820_t *)tags);
            break;
        }

        tags = (initium_tag_t *)round_up((ptr_t)tags + tags->size, 8);
    }

    printf("Tag list dump complete\n");

    #if defined(__i386__) || defined(__x86_64__)
        __asm__ volatile("wbinvd");
    #endif

    while (true)
        arch_pause();
}
