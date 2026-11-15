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
##  @file CApi.cpp

File Description:
##  Definition of the C API of utils::cli::Cli
\**************************************************************/

#include "utils/pool/middleware/MiddlewaresType.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/basic/NoneException.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/exception/CApi.hpp"
#include "utils/cli/CApi.hpp"
#include "utils/cli/Cli.hpp"
#include "utils/cli/CApi.h"
#include <source_location>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

//----------------------------------------------------------------//
/* TOOLS */

_hot _nodiscard static inline int status(const bool ok) noexcept
{
    return (ok ? UTILS_OK : UTILS_KO);
}

/* utils_cli_parsed_t is never defined: it's only the C name of a utils::cli::ParsedData */
_hot _nodiscard static inline utils::cli::ParsedData& parsed_data(utils_cli_parsed_t* parsed, const std::source_location loc = std::source_location::current())
{
    return *reinterpret_cast<utils::cli::ParsedData*>(utils::exception::c_api::not_null(parsed, "parsed data", loc));
}

_hot _nodiscard static inline const utils::cli::ParsedData& parsed_data(const utils_cli_parsed_t* parsed, const std::source_location loc = std::source_location::current())
{
    return *reinterpret_cast<const utils::cli::ParsedData*>(utils::exception::c_api::not_null(parsed, "parsed data", loc));
}

_hot _nodiscard static inline utils_cli_parsed_t* parsed_c(utils::cli::ParsedData& parsed) noexcept
{
    return reinterpret_cast<utils_cli_parsed_t*>(&parsed);
}

_hot _nodiscard static inline const utils_cli_parsed_t* parsed_c(const utils::cli::ParsedData& parsed) noexcept
{
    return reinterpret_cast<const utils_cli_parsed_t*>(&parsed);
}

// Return of a C callback -> c++ behavior (throw on error)
_hot static void check(utils_cli_t* self, const int ret, const bool command)
{
    // Success
    if (ret == UTILS_OK) _likely {return;}

    // Exit: a command stops the cli like the exit command, a hook / middleware interrupts it
    if (ret == C_API_CLI_EXIT) {
        if (command) throw utils::exception::NoneException(utils::exception::InternalCode::Exit);
        self->cli.interrupt();
        return;
    }

    // Error (UTILS_KO or unknown value), info given with utils_fail / utils_exception_set in utils_exception
    throw std::runtime_error(utils_exception.exception ? (utils_exception.info[0] != '\0' ? utils_exception.info : utils_exception.what) : "C callback failed (no info given with utils_fail)");
}

//----------------------------------------------------------------//
/* WRAPPER */

/* commands */
_cold _nodiscard static std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)> parsed_command(utils_cli_t* self, utils_cli_parsed_command_t fn, void* data)
{
    if (!fn) return {}; // Command not implemented (like an empty std::function)
    return [self, fn, data](const utils::cli::Cli&, const std::vector<std::string>& args) {
        std::vector<const char*> argv;
        argv.reserve(args.size() + 1);
        for (const std::string& arg: args)
            argv.push_back(arg.c_str());
        argv.push_back(nullptr); // argv[argc] = NULL (like main)
        utils::exception::c_api::reset_status(); // only the error of this callback
        check(self, fn(self, args.size(), argv.data(), data), true);
    };
}

_cold _nodiscard static std::function<void(const utils::cli::Cli&, const std::string&)> raw_command(utils_cli_t* self, utils_cli_raw_command_t fn, void* data)
{
    if (!fn) return {}; // Command not implemented (like an empty std::function)
    return [self, fn, data](const utils::cli::Cli&, const std::string& input) {
        utils::exception::c_api::reset_status(); // only the error of this callback
        check(self, fn(self, input.c_str(), data), true);
    };
}

/* middlewares */
_cold _nodiscard static utils::pool::Middleware<void> middleware(utils_cli_t* self, utils_cli_middleware_t fn, void* data)
{
    return [self, fn, data](void) {utils::exception::c_api::reset_status(); check(self, fn(data), false);};
}

_cold _nodiscard static utils::pool::Middleware<std::uint8_t> middleware(utils_cli_t* self, utils_cli_middleware_code_t fn, void* data)
{
    return [self, fn, data](std::uint8_t code) {utils::exception::c_api::reset_status(); check(self, fn(code, data), false);};
}

_cold _nodiscard static utils::pool::Middleware<char> middleware(utils_cli_t* self, utils_cli_middleware_char_t fn, void* data)
{
    return [self, fn, data](char c) {utils::exception::c_api::reset_status(); check(self, fn(c, data), false);};
}

