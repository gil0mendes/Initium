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
  * @brief               Initium loader internal definitions.
  */

 #ifndef __LOADER_INITIUM_H
 #define __LOADER_INITIUM_H

 #include <lib/allocator.h>
 #include <lib/list.h>

 #include <fs.h>
 #include <initium.h>
 #include <mmu.h>

 /** Image tag header structure. */
 typedef struct initium_itag {
     list_t header;                      /**< Link to image tag list. */

     uint32_t type;                      /**< Type of the tag. */
     uint64_t data[];                    /**< Tag data. */
 } initium_itag_t;

 /** Description of a module to load. */
 typedef struct initium_module {
     list_t header;                      /**< Link to module list. */

     fs_handle_t *handle;                /**< Handle to module. */
     char *name;                         /**< Base name of module. */
 } initium_module_t;

 /** Structure describing a virtual memory mapping. */
 typedef struct initium_mapping {
     list_t header;                         /**< Link to virtual mapping list. */

     initium_vaddr_t start;                /**< Start of the virtual memory range. */
     initium_vaddr_t size;                 /**< Size of the virtual memory range. */
     initium_paddr_t phys;                 /**< Physical address that this range maps to. */
 } initium_mapping_t;

 /** Structure containing Initium loader data. */
 typedef struct initium_loader {
     /** Details obtained by configuration command. */
     fs_handle_t *handle;                /**< Handle to kernel image. */
     void *ehdr;                         /**< ELF header. */
     void *phdrs;                        /**< ELF program headers. */
     load_mode_t mode;                   /**< Whether the kernel is 32- or 64-bit. */
     list_t itags;                       /**< Image tags. */
     initium_itag_image_t *image;        /**< Main image tag. */
     list_t modules;                     /**< Modules to load. */
     const char *path;                   /**< Path to kernel image (only valid during command). */
     bool success;                       /**< Success flag used during iteration functions. */

     /** State used by the main loader. */
     initium_tag_core_t *core;           /**< Core image tag (also head of the tag list). */
     initium_itag_load_t *load;          /**< Load image tag. */
     mmu_context_t *mmu;                 /**< MMU context for the kernel. */
     allocator_t allocator;              /**< Virtual address space allocator. */
     list_t mappings;                    /**< Virtual mapping information. */
     load_ptr_t entry;                   /**< Kernel entry point address. */
     load_ptr_t tags_virt;               /**< Virtual address of tag list. */
     mmu_context_t *trampoline_mmu;      /**< Kernel trampoline address space. */
     phys_ptr_t trampoline_phys;         /**< Page containing kernel entry trampoline. */
     load_ptr_t trampoline_virt;         /**< Virtual address of trampoline page. */
 } initium_loader_t;

 extern void *initium_find_itag(initium_loader_t *loader, uint32_t type);
 extern void *initium_next_itag(initium_loader_t *loader, void *data);

 extern void *initium_alloc_tag(initium_loader_t *loader, uint32_t type, size_t size);

 extern initium_vaddr_t initium_alloc_virtual(initium_loader_t *loader, initium_paddr_t phys, initium_vaddr_t size);
 extern void initium_map_virtual(initium_loader_t *loader, initium_vaddr_t addr, initium_paddr_t phys, initium_vaddr_t size);

 /** Iterate over all tags of a certain type in the image tag list. */
 #define initium_itag_foreach(_loader, _type, _vtype, _vname) \
     for (_vtype *_vname = initium_find_itag(_loader, _type); _vname; _vname = initium_next_itag(_loader, _vname))

 extern void initium_arch_check_kernel(initium_loader_t *loader);
 extern void initium_arch_check_load_params(initium_loader_t *loader, initium_itag_load_t *load);
 extern void initium_arch_setup(initium_loader_t *loader);
 extern void initium_arch_enter(initium_loader_t *loader) __noreturn;

 extern void initium_platform_setup(initium_loader_t *loader);

 #endif /* __LOADER_INITIUM_H */
