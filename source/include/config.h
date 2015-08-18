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
 * @brief               Configuration system.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <lib/list.h>

#include <console.h>

struct command;
struct device;
struct directory;
struct loader_ops;
struct value;

/** Structure containing an environment. */
typedef struct environ {
    /** Values set in the environment by the user. */
    list_t entries;

    /** Per-environment data used internally. */
    struct device *device;              /**< Current device. */
    struct fs_handle *directory;        /**< Current directory. */
    struct loader_ops *loader;          /**< Operating system loader operations. */
    void *loader_private;               /**< Data used by the loader. */
} environ_t;

/** Structure containing a list of commands. */
typedef list_t command_list_t;

/** Structure containing a list of values. */
typedef struct value_list {
    struct value *values;               /**< Array of values. */
    size_t count;                       /**< Number of arguments. */
} value_list_t;

/** Value type enumeration. */
typedef enum value_type {
    VALUE_TYPE_INTEGER,                 /**< Integer. */
    VALUE_TYPE_BOOLEAN,                 /**< Boolean. */
    VALUE_TYPE_STRING,                  /**< String. */
    VALUE_TYPE_LIST,                    /**< List. */
    VALUE_TYPE_COMMAND_LIST,            /**< Command list. */
    VALUE_TYPE_REFERENCE,               /**< Variable reference (not seen by commands). */
} value_type_t;

/** Structure containing a value used in the configuration.  */
typedef struct value {
    /** Type of the value. */
    value_type_t type;

    /** Storage for the value. */
    union {
        uint64_t integer;               /**< Integer. */
        bool boolean;                   /**< Boolean. */
        char *string;                   /**< String. */
        value_list_t *list;             /**< List. */
        command_list_t *cmds;           /**< Command list. */
    };
} value_t;

/** Structure describing a command. */
typedef struct command {
    const char *name;                   /**< Name of the command. */

    /** Execute the command.
     * @param args          List of arguments.
     * @return              Whether the command completed successfully. */
    bool (*func)(value_list_t *args);
} command_t;

/** Define a builtin command. */
#define BUILTIN_COMMAND(name, func) \
    static command_t __command_##func __used = { \
        name, \
        func \
    }; \
    DEFINE_BUILTIN(BUILTIN_TYPE_COMMAND, __command_##func)

/** Character returned from config_input_helper_t for end-of-file. */
#define EOF                 -1

/** Configuration reading helper function type.
 * @param nest          Nesting count, indicates whether the parser is currently
 *                      within a block that requires an end character. This is
 *                      used by the shell helper to determine whether to prompt
 *                      for another line.
 * @return              Character read, or EOF on end of file. */
typedef int (*config_read_helper_t)(unsigned nest);

extern console_t *config_console;
extern char *config_file_override;
extern environ_t *root_environ;
extern environ_t *current_environ;

#define config_vprintf(fmt, args) console_vprintf(config_console, fmt, args)
#define config_printf(fmt...) console_printf(config_console, fmt)

extern void config_error(const char *fmt, ...) __printf(1, 2);

extern void value_init(value_t *value, value_type_t type);
extern void value_destroy(value_t *value);
extern void value_copy(const value_t *source, value_t *dest);
extern void value_move(value_t *source, value_t *dest);

extern void value_list_destroy(value_list_t *list);
extern value_list_t *value_list_copy(const value_list_t *source);

extern environ_t *environ_create(environ_t *parent);
extern void environ_destroy(environ_t *env);
extern const value_t *environ_lookup(environ_t *env, const char *name);
extern const value_t *environ_insert(environ_t *env, const char *name, const value_t *value);
extern void environ_remove(environ_t *env, const char *name);
extern void environ_set_loader(environ_t *env, struct loader_ops *ops, void *private);
extern void environ_boot(environ_t *env) __noreturn;

extern void command_list_destroy(command_list_t *list);
extern command_list_t *command_list_copy(const command_list_t *source);
extern bool command_list_exec(command_list_t *list, environ_t *env);

extern command_list_t *config_parse(const char *path, config_read_helper_t helper);

extern void config_init(void);
extern void config_load(void);

#endif /* __CONFIG_H */