_cold _nodiscard static utils::pool::Middleware<const std::string&> middleware(utils_cli_t* self, utils_cli_middleware_str_t fn, void* data)
{
    return [self, fn, data](const std::string& str) {utils::exception::c_api::reset_status(); check(self, fn(str.c_str(), data), false);};
}

_cold _nodiscard static utils::pool::Middleware<const utils::cli::ParsedData&> middleware(utils_cli_t* self, utils_cli_middleware_parsed_t fn, void* data)
{
    return [self, fn, data](const utils::cli::ParsedData& parsed) {utils::exception::c_api::reset_status(); check(self, fn(parsed_c(parsed), data), false);};
}

// Check the handle & the middleware, then add it (f)
template<typename F>
_cold _nodiscard static int add_middleware(utils_cli_t* cli, const bool valid, F&& f)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils_cli_t& self = utils::exception::c_api::not_null_ref(cli, "cli handle");
        if (!valid) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The middleware is NULL");
        }
        f(self);
    }));
}

//----------------------------------------------------------------//
/* HANDLE */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_cli_t* utils_cli_create(bool sig)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_cli_t* {return new utils_cli_t(sig);});
}

_cold void utils_cli_destroy(utils_cli_t* cli)
{
    (void)utils::exception::c_api::guard([&](void) {
        if (!cli) return; // like free(NULL)

        // Stop a running cli, then join its thread (a joinable std::thread can't be destroyed)
        if (cli->cli.isRunning()) cli->cli.kill();
        cli->cli.join();
        if (cli->thread && cli->thread->joinable()) cli->thread->join();
    });
    delete cli;
}

//----------------------------------------------------------------//
/* RUN */

_cold int utils_cli_start(utils_cli_t* cli, const char* const* inputs, size_t count, size_t call)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils_cli_t& self = utils::exception::c_api::not_null_ref(cli, "cli handle");
        std::vector<std::string> list;

        // Initial inputs
        if (count > 0 && !inputs) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The inputs are NULL");
        }
        for (std::size_t i = 0; i < count; ++i)
            list.emplace_back(utils::exception::c_api::not_null(inputs[i], "input"));

        // Keep the thread (THREAD without DETACHED), the one of a previous run is finished (start throws if running)
        std::optional<std::thread> thread = self.cli.start(list, call);
        if (thread) {
            if (self.thread && self.thread->joinable()) self.thread->join();
            self.thread = std::move(thread);
        }
    }));
}

_cold void utils_cli_join(utils_cli_t* cli)
{
    (void)utils::exception::c_api::guard([&](void) {
        utils_cli_t& self = utils::exception::c_api::not_null_ref(cli, "cli handle");
        if (self.thread && self.thread->joinable()) self.thread->join();
        else self.cli.join();
        self.thread.reset();
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold void utils_cli_interrupt(utils_cli_t* cli)
{
    (void)utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.interrupt();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold void utils_cli_kill(utils_cli_t* cli)
{
    (void)utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.kill();});
}

//----------------------------------------------------------------//
/* STATUS */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold bool utils_cli_is_running(const utils_cli_t* cli)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(cli, "cli handle").cli.isRunning();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold bool utils_cli_was_interrupted(const utils_cli_t* cli)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(cli, "cli handle").cli.wasInterrupted();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold bool utils_cli_was_killed(const utils_cli_t* cli)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(cli, "cli handle").cli.wasKilled();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold bool utils_cli_was_stopped(const utils_cli_t* cli)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(cli, "cli handle").cli.wasStopped();});
}

//----------------------------------------------------------------//
/* FLAGS */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold void utils_cli_reset_flags(utils_cli_t* cli)
{
    (void)utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.resetFlags();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold void utils_cli_sub_flags(utils_cli_t* cli, uint32_t flags)
{
    (void)utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.subFlags(flags);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold void utils_cli_add_flags(utils_cli_t* cli, uint32_t flags)
{
    (void)utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.addFlags(flags);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold void utils_cli_set_flags(utils_cli_t* cli, uint32_t flags)
{
    (void)utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.setFlags(flags);});
}

//----------------------------------------------------------------//
/* COMMANDS */

_cold int utils_cli_set_parsed_command(utils_cli_t* cli, const char* name, utils_cli_parsed_command_t fn, int16_t min, int16_t max, void* data, bool force)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils_cli_t& self = utils::exception::c_api::not_null_ref(cli, "cli handle");
        const std::string command = utils::exception::c_api::not_null(name, "command name");
        if (force) self.cli.setCommand<true>(command, std::make_tuple(parsed_command(&self, fn, data), min, max));
        else self.cli.setCommand<false>(command, std::make_tuple(parsed_command(&self, fn, data), min, max));
    }));
}

