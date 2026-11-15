/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file CApi.h

File Description:
##  C API of utils::cli::Cli (C & C++ compatible)
##  Every function that can fail returns UTILS_OK / UTILS_KO (or NULL / 0), details in utils_exception (utils_exception.exception: failure of the last call)
##  Every callback returns UTILS_OK, UTILS_KO (info: utils_fail / utils_exception_set) or C_API_CLI_EXIT
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (CLI_CAPI_H is the guard of CApi.hpp)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef CLI_CAPI_C_H
    #define CLI_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../exception/CApi.h"  // utils_exception, utils_fail, UTILS_OK, UTILS_KO
    #include <stdbool.h>            // bool
    #include <stddef.h>             // size_t
    #include <stdint.h>             // uint8_t, int16_t, uint32_t

    //----------------------------------------------------------------//
    /* DEFINE */

    /* callbacks return (with UTILS_OK / UTILS_KO) */
    #define C_API_CLI_EXIT 1     // stop the cli (utils::exception::NoneException(Exit), like the exit command)
    #define C_API_CLI_NO_INPUT 2 // getc hook only: no char available yet (called again)

    /* commands arguments limit */
    #define C_API_CLI_NO_LIMIT -1 // no min / max number of arguments

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

/* opaque types */
typedef struct utils_cli utils_cli_t; // utils::cli::Cli (+ its thread)
typedef struct utils_cli_parsed utils_cli_parsed_t; // utils::cli::ParsedData

/* commands */
typedef int (*utils_cli_parsed_command_t)(const utils_cli_t* cli, size_t argc, const char* const* argv, void* data); // argv[0] = the command
typedef int (*utils_cli_raw_command_t)(const utils_cli_t* cli, const char* input, void* data);

/* hooks */
typedef int (*utils_cli_prompt_hook_t)(const utils_cli_t* cli, uint8_t code, void* data); // print the prompt
typedef int (*utils_cli_parser_hook_t)(const char* input, bool trim, bool logic, bool parse, utils_cli_parsed_t* out, void* data); // fill out (empty at the call)
typedef int (*utils_cli_getc_hook_t)(char* c, void* data); // UTILS_OK: *c read ('\0' = EOF), C_API_CLI_NO_INPUT: nothing yet

/* middlewares */
typedef int (*utils_cli_middleware_t)(void* data);
typedef int (*utils_cli_middleware_code_t)(uint8_t code, void* data);
typedef int (*utils_cli_middleware_char_t)(char c, void* data);
typedef int (*utils_cli_middleware_str_t)(const char* str, void* data);
typedef int (*utils_cli_middleware_parsed_t)(const utils_cli_parsed_t* parsed, void* data);

//----------------------------------------------------------------//
/* ENUM */

/* flags (utils::cli::Flag), a combination is given as uint32_t */
typedef enum utils_cli_flag {
    C_API_CLI_FLAG_DEBUG           = 1 << 0, // Active verbose for internal action (Nothing for now)
    C_API_CLI_FLAG_NOECHO          = 1 << 1, // Disable echo of the input
    C_API_CLI_FLAG_CATCH           = 1 << 2, // Enable error catching on execution
    C_API_CLI_FLAG_EMPTY_INPUT     = 1 << 3, // Ignore empty input (default: error)
    C_API_CLI_FLAG_TRIM            = 1 << 4, // Enable trim on input
    C_API_CLI_FLAG_PARSED          = 1 << 5, // Active parser for the input and call the parsed commands (default: raw)
    C_API_CLI_FLAG_PROMPT          = 1 << 6, // Active the prompt
    C_API_CLI_FLAG_LOGIC           = 1 << 7, // Enable logic with '&&', '||' and ';'
    C_API_CLI_FLAG_ARROW           = 1 << 8, // Activate left, right
    C_API_CLI_FLAG_HISTORY         = 1 << 9, // Activate history, up and down arrow
    C_API_CLI_FLAG_PERSISTENT      = 1 << 10, // Allow persistent memory between session (history, ...)
    C_API_CLI_FLAG_HINT            = 1 << 11, // Display hint when a command fail
    C_API_CLI_FLAG_AUTO_COMPLETION = 1 << 12, // Active auto completion with `\t` (only work on the first command for now)
    C_API_CLI_FLAG_MANUAL          = 1 << 13, // Enable manual call for each new input handling
    C_API_CLI_FLAG_THREAD          = 1 << 14, // Run in a thread (kept in the handle, see utils_cli_join)
    C_API_CLI_FLAG_DETACHED        = 1 << 15, // Detach the thread execution
    C_API_CLI_FLAG_NO_TTY          = 1 << 16 // Allow usage even without tty
} utils_cli_flag_t;

