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
 * @brief               Line editor.
 */

 #ifndef __LIB_LINE_EDITOR_H
 #define __LIB_LINE_EDITOR_H

 #include <types.h>

struct console;

/** Line editor state. */
typedef struct line_editor {
        struct console *console;         /**< Console to output to. */
        char *buf;                       /**< String being edited. */
        size_t len;                      /**< Current string length. */
        size_t offset;                   /**< Current string offset. */
} line_editor_t;

extern void line_editor_init(line_editor_t *editor, struct console *console, const char *str);
extern void line_editor_output(line_editor_t *editor);
extern void line_editor_input(line_editor_t *editor, uint16_t key);
extern char *line_editor_finish(line_editor_t *editor);
extern void line_editor_destroy(line_editor_t *editor);

 #endif /* __LIB_LINE_EDITOR_H */
