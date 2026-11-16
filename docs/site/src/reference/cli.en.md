+++
title = "Cli"
summary = "A customizable interactive command line: commands, prompt, history, arrows, hints, auto completion, logic operators, in the current thread or in a thread."
keywords = "cli command line interactive prompt history hint completion flags TERM2 setCommand start interrupt kill hooks middlewares exit code utils_cli_create"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_Cli` | `_Handling` | `utils::cli` | `utils/cli/Cli.hpp`, `utils/cli/Flags.hpp` |

A customizable interactive command line: commands, prompt, history, arrows, hints, auto completion, logic operators, run in the current thread or in a thread. Line wrapping is not handled.

## Flags {#flags}

`utils::cli::Flag` values are combined with `|` and set with `setFlags`, `addFlags`, `subFlags`, `resetFlags` (back to `flags::DEFAULT`).

| Flag | Effect |
|---|---|
| `DEBUG` | Verbose for the internal actions (nothing for now) |
| `NOECHO` | Don't echo the input |
| `CATCH` | Catch the exceptions of the commands (code `130`) instead of letting them go up |
| `EMPTY_INPUT` | Ignore the empty inputs (default: error `127`) |
| `TRIM` | Trim the input |
| `PARSED` | Split the input and call the parsed commands (`std::vector<std::string>`, default: raw `std::string`) |
| `PROMPT` | Display the prompt (only on a tty) |
| `LOGIC` | Enable `&&`, `\|\|` and `;` between commands (bash semantics) |
| `ARROW` | Left / right arrows in the input |
| `HISTORY` | History with the up / down arrows |
| `PERSISTENT` | Keep the history between sessions (`~/.utils-cli_history`, 10 000 lines max) |
| `HINT` | Display a hint when a command fails |
| `AUTO_COMPLETION` | Complete the first command with <kbd>Tab</kbd> |
| `MANUAL` | Handle only `call` inputs per `start` (see [Start and stop](#start-stop)) |
| `THREAD` | Run in a thread, `start` returns it |
| `DETACHED` | Detach the thread (`start` returns nothing, wait with `join()`) |
| `NO_TTY` | Allow the usage without a tty (default: `CliTTY` error) |

| Preset (`utils::cli::flags::`) | Content |
|---|---|
| `DUMB` | nothing |
| `DEFAULT` | `CATCH`, `EMPTY_INPUT`, `TRIM`, `PROMPT`, `ARROW` |
| `TERM1` | `DEFAULT` + `PARSED`, `LOGIC`, `HISTORY` |
| `TERM2` | `TERM1` + `HINT`, `AUTO_COMPLETION` |
| `TERM3` | `TERM2` + `THREAD` |
| `LOG` | `TERM3` + `DETACHED`, `NO_TTY` |
| `DEV` | `TERM2` + `DEBUG` |
| `MULTI_THREADING` | `THREAD`, `DETACHED` |
| `ALL` | every flag except `PERSISTENT` and `NO_TTY` |

## Commands {#commands}

| Kind | Signature | Used when |
|---|---|---|
| raw | `std::function<void(const Cli&, const std::string& input)>` | `PARSED` off |
| parsed | `std::tuple<std::function<void(const Cli&, const std::vector<std::string>& args)>, std::int16_t min, std::int16_t max>` | `PARSED` on, `args[0]` is the command, `min` / `max` count the other arguments (`-1` = no limit) |

| Method | Description |
|---|---|
| `setCommand<force>(name, command)` / `setCommands<force>(map)` | Add commands, `force = true` to override an existing one |
| `delCommand(name)` / `delCommands(names)` | Remove commands |
| `clearCommands()` | Remove every command |
| `resetCommands()` | Back to the built-in commands only |

Built-in commands: `help` (list of the commands), `exit` / `quit` / `bye` (stop the cli), `?` (last code and its meaning).

## Start and stop {#start-stop}

| Method | Description |
|---|---|
| `start(call = 1, failsafe = false)` | Run the cli; returns the thread with `THREAD` (not `DETACHED`), else `std::nullopt` |
| `start(input, ...)` / `start(inputs, ...)` | Same, executing the given inputs first |
| `interrupt()` | Stop after the current input (like <kbd>Ctrl</kbd>+<kbd>C</kbd> when the constructor gets `sig = true`) |
| `kill()` | Stop, can't be started again |
| `join()` | Wait until the cli stops (detached thread) |
| `isRunning()`, `wasInterrupted()`, `wasKilled()`, `wasStopped()` | State |
| `getCode()` / `strcode(code)` | Last code and its text |
| `getHistory()` | History of the inputs |
| `setInputDelimitor(c)` | End of input character (default `'\n'`) |

> [!NOTE]
> `Cli(true)` catches <kbd>Ctrl</kbd>+<kbd>C</kbd> / <kbd>Ctrl</kbd>+<kbd>Z</kbd> (the previous handlers are restored by the destructor). The terminal is put in raw mode while the cli runs, only when stdin is a tty.

> [!NOTE]
> Windows: the console is set through `utils::platform::tcgetattr` / `tcsetattr` (no line input, no echo, VT input and output); <kbd>Ctrl</kbd>+<kbd>Z</kbd> is a char (no `SIGTSTP`), the history is in `%USERPROFILE%` when `HOME` is not set.

## Hooks {#hooks}

| Hook | Setter / reset | Default |
|---|---|---|
| prompt `void(const Cli&, std::uint8_t code)` | `setPromptHook` / `resetPromptHook` | `defaultPromptHook`: prints `> ` |
| parser `ParsedData(const std::string&, bool trim, bool logic, bool parse)` | `setParserHook` / `resetParserHook` | `defaultParserHook`: quotes, escapes, `&&` / `\|\|` / `;` |
| getc `bool(char& c)` | `setGetCHook` / `resetGetCHook` | `defaultGetCHook`: reads stdin (returns `false` when nothing was read) |

`resetHooks()` resets the 3 hooks.

## Middlewares {#middlewares}

Public [middleware](page:pool#middleware) members, reset with `resetMiddlewares()`:

| Member | `before` / `after` arguments | Called |
|---|---|---|
| `cliMiddlewares` | - / - | start / end of the cli |
| `errorMiddlewares` | code / code | when an error code is set |
| `promptMiddlewares` | - / - | around the prompt |
| `inputMiddlewares` | - / `char` | after each key (only `after`) |
| `parserMiddlewares` | input / parsed data | around the parser |
| `execMiddlewares` | parsed data / parsed data | around the execution |
| `commandMiddlewares` | input / input | around each command |

## Exit codes {#exit-codes}

| Code | Meaning |
|---|---|
| `0` | OK |
| `1` | Internal error |
| `2` | Hook internal error or missing |
| `3` | Middleware internal error |
| `124` | Not enough arguments |
| `125` | Too many arguments |
| `126` | Unclosed escape sequence |
| `127` | Empty input |
| `128` | Unknown command |
| `129` | Command not implemented |
| `130` | Callback exception |
| `255` | Undefined error |

## Example {#example}

```cpp
#define _Cli
#include <utils/utils.hpp>

