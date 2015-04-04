/*
 * Copyright (C) 2015 Gil Mendes
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               MBR partition support.
 */

#ifndef __PARTITION_MBR_H
#define __PARTITION_MBR_H

#include <types.h>

/** MBR partition table signature. */
#define MBR_SIGNATURE 0xAA55

/** MBR partition description. */
typedef struct mbr_partition {
    uint8_t bootable;
    uint8_t start_head;
    uint8_t start_sector;
    uint8_t start_cylinder;
    uint8_t type;
    uint8_t end_head;
    uint8_t end_sector;
    uint8_t end_cylinder;
    uint32_t start_lba;
    uint32_t num_sectors;
} __packed mbr_partition_t;

/** MBR structure. */
typedef struct mbr {
    uint8_t bootcode[446];
    mbr_partition_t partitions[4];
    uint16_t signature;
} __packed mbr_t;

#endif /* __PARTITION_MBR_H */
