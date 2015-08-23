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
 * @brief               BIOS platform Iniitum loader functions.
 */

#include <bios/bios.h>
#include <bios/memory.h>

#include <loader/initium.h>

#include <assert.h>

void initium_platform_setup(initium_loader_t *loader) {
    bios_regs_t regs;
    uint32_t num_entries, entry_size;

    bios_regs_init(&regs);

    num_entries = 0;
    entry_size = 0;
    do {
	    regs.eax = 0xe820;
	    regs.edx = E820_SMAP;
	    regs.ecx = 64;
	    regs.edi = BIOS_MEM_BASE + (num_entries * entry_size);
	    bios_call(0x15, &regs);

	    if (regs.eflags & X86_FLAGS_CF) {
		    break;
		}

	    if (!num_entries) {
		    entry_size = regs.ecx;
		} else {
		    assert(entry_size == regs.ecx);
		}

	    num_entries++;
	} while (regs.ebx != 0);

    if (num_entries) {
        size_t size = num_entries * entry_size;
        initium_tag_bios_e820_t *tag = initium_alloc_tag(loader, INITIUM_TAG_BIOS_E820, sizeof(*tag) + size);

        tag->num_entries = num_entries;
        tag->entry_size = entry_size;

        memcpy(tag->entries, (void *)BIOS_MEM_BASE, size);
    }
}
