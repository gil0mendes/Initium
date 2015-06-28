/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2015 Gil Mendes
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
 * @brief               Utility functions/macros.
 */

#ifndef __LIB_UTILITY_H
#define __LIB_UTILITY_H

#include <arch/bitops.h>

#include <types.h>

/** Get the number of bits in a type. */
#define bits(t)         (sizeof(t) * 8)

/** Get the number of elements in an array. */
#define array_size(a)   (sizeof((a)) / sizeof((a)[0]))

/** Round a value up.
 * @param val           Value to round.
 * @param nearest       Boundary to round up to.
 * @return              Rounded value. */
#define round_up(val, nearest) \
    __extension__ \
    ({ \
        typeof(val) __n = val; \
        if (__n % (nearest)) { \
            __n -= __n % (nearest); \
            __n += nearest; \
        } \
        __n; \
    })

/** Round a value down.
 * @param val           Value to round.
 * @param nearest       Boundary to round up to.
 * @return              Rounded value. */
#define round_down(val, nearest)    \
    __extension__ \
    ({ \
        typeof(val) __n = val; \
        if (__n % (nearest)) \
            __n -= __n % (nearest); \
        __n; \
    })

/** Check if a value is a power of 2.
 * @param val           Value to check.
 * @return              Whether value is a power of 2. */
#define is_pow2(val) \
    ((val) && ((val) & ((val) - 1)) == 0)

/** Get the lowest value out of a pair of values. */
#define min(a, b) \
    ((a) < (b) ? (a) : (b))

/** Get the highest value out of a pair of values. */
#define max(a, b) \
    ((a) < (b) ? (b) : (a))

/** Swap two values */
#define swap(a, b) \
{ \
    typeof(a) __tmp = a; \
    a = b; \
    b = __tmp; \
}

/** Calculate the absolute value of the given value. */
#define abs(val) \
    ((val) < 0 ? -(val) : (val))

/** Checksum a memory range.
 * @param start         Start of range to check.
 * @param size          Size of range to check.
 * @return              True if checksum is equal to 0, false if not. */
static inline bool checksum_range(void *start, size_t size) {
    uint8_t *range = (uint8_t *)start;
    uint8_t checksum = 0;
    size_t i;

    for (i = 0; i < size; i++)
        checksum += range[i];

    return (checksum == 0);
}

extern void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

#endif /* __LIB_UTILITY_H */
