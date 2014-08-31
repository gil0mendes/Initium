/**
* The MIT License (MIT)
*
* Copyright (c) 2014 <author>
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
* @brief 					ISO9660 filesystem support
*/

#include <lib/ctype.h>
#include <lib/string.h>
#include <lib/utility.h>

#include <assert.h>
#include <endian.h>
#include <fs.h>
#include <loader.h>
#include <memory.h>

#include "iso9660.h"

/** Structure containing details of an ISO9660 filesystem. */
typedef struct iso9660_mount {
	int joliet_level;		/**< Joliet level. */
} iso9660_mount_t;

/** Structure containing details of an ISO9660 handle. */
typedef struct iso9660_handle {
	uint32_t data_len;		/**< Data length. */
	uint32_t extent;		/**< Extent block number. */
} iso9660_handle_t;

/** Convert a wide character to a multibyte sequence. */
static int utf8_wctomb(uint8_t *s, uint32_t wc, size_t max) {
	unsigned int bits = 0, j = 0, k;

	if(s == NULL) {
		return (wc >= 0x80);
	} else if(wc < 0x00000080) {
		*s = wc;
		return 1;
	}

	if(wc >= 0x04000000) {
		bits = 30;
		*s = 0xFC;
		j = 6;
	} else if(wc >= 0x00200000) {
		bits = 24;
		*s = 0xF8;
		j = 5;
	} else if(wc >= 0x00010000) {
		bits = 18;
		*s = 0xF0;
		j = 4;
	} else if(wc >= 0x00000800) {
		bits = 12;
		*s = 0xE0;
		j = 3;
	} else if(wc >= 0x00000080) {
		bits = 6;
		*s = 0xC0;
		j = 2;
	}

	if(j > max)
		return -1;

	*s |= (unsigned char)(wc >> bits);
	for(k = 1; k < j; k++) {
		bits -= 6;
		s[k] = 0x80 + ((wc >> bits) & 0x3f);
	}

	return k;
}

/** Convert big endian wide character string to UTF8. */
static int wcsntombs_be(uint8_t *s, uint8_t *pwcs, int inlen, int maxlen) {
	const uint8_t *ip;
	uint8_t *op;
	uint16_t c;
	int size;

	op = s;
	ip = pwcs;
	while((*ip || ip[1]) && (maxlen > 0) && (inlen > 0)) {
		c = (*ip << 8) | ip[1];
		if(c > 0x7f) {
			size = utf8_wctomb(op, c, maxlen);
			if(size == -1) {
				maxlen--;
			} else {
				op += size;
				maxlen -= size;
			}
		} else {
			*op++ = (uint8_t)c;
		}
		ip += 2;
		inlen--;
	}

	return op - s;
}

/** Parse a name from a directory record.
 * @param record	Record to parse.
 * @param buf		Buffer to write into. */
static void iso9660_parse_name(iso9660_directory_record_t *record, char *buf) {
	uint32_t i, len;

	len = (record->file_ident_len < ISO9660_MAX_NAME_LEN)
		? record->file_ident_len : ISO9660_MAX_NAME_LEN;

	for(i = 0; i < len; i++) {
		if(record->file_ident[i] == ISO9660_SEPARATOR2) {
			break;
		} else {
			buf[i] = tolower(record->file_ident[i]);
		}
	}

	if(i && buf[i - 1] == ISO9660_SEPARATOR1)
		i--;
	buf[i] = 0;
}

/** Parse a Joliet name from a directory record.
 * @param record	Record to parse.
 * @param buf		Buffer to write into. */
static void iso9660_parse_joliet_name(iso9660_directory_record_t *record, char *buf) {
	unsigned char len = wcsntombs_be(
		(uint8_t *)buf,
		record->file_ident,
		record->file_ident_len >> 1,
		ISO9660_NAME_SIZE);

	if((len > 2) && (buf[len - 2] == ';') && (buf[len - 1] == '1'))
		len -= 2;

	while(len >= 2 && (buf[len - 1] == '.'))
		len--;

	buf[len] = 0;
}

/** Generate a UUID.
 * @param pri		Primary volume descriptor.
 * @return		Pointer to allocated string for UUID. */
