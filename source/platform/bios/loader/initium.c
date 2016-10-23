/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Gil Mendes <gil00mendes@gmail.com>
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
#include <memory.h>

void initium_platform_setup(initium_loader_t *loader)
{
	void *buf __cleanup_free;
	size_t num_entries, entry_size, size;
	initium_tag_bios_e820_t *tag;

	// get a copy of the E820 memory map
	bios_memory_get_mmap(&buf, &num_entries, &entry_size);
	size = num_entries * entry_size;
	tag = initium_alloc_tag(loader, INITIUM_TAG_BIOS_E820, sizeof(*tag) + size);
	tag->num_entries = num_entries;
	tag->entry_size = entry_size;
	memcpy(tag->entries, buf, size);
}