_cold int utils_cli_set_raw_command(utils_cli_t* cli, const char* name, utils_cli_raw_command_t fn, void* data, bool force)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils_cli_t& self = utils::exception::c_api::not_null_ref(cli, "cli handle");
        const std::string command = utils::exception::c_api::not_null(name, "command name");
        if (force) self.cli.setCommand<true>(command, raw_command(&self, fn, data));
        else self.cli.setCommand<false>(command, raw_command(&self, fn, data));
    }));
}

_cold int utils_cli_set_parsed_commands(utils_cli_t* cli, const utils_cli_parsed_command_entry_t* commands, size_t count, bool force)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils_cli_t& self = utils::exception::c_api::not_null_ref(cli, "cli handle");
        std::unordered_map<std::string, std::tuple<std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>, std::int16_t, std::int16_t>> map;

        // Convert the entries
        if (count > 0 && !commands) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The commands are NULL");
        }
        for (std::size_t i = 0; i < count; ++i)
            map[utils::exception::c_api::not_null(commands[i].name, "command name")] = std::make_tuple(parsed_command(&self, commands[i].fn, commands[i].data), commands[i].min, commands[i].max);

        if (force) self.cli.setCommands<true>(map);
        else self.cli.setCommands<false>(map);
    }));
}

_cold int utils_cli_set_raw_commands(utils_cli_t* cli, const utils_cli_raw_command_entry_t* commands, size_t count, bool force)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils_cli_t& self = utils::exception::c_api::not_null_ref(cli, "cli handle");
        std::unordered_map<std::string, std::function<void(const utils::cli::Cli&, const std::string&)>> map;

        // Convert the entries
        if (count > 0 && !commands) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The commands are NULL");
        }
        for (std::size_t i = 0; i < count; ++i)
            map[utils::exception::c_api::not_null(commands[i].name, "command name")] = raw_command(&self, commands[i].fn, commands[i].data);

        if (force) self.cli.setCommands<true>(map);
        else self.cli.setCommands<false>(map);
    }));
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_del_command(utils_cli_t* cli, const char* name)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.delCommand(utils::exception::c_api::not_null(name, "command name"));}));
}

_cold int utils_cli_del_commands(utils_cli_t* cli, const char* const* names, size_t count)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils_cli_t& self = utils::exception::c_api::not_null_ref(cli, "cli handle");
        std::vector<std::string> list;

        if (count > 0 && !names) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The command names are NULL");
        }
        for (std::size_t i = 0; i < count; ++i)
            list.emplace_back(utils::exception::c_api::not_null(names[i], "command name"));
        self.cli.delCommands(list);
    }));
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_reset_commands(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.resetCommands();}));
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_clear_commands(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.clearCommands();}));
}

//----------------------------------------------------------------//
/* HOOKS */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_reset_hooks(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.resetHooks();}));
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_reset_prompt_hook(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.resetPromptHook();}));
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_reset_parser_hook(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.resetParserHook();}));
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_reset_getc_hook(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.resetGetCHook();}));
}

_cold int utils_cli_set_prompt_hook(utils_cli_t* cli, utils_cli_prompt_hook_t hook, void* data)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils_cli_t& self = utils::exception::c_api::not_null_ref(cli, "cli handle");
        utils_cli_t* ptr = &self;

        // NULL: no hook (error "Missing hook" at the call, like an empty std::function)
        if (!hook) {self.cli.setPromptHook({}); return;}
        self.cli.setPromptHook([ptr, hook, data](const utils::cli::Cli&, std::uint8_t code) {
            utils::exception::c_api::reset_status(); // only the error of this callback
            check(ptr, hook(ptr, code, data), false);
        });
    }));
}

_cold int utils_cli_set_parser_hook(utils_cli_t* cli, utils_cli_parser_hook_t hook, void* data)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils_cli_t& self = utils::exception::c_api::not_null_ref(cli, "cli handle");
        utils_cli_t* ptr = &self;

        // NULL: no hook (error "Missing hook" at the call, like an empty std::function)
        if (!hook) {self.cli.setParserHook({}); return;}
        self.cli.setParserHook([ptr, hook, data](const std::string& input, bool trim, bool logic, bool parse) {
            utils::cli::ParsedData out;
            utils::exception::c_api::reset_status(); // only the error of this callback
            check(ptr, hook(input.c_str(), trim, logic, parse, parsed_c(out), data), false);
            return out;
        });
    }));
}