/* flags preset (utils::cli::flags::*) */
typedef enum utils_cli_flags {
    C_API_CLI_FLAGS_ALL             = C_API_CLI_FLAG_DEBUG | C_API_CLI_FLAG_CATCH | C_API_CLI_FLAG_NOECHO | C_API_CLI_FLAG_TRIM | C_API_CLI_FLAG_EMPTY_INPUT | C_API_CLI_FLAG_PARSED | C_API_CLI_FLAG_PROMPT | C_API_CLI_FLAG_LOGIC | C_API_CLI_FLAG_ARROW | C_API_CLI_FLAG_HISTORY | C_API_CLI_FLAG_HINT | C_API_CLI_FLAG_AUTO_COMPLETION | C_API_CLI_FLAG_MANUAL | C_API_CLI_FLAG_THREAD | C_API_CLI_FLAG_DETACHED,
    C_API_CLI_FLAGS_DEFAULT         = C_API_CLI_FLAG_CATCH | C_API_CLI_FLAG_EMPTY_INPUT | C_API_CLI_FLAG_TRIM | C_API_CLI_FLAG_PROMPT | C_API_CLI_FLAG_ARROW,
    C_API_CLI_FLAGS_DUMB            = 0,
    C_API_CLI_FLAGS_TERM1           = C_API_CLI_FLAG_CATCH | C_API_CLI_FLAG_EMPTY_INPUT | C_API_CLI_FLAG_TRIM | C_API_CLI_FLAG_PARSED | C_API_CLI_FLAG_PROMPT | C_API_CLI_FLAG_LOGIC | C_API_CLI_FLAG_ARROW | C_API_CLI_FLAG_HISTORY,
    C_API_CLI_FLAGS_TERM2           = C_API_CLI_FLAGS_TERM1 | C_API_CLI_FLAG_HINT | C_API_CLI_FLAG_AUTO_COMPLETION,
    C_API_CLI_FLAGS_TERM3           = C_API_CLI_FLAGS_TERM2 | C_API_CLI_FLAG_THREAD,
    C_API_CLI_FLAGS_LOG             = C_API_CLI_FLAGS_TERM3 | C_API_CLI_FLAG_DETACHED | C_API_CLI_FLAG_NO_TTY,
    C_API_CLI_FLAGS_DEV             = C_API_CLI_FLAGS_TERM2 | C_API_CLI_FLAG_DEBUG,
    C_API_CLI_FLAGS_MULTI_THREADING = C_API_CLI_FLAG_THREAD | C_API_CLI_FLAG_DETACHED
} utils_cli_flags_t;

//----------------------------------------------------------------//
/* STRUCT */

/* entries for utils_cli_set_parsed_commands */
typedef struct utils_cli_parsed_command_entry {
    const char* name;
    utils_cli_parsed_command_t fn;
    int16_t min; // C_API_CLI_NO_LIMIT: no minimum of arguments
    int16_t max; // C_API_CLI_NO_LIMIT: no maximum of arguments
    void* data; // given back to fn
} utils_cli_parsed_command_entry_t;

/* entries for utils_cli_set_raw_commands */
typedef struct utils_cli_raw_command_entry {
    const char* name;
    utils_cli_raw_command_t fn;
    void* data; // given back to fn
} utils_cli_raw_command_entry_t;

//----------------------------------------------------------------//
/* PROTOTYPE */

/* handle */
utils_cli_t* utils_cli_create(bool sig); // NULL on error, sig: catch of ctrl-c & ctrl-z
void utils_cli_destroy(utils_cli_t* cli); // kill & join a running cli

/* run */
int utils_cli_start(utils_cli_t* cli, const char* const* inputs, size_t count, size_t call); // inputs executed on start (NULL / 0: none)
void utils_cli_join(utils_cli_t* cli); // join the thread kept in the handle (THREAD) or wait the end of the cli
void utils_cli_interrupt(utils_cli_t* cli);
void utils_cli_kill(utils_cli_t* cli); // can't be undone

/* status */
bool utils_cli_is_running(const utils_cli_t* cli);
bool utils_cli_was_interrupted(const utils_cli_t* cli);
bool utils_cli_was_killed(const utils_cli_t* cli);
bool utils_cli_was_stopped(const utils_cli_t* cli);

/* flags */
void utils_cli_reset_flags(utils_cli_t* cli);
void utils_cli_sub_flags(utils_cli_t* cli, uint32_t flags);
void utils_cli_add_flags(utils_cli_t* cli, uint32_t flags);
void utils_cli_set_flags(utils_cli_t* cli, uint32_t flags);

/* commands */
int utils_cli_set_parsed_command(utils_cli_t* cli, const char* name, utils_cli_parsed_command_t fn, int16_t min, int16_t max, void* data, bool force); // force: override an existing one
int utils_cli_set_raw_command(utils_cli_t* cli, const char* name, utils_cli_raw_command_t fn, void* data, bool force);
int utils_cli_set_parsed_commands(utils_cli_t* cli, const utils_cli_parsed_command_entry_t* commands, size_t count, bool force);
int utils_cli_set_raw_commands(utils_cli_t* cli, const utils_cli_raw_command_entry_t* commands, size_t count, bool force);
int utils_cli_del_command(utils_cli_t* cli, const char* name);
int utils_cli_del_commands(utils_cli_t* cli, const char* const* names, size_t count);
int utils_cli_reset_commands(utils_cli_t* cli); // default commands (exit, ...)
int utils_cli_clear_commands(utils_cli_t* cli);