int main(void)
{
    utils::cli::Cli cli(true); // catch ctrl-c & ctrl-z
    cli.setFlags(utils::cli::flags::TERM2);
    cli.setCommand("add", std::make_tuple(
        std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>(
            [](const utils::cli::Cli&, const std::vector<std::string>& args) {
                std::cout << std::stoi(args[1]) + std::stoi(args[2]) << std::endl;
            }),
        std::int16_t{2}, std::int16_t{2} // exactly 2 arguments
    ));
    (void)cli.start();
    return cli.getCode();
}
```

## In C {#in-c}

Every call of `Cli` goes through the opaque handle `utils_cli_t*` (`utils_cli_create(sig)` / `utils_cli_destroy`). The thread of `THREAD` is kept in the handle: wait for it with `utils_cli_join`.

| C++ | C |
|---|---|
| `Cli(sig)` | `utils_cli_create(sig)` (`NULL` on error) / `utils_cli_destroy(cli)` (kills and joins a running cli) |
| `setFlags(flags::TERM2)` | `utils_cli_set_flags(cli, C_API_CLI_FLAGS_TERM2)` (`C_API_CLI_FLAG_*` combined with `\|`) |
| parsed command | `utils_cli_set_parsed_command(cli, name, fn, min, max, data, force)`, `fn(cli, argc, argv, data)` with `argv[0]` = the command |
| raw command | `utils_cli_set_raw_command(cli, name, fn, data, force)` |
| `start(inputs, call)` | `utils_cli_start(cli, inputs, count, call)` |
| hooks, middlewares | `utils_cli_set_*_hook`, `utils_cli_<name>_middlewares_add_before` / `_add_after` / `_clear` |
| `strcode(code)`, `getHistory()` | `utils_cli_strcode` / `utils_cli_strcode_r`, `utils_cli_history_get` / `_r`, `utils_cli_history_size` |
| `ParsedData` | `utils_cli_parsed_t*` (`utils_cli_parsed_*`) |

A callback returns `UTILS_OK`, `UTILS_KO` (with `utils_fail(code, info)` for the message) or `C_API_CLI_EXIT` to stop the cli like the `exit` command.

```c
#define _Cli
#include <utils.h>
#include <stdio.h>
#include <stdlib.h>

static int add(const utils_cli_t* cli, size_t argc, const char* const* argv, void* data)
{
    (void)cli;
    (void)argc;
    (void)data;
    printf("%ld\n", strtol(argv[1], NULL, 10) + strtol(argv[2], NULL, 10));
    return UTILS_OK;
}

int main(void)
{
    utils_cli_t* cli = utils_cli_create(true);   // catch ctrl-c & ctrl-z
    int code = 1;

    if (!cli)
        return 1;
    utils_cli_set_flags(cli, C_API_CLI_FLAGS_TERM2);
    if (utils_cli_set_parsed_command(cli, "add", add, 2, 2, NULL, false) == UTILS_OK
        && utils_cli_start(cli, NULL, 0, 1) == UTILS_OK)
        code = utils_cli_get_code(cli);
    else
        fprintf(stderr, "%s\n", utils_exception_format());
    utils_cli_destroy(cli);
    return code;
}
```