_cold int utils_cli_set_getc_hook(utils_cli_t* cli, utils_cli_getc_hook_t hook, void* data)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils_cli_t& self = utils::exception::c_api::not_null_ref(cli, "cli handle");
        utils_cli_t* ptr = &self;

        // NULL: no hook (error "Missing hook" at the call, like an empty std::function)
        if (!hook) {self.cli.setGetCHook({}); return;}
        self.cli.setGetCHook([ptr, hook, data](char& c) {
            utils::exception::c_api::reset_status(); // only the error of this callback
            const int ret = hook(&c, data);
            if (ret == C_API_CLI_NO_INPUT) {return false;} // called again
            check(ptr, ret, false);
            return (ret == UTILS_OK); // exit: the cli is interrupted
        });
    }));
}

//----------------------------------------------------------------//
/* DEFAULT HOOKS */

_cold int utils_cli_default_prompt_hook(const utils_cli_t* cli, uint8_t code, _unused void* data)
{
    return status(utils::exception::c_api::guard([&](void) {utils::cli::defaultPromptHook(utils::exception::c_api::not_null_ref(cli, "cli handle").cli, code);}));
}

_cold int utils_cli_default_parser_hook(const char* input, bool trim, bool logic, bool parse, utils_cli_parsed_t* out, _unused void* data)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils::cli::ParsedData& parsed = parsed_data(out);
        parsed = utils::cli::defaultParserHook(utils::exception::c_api::not_null(input, "input"), trim, logic, parse);
    }));
}

_cold int utils_cli_default_getc_hook(char* c, _unused void* data)
{
    bool read = false;

    if (!utils::exception::c_api::guard([&](void) {read = utils::cli::defaultGetCHook(*utils::exception::c_api::not_null(c, "char"));})) _unlikely {return UTILS_KO;}
    return (read ? UTILS_OK : C_API_CLI_NO_INPUT);
}

//----------------------------------------------------------------//
/* MIDDLEWARES */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_reset_middlewares(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.resetMiddlewares();}));
}

/* cli */
_cold int utils_cli_cli_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.cliMiddlewares.addBefore(middleware(&self, fn, data));});
}

_cold int utils_cli_cli_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.cliMiddlewares.addAfter(middleware(&self, fn, data));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_cli_middlewares_clear(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.cliMiddlewares.clear();}));
}

/* error */
_cold int utils_cli_error_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_code_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.errorMiddlewares.addBefore(middleware(&self, fn, data));});
}

_cold int utils_cli_error_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_code_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.errorMiddlewares.addAfter(middleware(&self, fn, data));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_error_middlewares_clear(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.errorMiddlewares.clear();}));
}

/* prompt */
_cold int utils_cli_prompt_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.promptMiddlewares.addBefore(middleware(&self, fn, data));});
}

_cold int utils_cli_prompt_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.promptMiddlewares.addAfter(middleware(&self, fn, data));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_prompt_middlewares_clear(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.promptMiddlewares.clear();}));
}

/* input */
_cold int utils_cli_input_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.inputMiddlewares.addBefore(middleware(&self, fn, data));});
}

_cold int utils_cli_input_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_char_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.inputMiddlewares.addAfter(middleware(&self, fn, data));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_input_middlewares_clear(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.inputMiddlewares.clear();}));
}

/* parser */
_cold int utils_cli_parser_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_str_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.parserMiddlewares.addBefore(middleware(&self, fn, data));});
}

_cold int utils_cli_parser_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_parsed_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.parserMiddlewares.addAfter(middleware(&self, fn, data));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_parser_middlewares_clear(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.parserMiddlewares.clear();}));
}

/* exec */
_cold int utils_cli_exec_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_parsed_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.execMiddlewares.addBefore(middleware(&self, fn, data));});
}

_cold int utils_cli_exec_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_parsed_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.execMiddlewares.addAfter(middleware(&self, fn, data));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_exec_middlewares_clear(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.execMiddlewares.clear();}));
}

/* command */
_cold int utils_cli_command_middlewares_add_before(utils_cli_t* cli, utils_cli_middleware_str_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.commandMiddlewares.addBefore(middleware(&self, fn, data));});
}