/* hooks */
int utils_cli_reset_hooks(utils_cli_t* cli);
int utils_cli_reset_prompt_hook(utils_cli_t* cli);
int utils_cli_reset_parser_hook(utils_cli_t* cli);
int utils_cli_reset_getc_hook(utils_cli_t* cli);
int utils_cli_set_prompt_hook(utils_cli_t* cli, utils_cli_prompt_hook_t hook, void* data);
int utils_cli_set_parser_hook(utils_cli_t* cli, utils_cli_parser_hook_t hook, void* data);
int utils_cli_set_getc_hook(utils_cli_t* cli, utils_cli_getc_hook_t hook, void* data);

/* default hooks (same signature as the hooks, data unused) */
int utils_cli_default_prompt_hook(const utils_cli_t* cli, uint8_t code, void* data);
int utils_cli_default_parser_hook(const char* input, bool trim, bool logic, bool parse, utils_cli_parsed_t* out, void* data);
int utils_cli_default_getc_hook(char* c, void* data);

/* middlewares (before / after of each step, called in the order of addition) */
int utils_cli_reset_middlewares(utils_cli_t* cli);
int utils_cli_cli_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_t fn, void* data); // When the cli start & end
int utils_cli_cli_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_t fn, void* data);
int utils_cli_cli_middlewares_clear(utils_cli_t* cli);
int utils_cli_error_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_code_t fn, void* data); // When an error is triggered
int utils_cli_error_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_code_t fn, void* data);
int utils_cli_error_middlewares_clear(utils_cli_t* cli);
int utils_cli_prompt_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_t fn, void* data); // When the prompt is displayed
int utils_cli_prompt_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_t fn, void* data);
int utils_cli_prompt_middlewares_clear(utils_cli_t* cli);
int utils_cli_input_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_t fn, void* data); // When a key is pressed
int utils_cli_input_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_char_t fn, void* data);
int utils_cli_input_middlewares_clear(utils_cli_t* cli);
int utils_cli_parser_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_str_t fn, void* data); // When the parser is called
int utils_cli_parser_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_parsed_t fn, void* data);
int utils_cli_parser_middlewares_clear(utils_cli_t* cli);
int utils_cli_exec_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_parsed_t fn, void* data); // When the parsed data is executed
int utils_cli_exec_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_parsed_t fn, void* data);
int utils_cli_exec_middlewares_clear(utils_cli_t* cli);
int utils_cli_command_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_str_t fn, void* data); // When a command is executed
int utils_cli_command_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_str_t fn, void* data);
int utils_cli_command_middlewares_clear(utils_cli_t* cli);

/* setter / getter */
void utils_cli_set_input_delimitor(utils_cli_t* cli, char c);
char utils_cli_get_input_delimitor(const utils_cli_t* cli);
uint8_t utils_cli_get_code(const utils_cli_t* cli);
uint32_t utils_cli_get_flags(const utils_cli_t* cli);

/* strings: the pointer is valid until the next call of the same function in the thread (NULL on error) */
const char* utils_cli_strcode(const utils_cli_t* cli, uint8_t code);
const char* utils_cli_history_get(const utils_cli_t* cli, size_t i);
size_t utils_cli_history_size(const utils_cli_t* cli);

/* strings (_r): copied in buf (truncated, always '\0' terminated), returns the full length like snprintf (0 on error) */
size_t utils_cli_strcode_r(const utils_cli_t* cli, uint8_t code, char* buf, size_t size);
size_t utils_cli_history_get_r(const utils_cli_t* cli, size_t i, char* buf, size_t size);

/* parsed data (one group per command, PARSED: <command> <command> [args...] <separator>, raw: <command> <input> <separator>, separator: "&&", "||", ";" or "") */
utils_cli_parsed_t* utils_cli_parsed_create(void); // NULL on error
void utils_cli_parsed_destroy(utils_cli_parsed_t* parsed);
int utils_cli_parsed_clear(utils_cli_parsed_t* parsed);
int utils_cli_parsed_add_group(utils_cli_parsed_t* parsed); // new command
int utils_cli_parsed_add_arg(utils_cli_parsed_t* parsed, const char* arg); // to the last group
size_t utils_cli_parsed_groups(const utils_cli_parsed_t* parsed);
size_t utils_cli_parsed_size(const utils_cli_parsed_t* parsed, size_t group);
const char* utils_cli_parsed_get(const utils_cli_parsed_t* parsed, size_t group, size_t i); // valid until parsed is changed (NULL on error)

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* CLI_CAPI_C_H */