static char *iso9660_make_uuid(iso9660_primary_volume_desc_t *pri) {
	iso9660_timestamp_t *time;
	char *uuid;
	size_t i;

	/* If the modification time is set, then base the UUID off that, else
	 * use the creation time. The ISO9660 says that a date is unset if all
	 * the fields are '0' and the offset is 0. */
	time = &pri->vol_mod_time;
	for(i = 0; i < 16; i++) {
		if(((uint8_t *)time)[i] != 0)
			break;
	}
	if(i == 16 && time->offset == 0)
		time = &pri->vol_cre_time;

	/* Create the UUID string. */
	uuid = malloc(23);
	sprintf(uuid, "%c%c%c%c-%c%c-%c%c-%c%c-%c%c-%c%c-%c%c",
		time->year[0], time->year[1], time->year[2], time->year[3],
		time->month[0], time->month[1], time->day[0], time->day[1],
		time->hour[0], time->hour[1], time->minute[0], time->minute[1],
		time->second[0], time->second[1], time->centisecond[0],
		time->centisecond[1]);
	return uuid;
}

/** Create a handle from a directory record.
 * @param mount		Mount the node is from.
 * @param rec		Record to create from.
 * @return		Pointer to handle. */
static file_handle_t *iso9660_handle_create(mount_t *mount, iso9660_directory_record_t *rec) {
	iso9660_handle_t *data = malloc(sizeof(iso9660_handle_t));
	data->data_len = le32_to_cpu(rec->data_len_le);
	data->extent = le32_to_cpu(rec->extent_loc_le);
	return file_handle_create(mount, (rec->file_flags & (1<<1)), data);
}

/** Mount an ISO9660 filesystem.
 * @param mount		Mount structure to fill in.
 * @return		Whether the file contains the FS. */
static bool iso9660_mount(mount_t *mount) {
	iso9660_primary_volume_desc_t *pri = NULL;
	iso9660_supp_volume_desc_t *sup = NULL;
	iso9660_volume_desc_t *desc;
	iso9660_mount_t *data;
	int joliet = 0, i;
	bool ret = false;
	char *buf;

	/* Read in volume descriptors until we find the primary descriptor.
	 * I don't actually know whether there's a limit on the number of
	 * descriptors - I just put in a sane one so we don't loop for ages. */
	buf = malloc(ISO9660_BLOCK_SIZE);
	for(i = ISO9660_DATA_START; i < 128; i++) {
		if(!disk_read(mount->disk, buf, ISO9660_BLOCK_SIZE, i * ISO9660_BLOCK_SIZE))
			goto out;

		/* Check that the identifier is valid. */
		desc = (iso9660_volume_desc_t *)buf;
		if(strncmp((char *)desc->ident, "CD001", 5) != 0)
			goto out;

		if(desc->type == ISO9660_VOL_DESC_PRIMARY) {
			pri = malloc(sizeof(iso9660_primary_volume_desc_t));
			memcpy(pri, buf, sizeof(iso9660_primary_volume_desc_t));
		} else if(desc->type == ISO9660_VOL_DESC_SUPPLEMENTARY) {
			/* Determine whether Joliet is supported. */
			sup = (iso9660_supp_volume_desc_t *)desc;
			if(sup->esc_sequences[0] == 0x25 && sup->esc_sequences[1] == 0x2F) {
				if(sup->esc_sequences[2] == 0x40) {
					joliet = 1;
				} else if(sup->esc_sequences[2] == 0x43) {
					joliet = 2;
				} else if(sup->esc_sequences[2] == 0x45) {
					joliet = 3;
				} else {
					continue;
				}

				sup = malloc(sizeof(iso9660_supp_volume_desc_t));
				memcpy(sup, buf, sizeof(iso9660_supp_volume_desc_t));
			} else {
				sup = NULL;
			}
		} else if(desc->type == ISO9660_VOL_DESC_TERMINATOR) {
			break;
		}
	}

	/* Check whether a descriptor was found. */
	if(!pri)
		goto out;

	/* Store details of the filesystem in the mount structure. */
	data = mount->data = malloc(sizeof(iso9660_mount_t));
	data->joliet_level = joliet;

	/* Store the filesystem label and UUID. */
	pri->vol_ident[31] = 0;
	pri->sys_ident[31] = 0;
	mount->uuid = iso9660_make_uuid(pri);
	mount->label = strdup(strstrip((char *)pri->vol_ident));

	/* Retreive the root node. */
	if(joliet) {
		assert(sup);
		mount->root = iso9660_handle_create(mount,
			(iso9660_directory_record_t *)&sup->root_dir_record);
	} else {
		mount->root = iso9660_handle_create(mount,
			(iso9660_directory_record_t *)&pri->root_dir_record);
	}
	dprintf("iso9660: mounted %s (%s) (joliet: %d, uuid: %s)\n", mount->label,
		mount->disk->device.name, joliet, mount->uuid);
	ret = true;
out:
	if(pri) {
		free(pri);
	}
	if(sup) {
		free(sup);
	}
	free(buf);
	return ret;
}