_cold int utils_cli_command_middlewares_add_after(utils_cli_t* cli, utils_cli_middleware_str_t fn, void* data)
{
    return add_middleware(cli, fn, [&](utils_cli_t& self) {self.cli.commandMiddlewares.addAfter(middleware(&self, fn, data));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_command_middlewares_clear(utils_cli_t* cli)
{
    return status(utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.commandMiddlewares.clear();}));
}

//----------------------------------------------------------------//
/* SETTER / GETTER */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold void utils_cli_set_input_delimitor(utils_cli_t* cli, char c)
{
    (void)utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(cli, "cli handle").cli.setInputDelimitor(c);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold char utils_cli_get_input_delimitor(const utils_cli_t* cli)
{
    return utils::exception::c_api::guard_fallback('\0', [&](void) -> char {return utils::exception::c_api::not_null_ref(cli, "cli handle").cli.getInputDelimitor();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold uint8_t utils_cli_get_code(const utils_cli_t* cli)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> uint8_t {return utils::exception::c_api::not_null_ref(cli, "cli handle").cli.getCode();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold uint32_t utils_cli_get_flags(const utils_cli_t* cli)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> uint32_t {return utils::exception::c_api::not_null_ref(cli, "cli handle").cli.getFlags();});
}

/* strings (thread local buffer) */
_cold const char* utils_cli_strcode(const utils_cli_t* cli, uint8_t code)
{
    // xstyle: ignore-next CPP-THIS (false positive: _thread_local is the attribute macro, not a member)
    static _thread_local std::string str;

    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {str = utils::exception::c_api::not_null_ref(cli, "cli handle").cli.strcode(code); return str.c_str();});
}

_cold const char* utils_cli_history_get(const utils_cli_t* cli, size_t i)
{
    // xstyle: ignore-next CPP-THIS (false positive: _thread_local is the attribute macro, not a member)
    static _thread_local std::string str;

    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {
        const std::vector<std::string> history = utils::exception::c_api::not_null_ref(cli, "cli handle").cli.getHistory();
        if (i >= history.size()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "History index out of range");
        }
        str = history[i];
        return str.c_str();
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold size_t utils_cli_history_size(const utils_cli_t* cli)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(cli, "cli handle").cli.getHistory().size();});
}

/* strings (_r: buffer of the user) */
// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold size_t utils_cli_strcode_r(const utils_cli_t* cli, uint8_t code, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::copy_r(utils::exception::c_api::not_null_ref(cli, "cli handle").cli.strcode(code), buf, size);});
}

_cold size_t utils_cli_history_get_r(const utils_cli_t* cli, size_t i, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {
        const std::vector<std::string> history = utils::exception::c_api::not_null_ref(cli, "cli handle").cli.getHistory();
        if (i >= history.size()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "History index out of range");
        }
        return utils::exception::c_api::copy_r(history[i], buf, size);
    });
}

//----------------------------------------------------------------//
/* PARSED DATA */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_cli_parsed_t* utils_cli_parsed_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_cli_parsed_t* {return parsed_c(*new utils::cli::ParsedData());});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold void utils_cli_parsed_destroy(utils_cli_parsed_t* parsed)
{
    (void)utils::exception::c_api::guard([&](void) {delete reinterpret_cast<utils::cli::ParsedData*>(parsed);}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_parsed_clear(utils_cli_parsed_t* parsed)
{
    return status(utils::exception::c_api::guard([&](void) {parsed_data(parsed).clear();}));
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_cli_parsed_add_group(utils_cli_parsed_t* parsed)
{
    return status(utils::exception::c_api::guard([&](void) {parsed_data(parsed).emplace_back();}));
}

_cold int utils_cli_parsed_add_arg(utils_cli_parsed_t* parsed, const char* arg)
{
    return status(utils::exception::c_api::guard([&](void) {
        utils::cli::ParsedData& data = parsed_data(parsed);
        if (data.empty()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Empty, "No group to add the argument (utils_cli_parsed_add_group first)");
        }
        data.back().emplace_back(utils::exception::c_api::not_null(arg, "argument"));
    }));
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold size_t utils_cli_parsed_groups(const utils_cli_parsed_t* parsed)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return parsed_data(parsed).size();});
}

_cold size_t utils_cli_parsed_size(const utils_cli_parsed_t* parsed, size_t group)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {
        const utils::cli::ParsedData& data = parsed_data(parsed);
        if (group >= data.size()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Parsed data group out of range");
        }
        return data[group].size();
    });
}

_cold const char* utils_cli_parsed_get(const utils_cli_parsed_t* parsed, size_t group, size_t i)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {
        const utils::cli::ParsedData& data = parsed_data(parsed);
        if (group >= data.size() || i >= data[group].size()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Parsed data index out of range");
        }
        return data[group][i].c_str();
    });
}
