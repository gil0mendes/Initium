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
 * @brief		Compiler-specific macros/definitions.
 */

#ifndef __COMPILER_H
#define __COMPILER_H

#ifdef __GNUC__
	#define __unused			__attribute__((unused))
	#define __used				__attribute__((used))
	#define __packed			__attribute__((packed))
	#define __aligned(a)		__attribute__((aligned(a)))
	#define __noreturn			__attribute__((noreturn))
	#define __malloc			__attribute__((malloc))
	#define __printf(a, b)		__attribute__((format(printf, a, b)))
	#define __deprecated		__attribute__((deprecated))
	#define __section(s)		__attribute__((section(s)))
	#define likely(x)			__builtin_expect(!!(x), 1)
	#define unlikely(x)			__builtin_expect(!!(x), 0)
#else
	#error "Initium does not currently support compilers other than GCC"
#endif

#define STRINGIFY(val)		#val
#define XSTRINGIFY(val)		STRINGIFY(val)

#define STATIC_ASSERT(cond)	\
	do { \
		struct __static_assert { \
			char static_assert_failed[(cond) ? 1 : -1]; \
		}; \
	} while(0);

#endif // __COMPILER_H
