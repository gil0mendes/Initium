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
 * @brief 					Configuration system
 */

#include <lib/ctype.h>
#include <lib/string.h>
#include <lib/utility.h>

#include <assert.h>
#include <config.h>
#include <fs.h>
#include <loader.h>
#include <memory.h>
#include <menu.h>

/** Structure containing details of a command to run. */
typedef struct command_list_entry {
	list_t header;			/**< Link to command list. */
	char *name;			/**< Name of the command. */
	value_list_t *args;		/**< List of arguments. */
} command_list_entry_t;

/** Structure containing an environment entry. */
typedef struct environ_entry {
	list_t header;			/**< Link to environment. */
	char *name;			/**< Name of entry. */
	value_t value;			/**< Value of the entry. */
} environ_entry_t;

/** Length of the temporary buffer. */
#define TEMP_BUF_LEN		512

/** Character returned from get_next_char() for end-of-file. */
#define EOF			-1

static void value_list_destroy(value_list_t *list);
static value_list_t *value_list_copy(value_list_t *source);
static command_list_t *command_list_copy(command_list_t *source);
static void command_list_destroy(command_list_t *list);
static command_list_t *parse_command_list(int endch);

/** Temporary buffer to collect strings in. */
static char temp_buf[TEMP_BUF_LEN];
static size_t temp_buf_idx = 0;

/** Current parsing state. */
static const char *current_file;	/**< Pointer to data for current file. */
static const char *current_file_path;	/**< Current configuration file path. */
static size_t current_file_offset;	/**< Current offset in the file. */
static size_t current_file_size;	/**< Size of the current file. */
static int current_line;		/**< Current line in the file. */
static int current_col;			/**< Current column in the file (minus 1). */

/** Configuration file paths to try. */
static const char *config_file_paths[] = {
	"/system/boot/loader.cfg",
	"/boot/loader.cfg",
	"/loader.cfg",
};

/** Overridden configuration file path. */
char *config_file_override = NULL;

/** Root environment. */
environ_t *root_environ = NULL;

/** Current environment. */
environ_t *current_environ = NULL;

/** Read a character from the input file.
 * @return		Character read. */
static int get_next_char(void) {
	char ch;

	if(current_file_offset < current_file_size) {
		ch = current_file[current_file_offset++];
		if(ch == '\n') {
			current_line++;
			current_col = 0;
		} else if(ch == '\t') {
			current_col += 8 - (current_col % 8);
		} else {
			current_col++;
		}
		return ch;
	} else {
		return EOF;
	}
}

/** Rewind the input by a character. */
static void rewind_input(void) {
	if(current_file_offset > 0) {
		current_file_offset--;
		if(current_col > 0) {
			current_col--;
		} else if(current_line > 1) {
			current_line--;
		}
	}
}

/** Print details of a syntax error.
 * @param fmt		Message format.
 * @param ...		Arguments to substitute into format string. */
static void syntax_error(const char *fmt, ...) {
	va_list args;

	dprintf("%s:%d:%d: error: ", current_file_path, current_line, current_col);
	va_start(args, fmt);
	dvprintf(fmt, args);
	va_end(args);
	dprintf("\n");
}

/** Initialize a value to a default (empty) value.
 * @param value		Value to initialize.
 * @param type		Type of the value. */
void value_init(value_t *value, int type) {
	value->type = type;

	switch(type) {
	case VALUE_TYPE_INTEGER:
		value->integer = 0;
		break;
	case VALUE_TYPE_BOOLEAN:
		value->boolean = false;
		break;
	case VALUE_TYPE_STRING:
		value->string = strdup("");
		break;
	case VALUE_TYPE_LIST:
		value->list = malloc(sizeof(value_list_t));
		value->list->values = NULL;
		value->list->count = 0;
		break;
	case VALUE_TYPE_COMMAND_LIST:
		value->cmds = malloc(sizeof(command_list_t));
		list_init(value->cmds);
		break;
	default:
		assert(0 && "Setting invalid value type");
	}
}

/** Copy the contents of one value to another.
 * @param source	Source value.
 * @param dest		Destination value. */
void value_copy(const value_t *source, value_t *dest) {
	dest->type = source->type;

	switch(dest->type) {
	case VALUE_TYPE_INTEGER:
		dest->integer = source->integer;
		break;
	case VALUE_TYPE_BOOLEAN:
		dest->boolean = source->boolean;
		break;
	case VALUE_TYPE_STRING:
		dest->string = strdup(source->string);
		break;
	case VALUE_TYPE_LIST:
		dest->list = value_list_copy(source->list);
		break;
	case VALUE_TYPE_COMMAND_LIST:
		dest->cmds = command_list_copy(source->cmds);
		break;
	}
}

/** Destroy a value.
 * @param value		Value to destroy. */