/** Close an ISO9660 handle.
 * @param handle	Handle to close. */
static void iso9660_close(file_handle_t *handle) {
	free(handle->data);
}

/** Read from an ISO9660 handle.
 * @param handle	Handle to read from.
 * @param buf		Buffer to read into.
 * @param count		Number of bytes to read.
 * @param offset	Offset to read from.
 * @return		Whether the read succeeded. */
static bool iso9660_read(file_handle_t *handle, void *buf, size_t count, offset_t offset) {
	iso9660_handle_t *data = handle->data;

	if(!count || offset >= data->data_len) {
		return false;
	} else if((offset + count) > data->data_len) {
		return false;
	}

	return disk_read(handle->mount->disk, buf, count,
		(data->extent * ISO9660_BLOCK_SIZE) + offset);
}

/** Get the size of an ISO9660 file.
 * @param handle	Handle to the file.
 * @return		Size of the file. */
static offset_t iso9660_size(file_handle_t *handle) {
	iso9660_handle_t *data = handle->data;
	return data->data_len;
}

/** Iterate over directory entries.
 * @param handle	Handle to directory.
 * @param cb		Callback to call on each entry.
 * @param arg		Data to pass to callback.
 * @return		Whether read successfully. */
static bool iso9660_iterate(file_handle_t *handle, dir_iterate_cb_t cb, void *arg) {
	iso9660_mount_t *mount = handle->mount->data;
	iso9660_handle_t *data = handle->data;
	iso9660_directory_record_t *rec;
	char name[ISO9660_NAME_SIZE];
	uint32_t offset = 0;
	file_handle_t *child;
	char *buf;
	bool ret;

	/* Read in all the directory data. */
	buf = malloc(data->data_len);
	if(!iso9660_read(handle, buf, data->data_len, 0)) {
		free(buf);
		return false;
	}

	/* Iterate through each entry. */
	while(offset < data->data_len) {
		rec = (iso9660_directory_record_t *)(buf + offset);
		offset += rec->rec_len;

		if(rec->rec_len == 0) {
			/* A zero record length means we should move on to the
			 * next block. If this is the end, this will cause us
			 * to break out of the while loop if offset becomes >=
			 * data_len. */
			offset = round_up(offset, ISO9660_BLOCK_SIZE);
			continue;
		} else if(rec->file_flags & (1<<0)) {
			continue;
		} else if(rec->file_flags & (1<<1) && rec->file_ident_len == 1) {
			if(rec->file_ident[0] == 0 || rec->file_ident[0] == 1)
				continue;
		}

		/* Parse the name based on the Joliet level. */
		if(mount->joliet_level) {
			iso9660_parse_joliet_name(rec, name);
		} else {
			iso9660_parse_name(rec, name);
		}

		child = iso9660_handle_create(handle->mount, rec);
		ret = cb(name, child, arg);
		file_close(child);
		if(!ret)
			break;
	}

	free(buf);
	return true;
}

/** ISO9660 filesystem operations structure. */
BUILTIN_FS_TYPE(iso9660_fs_type) = {
	.mount = iso9660_mount,
	.close = iso9660_close,
	.read = iso9660_read,
	.size = iso9660_size,
	.iterate = iso9660_iterate,
};
