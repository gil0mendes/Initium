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
 * @brief		Character type functions.
 */

#ifndef __LIB_CTYPE_H
#define __LIB_CTYPE_H

/** Test if a character is lower-case.
 * @param ch		Character to test.
 * @return		Non-zero if is lower-case, zero if not. */
static inline int islower(int ch) {
	return (ch >= 'a' && ch <= 'z');
}

/** Test if a character is upper-case.
 * @param ch		Character to test.
 * @return		Non-zero if is upper-case, zero if not. */
static inline int isupper(int ch) {
	return (ch >= 'A' && ch <= 'Z');
}

/** Test if a character is alphabetic.
 * @param ch		Character to test.
 * @return		Non-zero if is alphabetic, zero if not. */
static inline int isalpha(int ch) {
	return islower(ch) || isupper(ch);
}

/** Test if a character is a digit.
 * @param ch		Character to test.
 * @return		Non-zero if is digit, zero if not. */
static inline int isdigit(int ch) {
	return (ch >= '0' && ch <= '9');
}

/** Test if a character is alphanumeric.
 * @param ch		Character to test.
 * @return		Non-zero if is alpha-numeric, zero if not. */
static inline int isalnum(int ch) {
	return (isalpha(ch) || isdigit(ch));
}

/** Test if a character is an ASCII character.
 * @param ch		Character to test.
 * @return		Non-zero if is ASCII, zero if not. */
static inline int isascii(int ch) {
	return ((unsigned int)ch < 128u);
}

/** Test if a character is blank.
 * @param ch		Character to test.
 * @return		Non-zero if is blank space, zero if not. */
static inline int isblank(int ch) {
	return (ch == ' ' || ch == '\t');
}

/** Test if a character is a control character.
 * @param ch		Character to test.
 * @return		Non-zero if is control character, zero if not. */
static inline int iscntrl(int ch) {
	return ((unsigned int)ch < 32u || ch == 127);
}

/** Test if a character is a printable character.
 * @param ch		Character to test.
 * @return		Non-zero if is printable, zero if not. */
static inline int isprint(int ch) {
	ch &= 0x7F;
	return (ch >= 0x20 && ch < 0x7F);
}

/** Check for any printable character except space.
 * @param ch		Character to test.
 * @return		Non-zero if check passed, zero if not. */
static inline int isgraph(int ch) {
	if(ch == ' ') {
		return 0;
	}
	return isprint(ch);
}

/** Test if a character is space.
 * @param ch		Character to test.
 * @return		Non-zero if is whitespace, zero if not. */
static inline int isspace(int ch) {
	return (ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f'
		|| ch == '\r' || ch == ' ');
}

/** Test if a character is punctuation.
 * @param ch		Character to test.
 * @return		Non-zero if is punctuation, zero if not. */
static inline int ispunct(int ch) {
	return (isprint(ch) && !isalnum(ch) && !isspace(ch));
}

/** Test if a character is a hexadecimal digit.
 * @param ch		Character to test.
 * @return		Non-zero if is alphabetic, zero if not. */
static inline int isxdigit(int ch) {
	return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f')
		|| (ch >= 'A' && ch <= 'F');
}

/** Convert a character to ASCII.
 * @param ch		Character to convert.
 * @return		Converted value. */
static inline int toascii(int ch) {
	return (ch & 0x7F);
}

/** Convert a character to lower-case.
 * @param ch		Character to convert.
 * @return		Converted character. */
static inline int tolower(int ch) {
	if(isalpha(ch)) {
		return ch | 0x20;
	} else {
		return ch;
	}
}

/** Convert a character to upper-case.
 * @param ch		Character to convert.
 * @return		Converted character. */
static inline int toupper(int ch) {
	if(isalpha(ch)) {
		return ch & ~0x20;
	} else {
		return ch;
	}
}

#define _tolower(ch)	((ch) | 0x20)
#define _toupper(ch)	((ch) & ~0x20)

#endif /* __LIB_CTYPE_H */