void value_destroy(value_t *value) {
	switch(value->type) {
	case VALUE_TYPE_STRING:
		if(value->string)
			free(value->string);
		break;
	case VALUE_TYPE_LIST:
		if(value->list)
			value_list_destroy(value->list);
		break;
	case VALUE_TYPE_COMMAND_LIST:
		if(value->cmds)
			command_list_destroy(value->cmds);
		break;
	default:
		break;
	}
}

/** Copy a value list.
 * @param source	Source list.
 * @return		Pointer to destination list. */
static value_list_t *value_list_copy(value_list_t *source) {
	value_list_t *dest = malloc(sizeof(value_list_t));
	size_t i;

	dest->count = source->count;

	if(source->count) {
		dest->values = malloc(sizeof(value_t) * source->count);
		for(i = 0; i < source->count; i++)
			value_copy(&source->values[i], &dest->values[i]);
	} else {
		dest->values = NULL;
	}

	return dest;
}

/** Destroy an argument list.
 * @param list		List to destroy. */
static void value_list_destroy(value_list_t *list) {
	size_t i;

	for(i = 0; i < list->count; i++)
		value_destroy(&list->values[i]);

	free(list->values);
	free(list);
}

/** Copy a command list.
 * @param source	Source list.
 * @return		Pointer to destination list. */
static command_list_t *command_list_copy(command_list_t *source) {
	command_list_t *dest = malloc(sizeof(command_list_t));
	command_list_entry_t *entry, *copy;

	list_init(dest);

	list_foreach(source, iter) {
		entry = list_entry(iter, command_list_entry_t, header);

		copy = malloc(sizeof(command_list_entry_t));
		list_init(&copy->header);
		copy->name = strdup(entry->name);
		copy->args = value_list_copy(entry->args);
		list_append(dest, &copy->header);
	}

	return dest;
}

/** Destroy a command list.
 * @param list		List to destroy. */
static void command_list_destroy(command_list_t *list) {
	command_list_entry_t *command;

	list_foreach_safe(list, iter) {
		command = list_entry(iter, command_list_entry_t, header);

		list_remove(&command->header);
		if(command->args)
			value_list_destroy(command->args);
		free(command->name);
		free(command);
	}

	free(list);
}

/** Parse an integer.
 * @param intp		Where to store integer parsed.
 * @return		Whether successful. */
static bool parse_integer(uint64_t *intp) {
	int ch;

	while(true) {
		ch = get_next_char();
		if(isdigit(ch)) {
			temp_buf[temp_buf_idx++] = ch;
		} else {
			temp_buf[temp_buf_idx] = 0;
			*intp = strtoull(temp_buf, NULL, 0);
			temp_buf_idx = 0;
			return true;
		}
	}
}

/** Parse a string.
 * @return		Pointer to string on success, NULL on failure. */
static char *parse_string(void) {
	bool escaped = false;
	char *ret;
	int ch;

	while(true) {
		ch = get_next_char();
		if(ch == EOF) {
			syntax_error("unexpected EOF, expected end of string");
			return NULL;
		} else if(!escaped && ch == '"') {
			temp_buf[temp_buf_idx] = 0;
			ret = strdup(temp_buf);
			temp_buf_idx = 0;
			return ret;
		} else {
			if(ch == '\\' && !escaped) {
				escaped = true;
			} else {
				temp_buf[temp_buf_idx++] = ch;
				escaped = false;
			}
		}
	}
}

/** Parse an value list.
 * @param endch		End character for the list.
 * @return		Pointer to list on success, NULL on failure. */
static value_list_t *parse_value_list(int endch) {
	bool need_space = false;
	value_list_t *list;
	value_t *value;
	int ch;

	list = malloc(sizeof(value_list_t));
	list->values = NULL;
	list->count = 0;

	while(true) {
		ch = get_next_char();
		if(ch == endch) {
			return list;
		} else if(isspace(ch)) {
			need_space = false;
			continue;
		} else if(need_space) {
			syntax_error("expected space");
			goto fail;
		}

		/* Start of a new argument. */
		list->values = realloc(list->values, sizeof(value_t) * (list->count + 1));
		value = &list->values[list->count++];
		need_space = true;
		if(isdigit(ch)) {
			/* Integers are a bit dodgy. They have no special
			 * start/end character like everything else, so
			 * parse_integer() returns when it encounters anything
			 * that's not a digit. In this case we have to return
			 * to the previous character before continuing. */
			value->type = VALUE_TYPE_INTEGER;
			rewind_input();
			if(!parse_integer(&value->integer))
				goto fail;

			rewind_input();
		} else if(ch == 't') {
			value->type = VALUE_TYPE_BOOLEAN;
			value->boolean = true;
			if(get_next_char() != 'r' || get_next_char() != 'u' || get_next_char() != 'e') {
				syntax_error("unexpected character");
				goto fail;
			}
		} else if(ch == 'f') {
			value->type = VALUE_TYPE_BOOLEAN;
			value->boolean = false;
			if(get_next_char() != 'a' || get_next_char() != 'l' || get_next_char() != 's'
				|| get_next_char() != 'e')
			{
				syntax_error("unexpected character");
				goto fail;
			}
		} else if(ch == '"') {
			value->type = VALUE_TYPE_STRING;
			if(!(value->string = parse_string()))
				goto fail;
		} else if(ch == '[') {
			value->type = VALUE_TYPE_LIST;
			if(!(value->list = parse_value_list(']')))
				goto fail;
		} else if(ch == '{') {
			value->type = VALUE_TYPE_COMMAND_LIST;
			if(!(value->cmds = parse_command_list('}')))
				goto fail;
		}
	}
fail:
	value_list_destroy(list);
	return NULL;
}

