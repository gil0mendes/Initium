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
 * @brief             Status code definitions
 */

#ifndef __STATUS_H
#define __STATUS_H

/**
 * Status codes.
 *
 * Status codes used for reporting errors. When updating, be sure to update
 * the error string table in lib/printf.c.
 */
typedef enum status {
    STATUS_SUCCESS,                 /**< Operation completed successfully. */
    STATUS_NOT_SUPPORTED,           /**< Operation not supported. */
    STATUS_INVALID_ARG,             /**< Invalid argument. */
    STATUS_TIMED_OUT,               /**< Timed out while waiting. */
    STATUS_NO_MEMORY,               /**< Out of memory. */
    STATUS_NOT_DIR,                 /**< Not a directory. */
    STATUS_NOT_FILE,                /**< Not a regular file. */
    STATUS_NOT_FOUND,               /**< Not found. */
    STATUS_UNKNOWN_FS,              /**< Filesystem on device is unknown. */
    STATUS_CORRUPT_FS,              /**< Corruption detected on the filesystem. */
    STATUS_READ_ONLY,               /**< Filesystem is read only. */
    STATUS_END_OF_FILE,             /**< Read beyond end of file. */
    STATUS_SYMLINK_LIMIT,           /**< Exceeded nested symbolic link limit. */
    STATUS_DEVICE_ERROR,            /**< Device error. */
    STATUS_UNKNOWN_IMAGE,           /**< Image has an unrecognised format. */
    STATUS_MALFORMED_IMAGE,         /**< Image format is incorrect. */
    STATUS_SYSTEM_ERROR,            /**< Error from system firmware. */
} status_t;

#endif /* __STATUS_H */
