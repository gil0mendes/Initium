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
* @brief 				COnfiguration system
*/

#ifndef __CONFIG_H
#define __CONFIG_H

#include <lib/list.h>

#include <fs.h>
#include <loader.h>

// Strucuture containing an environment
typedef struct environ {
	struct environ *parent; 			// parent environment

	// value set in the environment by the user
	list_t entries;

	// per-environment data used internally
	struct device *device;			// current device
	struct loader_type *loader; 	// operating system loader type
	void *data;										// data used by the loader
} environ_t;

// strucuture containing a list of commands
typedef list_t command_list_t;

// structure containing a list of values
typedef struct value_list {
	struct value *values;			// Arrays of values
	size_t count;							// Number of arguments
} value_list_t;

// structure containing a list of values
typedef struct value {
	// type of the value
	enum {
		// types that can be set from the configuration file
		VALUE_TYPE_INTEGER,				// Integer
		VALUE_TYPE_BOOLEAN,				// Boolean
		VALUE_TYPE_STRING,				// String
		VALUE_TYPE_LIST,					// List
		VALUE_TYPE_COMMAND_LIST,	// Command list
	} type;

	// Actual value
	union {
		uint64_t integer;				// Integer
		bool boolean;						// Boolean
		char *string;						// String
		value_list_t *list;			// List
		command_list_t *cmds;		// Command list
	};
} value_t;

// Structure describing a command that can be used in a command list.
typedef struct command {
	const char *name;		// Name of the command

	/**
	 * Execute the command.
	 *
	 * @param args		List of arguments.
	 * @return		Whether the command completed successfully.
	 */
	bool (*func)(value_list_t *args);
} command_t;

// Define a command, to be automatically added to the command list
#define BUILTIN_COMMAND(name, func) \
	static command_t __command_##func __used = { \
		name, \
		func \
	}; \
	DEFINE_BUILTIN(BUILTIN_TYPE_COMMAND, __command_##func)

extern char *config_file_override;
extern environ_t *root_environ;
extern environ_t *current_environ;

extern void value_init(value_t *value, int type);
extern void value_copy(const value_t *source, value_t *dest);
extern void value_destroy(value_t *value);

extern bool command_list_exec(command_list_t *list, environ_t **envp);

extern environ_t *environ_create(environ_t *parent);
extern value_t *environ_lookup(environ_t *env, const char *name);
extern value_t *environ_insert(environ_t *env, const char *name, value_t *value);
extern void environ_remove(environ_t *env, const char *name);
extern void environ_destroy(environ_t *env);

extern void config_init(void);
#endif // __CONFIG_H