/** Parse a command list.
 * @param endch		Character signalling the end of the list.
 * @return		Pointer to list on success, NULL on failure. */
static command_list_t *parse_command_list(int endch) {
	command_list_entry_t *command;
	bool in_comment = false;
	command_list_t *list;
	int ch;

	list = malloc(sizeof(command_list_t));
	list_init(list);

	while(true) {
		ch = get_next_char();
		if(in_comment) {
			if(ch == '\n')
				in_comment = false;
			continue;
		} else if(ch == endch || isspace(ch)) {
			if(temp_buf_idx == 0) {
				if(ch == endch) {
					return list;
				} else {
					continue;
				}
			}

			temp_buf[temp_buf_idx] = 0;
			if(strcmp(temp_buf, "#") == 0) {
				in_comment = true;
				temp_buf_idx = 0;
				continue;
			}

			/* End of command name, push it onto the list. */
			command = malloc(sizeof(command_list_entry_t));
			list_init(&command->header);
			command->name = strdup(temp_buf);
			list_append(list, &command->header);
			temp_buf_idx = 0;
			if(ch == '\n') {
				command->args = malloc(sizeof(value_list_t));
				command->args->values = NULL;
				command->args->count = 0;
			} else {
				if(!(command->args = parse_value_list('\n')))
					goto fail;
			}
		} else if(ch == EOF) {
			syntax_error("unexpected end of file");
			goto fail;
		} else {
			temp_buf[temp_buf_idx++] = ch;
			continue;
		}
	}
fail:
	command_list_destroy(list);
	return NULL;
}

/** Parse configuration data.
 * @param buf		Pointer to NULL-terminated buffer containing file data.
 * @param path		Path of the file (used in error output).
 * @return		Whether the file was loaded successfully. */
static bool config_parse(const char *buf, const char *path) {
	command_list_t *list;
	bool ret;

	current_file = buf;
	current_file_path = path;
	current_file_offset = 0;
	current_file_size = strlen(buf);
	current_line = 1;
	current_col = 0;

	if(!(list = parse_command_list(EOF)))
		return false;

	ret = command_list_exec(list, &root_environ);
	command_list_destroy(list);
	return ret;
}

/** Load a configuration file.
 * @param path		Path of the file.
 * @return		Whether the file was loaded successfully. */
static bool config_load(const char *path) {
	file_handle_t *handle;
	size_t size;
	char *buf;
	bool ret;

	if(!(handle = file_open(path, NULL)))
		return false;

	size = file_size(handle);
	buf = malloc(size + 1);
	if(!file_read(handle, buf, size, 0)) {
		free(buf);
		file_close(handle);
		return false;
	}
	buf[size] = 0;

	ret = config_parse(buf, path);
	free(buf);
	file_close(handle);
	return ret;
}

/** Execute a single command from a command list.
 * @param entry		Entry to execute.
 * @return		Whether successful. */
static bool command_exec(command_list_entry_t *entry) {
	BUILTIN_ITERATE(BUILTIN_TYPE_COMMAND, command_t, command) {
		if(strcmp(command->name, entry->name) == 0)
			return command->func(entry->args);
	}

	dprintf("unknown command '%s'\n", entry->name);
	return false;
}

/**
 * Execute a command list.
 *
 * Executes all the commands contained in a command list. A new environment
 * will be created with its parent as the current environment to execute the
 * commands under.
 *
 * @param list		List of commands.
 * @param envp		Where to store pointer to environment commands were
 *			executed under upon success. If this is non-NULL, it
 *			is the caller's responsibility to destroy the
 *			environment when it is no longer needed.
 *
 * @return		Whether all of the commands completed successfully.
 */
