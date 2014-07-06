/**
* The MIT License (MIT)
*
* Copyright (c) 2014 Gil Mendes
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
* @brief 			Time handling function
*/

#ifndef __TIME_H
#define __TIME_H

#include <types.h>

// Type used to store a time value in microseconds
typedef int64_t timeout_t;

// Convert microseconds to seconds
#define USECS2SECS(secs)	(secs / 1000000)

// Convert seconds to microseconds
#define SECS2USECS(secs)	((useconds_t)secs * 1000000)

// Convert microseconds to milliseconds
#define USECS2MSECS(msecs)	(msecs / 1000)

// Convert milliseconds to microseconds
#define MSECS2USECS(msecs)	((useconds_t)msecs * 1000)

extern void spin(timeout_t us);

#endif // __TIME_H