bool command_list_exec(command_list_t *list, environ_t **envp) {
	command_list_entry_t *entry;
	environ_t *env, *prev;

	/* Create a new environment, and set it as the current. */
	env = environ_create(current_environ);
	prev = current_environ;
	current_environ = env;

	/* Set envp here just so that root_environ is set when we're executing
	 * the commands. */
	if(envp)
		*envp = env;

	list_foreach(list, iter) {
		entry = list_entry(iter, command_list_entry_t, header);
		if(!command_exec(entry)) {
			current_environ = prev;
			environ_destroy(env);
			return false;
		}
	}

	/* Restore the previous environment if not NULL. This has the effect of
	 * keeping current_environ set to the root environment when we finish
	 * executing the top level configuration. */
	if(prev)
		current_environ = prev;

	/* Destroy the environment if it's no longer wanted. */
	if(!envp)
		environ_destroy(env);

	return true;
}

/** Create a new environment.
 * @param parent	Parent environment.
 * @return		Pointer to created environment. */
environ_t *environ_create(environ_t *parent) {
	environ_t *env = malloc(sizeof(environ_t));

	list_init(&env->entries);
	env->parent = parent;
	//env->device = boot_device;
	env->loader = NULL;
	env->data = NULL;
	return env;
}

/**
 * Look up an entry in an environment.
 *
 * Looks up an entry by name in an environment. If the entry is not found, it
 * will be recursively looked up in the parent environment, until the root
 * environment is reached.
 *
 * @param env		Environment to look up in.
 * @param name		Name of entry to look up.
 *
 * @return		Pointer to value if found, NULL if not.
 */
value_t *environ_lookup(environ_t *env, const char *name) {
	environ_entry_t *entry;

	list_foreach(&env->entries, iter) {
		entry = list_entry(iter, environ_entry_t, header);
		if(strcmp(entry->name, name) == 0)
			return &entry->value;
	}

	return (env->parent) ? environ_lookup(env->parent, name) : NULL;
}

/** Insert an entry into an environment.
 * @param env		Environment to insert into.
 * @param name		Name of entry to look up.
 * @param value		Value to insert. Will be copied.
 * @return		Pointer to inserted value. */
value_t *environ_insert(environ_t *env, const char *name, value_t *value) {
	environ_entry_t *entry;

	/* Look for an existing entry with the same name. */
	list_foreach(&env->entries, iter) {
		entry = list_entry(iter, environ_entry_t, header);
		if(strcmp(entry->name, name) == 0) {
			value_destroy(&entry->value);
			value_copy(value, &entry->value);
			return &entry->value;
		}
	}

	/* Create a new entry. */
	entry = malloc(sizeof(environ_entry_t));
	list_init(&entry->header);
	entry->name = strdup(name);
	value_copy(value, &entry->value);
	list_append(&env->entries, &entry->header);

	return &entry->value;
}

/** Remove an entry from an environment.
 * @param env		Environment to remove from.
 * @param name		Name of entry to remove. */
void environ_remove(environ_t *env, const char *name) {
	environ_entry_t *entry;

	/* Look for an existing entry with the same name. */
	list_foreach(&env->entries, iter) {
		entry = list_entry(iter, environ_entry_t, header);
		if(strcmp(entry->name, name) == 0) {
			list_remove(&entry->header);
			value_destroy(&entry->value);
			free(entry->name);
			free(entry);
			return;
		}
	}
}

/** Destroy an environment.
 * @param env		Environment to destroy. */
void environ_destroy(environ_t *env) {
	environ_entry_t *entry;

	list_foreach_safe(&env->entries, iter) {
		entry = list_entry(iter, environ_entry_t, header);

		list_remove(&entry->header);
		free(entry->name);
		value_destroy(&entry->value);
		free(entry);
	}

	free(env);
}

/** Set a value in the environment.
 * @param args		Argument list.
 * @return		Whether successful. */
static bool config_cmd_set(value_list_t *args) {
	if(args->count != 2 || args->values[0].type != VALUE_TYPE_STRING) {
		dprintf("config: set: invalid arguments\n");
		return false;
	}

	/* We make the environment immutable after the loader type has been
	 * set as the loaders ensure that all the environment variables they
	 * require exist and are the expected type when executing the command.
	 * Disallowing changes after the command has been run means it is not
	 * necessary to check everything all over again. */
	if(current_environ->loader) {
		dprintf("config: set: environment immutable after loader has been set\n");
		return false;
	}

	environ_insert(current_environ, args->values[0].string, &args->values[1]);
	return true;
}

BUILTIN_COMMAND("set", config_cmd_set);

/** Set up the configuration system and load the configuration file. */
void config_init(void) {
	size_t i;

	if(config_file_override) {
		if(!config_load(config_file_override))
			boot_error("Specified configuration file does not exist");
	} else {
		/* Try the various paths. */
		for(i = 0; i < array_size(config_file_paths); i++) {
			if(config_load(config_file_paths[i]))
				return;
		}

		boot_error("Could not load configuration file");
	}
}
