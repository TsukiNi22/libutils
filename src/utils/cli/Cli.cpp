/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 14/08/2026 by @author Tsukini

File Name:
##  @file Cli.cpp

File Description:
##  Definition of the main cli functions
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/WarningException.hpp"
#include "utils/exception/basic/NoneException.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/IException.hpp"
#include "utils/algorithms/c2dmp-hsm/c2dmp-hsm.hpp"
#include "utils/manip/iomanip/ANSI.hpp"
#include "utils/cli/Cli.hpp"
#include "utils/cli/Flags.hpp"
#include <unistd.h>
#include <unordered_map>
#include <shared_mutex>
#include <functional>
#include <exception>
#include <algorithm>
#include <iostream>
#include <optional>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <thread>
#include <vector>
#include <string>
#include <tuple>

_hot _nodiscard static std::string trim(const std::string& s)
{
    std::size_t start = s.find_first_not_of(" ");
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(" ");
    return s.substr(start, end - start + 1);
}

_hot static void delete_chars(const std::size_t n)
{
    for (std::size_t i = 0; i < n; ++i)
        std::cout << "\b \b";
    std::cout << std::flush;
}

_hot void utils::cli::Cli::launch_(const std::size_t call)
{
    try {this->cliMiddlewares.callBefore();}
    catch (const utils::exception::ErrorException& e) {std::cerr << e.what() << ": " << e.info() << std::endl;}
    std::string input;

    // Main loop
    try {
        for (std::size_t i = 0; (!(this->_flags & utils::cli::Flag::MANUAL) || i < call) && !this->_interrupted; ++i) {
            // Display prompt
            if (this->_flags & utils::cli::Flag::PROMPT) {
                try {
                    this->prompt_();
                } catch (const utils::exception::ErrorException& e) {
                    utils::exception::InternalCode code = e.getCode();
                    if (code == utils::exception::InternalCode::CliHook) this->_code = 2;
                    else if (code == utils::exception::InternalCode::MiddlewareCall) this->_code = 3;
                    else _unlikely {this->_code = 255;}
                    try {this->errorMiddlewares.callBefore(this->_code);}
                    catch (const utils::exception::ErrorException& e) {std::cerr << e.what() << ": " << e.info() << std::endl;}
                    if (this->_code <= 3 || this->_code == 255) std::cerr << e.what() << ": " << e.info() << std::endl;
                    else std::cout << e.info() << std::endl;
                    try {this->errorMiddlewares.callAfter(this->_code);}
                    catch (const utils::exception::ErrorException& e) {std::cerr << e.what() << ": " << e.info() << std::endl;}
                }
            }

            // Get input & Check if it's empty
            try {
                input = this->getInput_();
                if (this->_interrupted) { // Check for interrupt
                    this->_running = false;
                    return;
                }
                std::unique_lock historyLock(this->_historyLock);
                if (this->_history.size() == 0 || this->_history.back() != input)
                    this->_history.push_back(input);
            } catch (const utils::exception::ErrorException& e) {
                utils::exception::InternalCode code = e.getCode();
                if (code == utils::exception::InternalCode::CliInternal) this->_code = 1;
                else if (code == utils::exception::InternalCode::CliHook) this->_code = 2;
                else if (code == utils::exception::InternalCode::MiddlewareCall) this->_code = 3;
                else _unlikely {this->_code = 255;}
                try {this->errorMiddlewares.callBefore(this->_code);}
                catch (const utils::exception::ErrorException& e) {std::cerr << e.what() << ": " << e.info() << std::endl;}
                if (this->_code <= 3 || this->_code == 255) std::cerr << e.what() << ": " << e.info() << std::endl;
                else std::cout << e.info() << std::endl;
                try {this->errorMiddlewares.callAfter(this->_code);}
                catch (const utils::exception::ErrorException& e) {std::cerr << e.what() << ": " << e.info() << std::endl;}
                continue;
            }
            if (!(this->_flags & utils::cli::Flag::EMPTY_INPUT) && input.empty()) {
                this->_code = 127;
                try {this->errorMiddlewares.callBefore(this->_code);}
                catch (const utils::exception::ErrorException& e) {std::cerr << e.what() << ": " << e.info() << std::endl;}
                std::cout << "Empty input" << std::endl;
                try {this->errorMiddlewares.callAfter(this->_code);}
                catch (const utils::exception::ErrorException& e) {std::cerr << e.what() << ": " << e.info() << std::endl;}
                continue;
            }

            // Parse & Execute input
            try {
                this->exec_(this->parse_(input));
            } catch (const utils::exception::ErrorException& e) {
                utils::exception::InternalCode code = e.getCode();
                if (code == utils::exception::InternalCode::CliHook) this->_code = 2;
                else if (code == utils::exception::InternalCode::MiddlewareCall) this->_code = 3;
                else if (code == utils::exception::InternalCode::CliParser) {
                    std::string info = e.info();
                    if (info == "Not enough arguments") this->_code = 124;
                    else if (info == "Too many arguments") this->_code = 125;
                    else if (info == "Unclosed escape sequence") this->_code = 126;
                } else if (code == utils::exception::InternalCode::CliExecution) {
                    std::string info = e.info();
                    if (info == "Unknown command") this->_code = 128;
                    else if (info == "Command not implemented") this->_code = 129;
                    else if (info.starts_with("Callback exception: ")) {
                        if (!(this->_flags & utils::cli::Flag::CATCH))
                            throw; // _running is reset by start (last access to the cli)
                        this->_code = 130;
                    }
                } else _unlikely {this->_code = 255;}
                try {this->errorMiddlewares.callBefore(this->_code);}
                catch (const utils::exception::ErrorException& e) {std::cerr << e.what() << ": " << e.info() << std::endl;}
                if (this->_code <= 3 || this->_code == 255) std::cerr << e.what() << ": " << e.info() << std::endl;
                else std::cout << e.info() << std::endl;
                try {this->errorMiddlewares.callAfter(this->_code);}
                catch (const utils::exception::ErrorException& e) {std::cerr << e.what() << ": " << e.info() << std::endl;}
                continue;
            }
        }
    } catch (const utils::exception::NoneException& e) { // Catch exit throw
        if (e.getCode() != utils::exception::InternalCode::Exit)
            throw; // _running is reset by start (last access to the cli)
    }

    try {this->cliMiddlewares.callAfter();}
    catch (const utils::exception::ErrorException& e) {std::cerr << e.what() << ": " << e.info() << std::endl;}
    this->_running = false;
}

_cold void utils::cli::Cli::join(void) const noexcept
{
    while (this->_running)
        std::this_thread::yield();
}

_cold std::optional<std::thread> utils::cli::Cli::start(const std::size_t call, const bool failsafe)
{
    return this->start(std::vector<std::string>{}, call, failsafe);
}

_cold std::optional<std::thread> utils::cli::Cli::start(const std::string& input, const std::size_t call, const bool failsafe)
{
    return this->start(std::vector<std::string>{input}, call, failsafe);
}

_cold std::optional<std::thread> utils::cli::Cli::start(const std::vector<std::string>& inputs, const std::size_t call, const bool failsafe)
{
    // Check status
    if (!(this->_flags & utils::cli::Flag::NO_TTY) && !isatty(STDOUT_FILENO)) {
        if (failsafe) return std::nullopt;
        throw utils::exception::ErrorException(utils::exception::InternalCode::CliTTY);
    } else if (this->_killed) {
        if (failsafe) return std::nullopt;
        throw utils::exception::WarningException(utils::exception::InternalCode::Killed);
    }

    // Claim the running state (atomic: 2 starts can't both launch)
    bool expected = false;
    if (!this->_running.compare_exchange_strong(expected, true)) {
        if (failsafe) return std::nullopt;
        throw utils::exception::WarningException(utils::exception::InternalCode::AlreadyRunning);
    }

    // Initial inputs
    for (const std::string& input: inputs)
        this->_initInput.push(input);
    this->_interrupted = false; // Reset interrupt status

    // Launch in a thread
    if (this->_flags & utils::cli::Flag::THREAD) {
        // Create the thread (the exceptions are caught inside)
        std::thread t([this, call](void) {
            try {
                this->launch_(call); // set _running to false at its end (last access to the cli)
            } catch (const utils::exception::IException& e) {
                std::cerr << e.formated() << std::endl;
                this->_code = 255;
                this->_running = false; // last access to the cli
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                this->_code = 255;
                this->_running = false; // last access to the cli
            }
        });

        // Detach the thread if needed
        if (this->_flags & utils::cli::Flag::DETACHED) {
            t.detach();
            return std::nullopt;
        }

        // Return the thread for non detached thread (let the user handle it)
        return t;
    }

    // Default linear launch
    try {
        this->launch_(call);
    } catch (...) {
        this->_running = false;
        throw;
    }
    return std::nullopt;
}

_hot void utils::cli::Cli::prompt_(void)
{
    this->promptMiddlewares.callBefore();

    // Call the hook
    if (isatty(STDOUT_FILENO)) {
        std::function<void(const utils::cli::Cli&, std::uint8_t)> hook;
        {std::lock_guard lock(this->_hooksLock); hook = this->_promptHook;} // called without the lock
        if (hook) _likely {
            try {
                hook(*this, this->_code);
            } catch (const std::exception& e) {
                throw utils::exception::ErrorException(utils::exception::InternalCode::CliHook, e.what());
            }
        } else _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::CliHook, "Missing hook for prompting");
        }
    }

    this->promptMiddlewares.callAfter();
}

template<std::size_t depth>
_cold _nodiscard static std::string get_hint(const std::vector<std::string>& list, const std::string& s)
{
    float minDist = 0.0f, dist = 0.0f;
    bool first = true;
    std::string hint;

    // Check for each
    for (const std::string& actual: list) {
        dist = utils::algorithms::c2dmp::c2dmp<depth>(s, actual);
        if (first || dist < minDist) {
            minDist = dist;
            hint = actual;
            first = false;
        }
    }

    return first ? "[None]" : hint;
}

_hot _nodiscard std::string utils::cli::Cli::getInput_(void)
{
    std::vector<std::string> commands;
    bool echo = !(this->_flags & utils::cli::Flag::NOECHO);
    std::size_t indexBuffer = 0, indexHistory = 0;
    {std::shared_lock historyLock(this->_historyLock); indexHistory = this->_history.size();}
    std::size_t lastInputSize = indexBuffer, lastIndexBuffer = indexBuffer;
    bool escape = false, onInput = false;
    std::string input, inputBuff;
    char c = '\0';

    // Initial inputs
    if (this->_initInput.size() > 0) {
        input = this->_initInput.front();
        this->_initInput.pop();
        if (echo && isatty(STDOUT_FILENO)) std::cout << input << std::flush;
        return input;
    }

    // Use of internal input buffer stored (input was previously interrupted)
    else if (!this->_input.empty()) {
        input = this->_input;
        this->_input.clear();
        indexBuffer = input.size();
        lastInputSize = indexBuffer;
        lastIndexBuffer = indexBuffer;
        if (echo && isatty(STDOUT_FILENO)) std::cout << input << std::flush;
    }

    // Loop to get full input
    while (!this->_interrupted) {
        // Get the char
        std::function<bool(char&)> hook;
        {std::lock_guard lock(this->_hooksLock); hook = this->_getcHook;} // called without the lock (wait for a key)
        if (hook) _likely {
            try {
                this->inputMiddlewares.callBefore();
                while (!hook(c) && !this->_interrupted)
                    std::this_thread::yield();
                this->inputMiddlewares.callAfter(c);
            } catch (const std::exception& e) {
                //throw utils::exception::NoneException(utils::exception::InternalCode::Exit);
                throw utils::exception::ErrorException(utils::exception::InternalCode::CliHook, e.what());
            }
        } else _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::CliHook, "Missing hook for char getter");
        }
        if (this->_interrupted) break; // Check for interrupt

        // EOF handling
        if (c == 0) {
            if (isatty(STDOUT_FILENO))
                std::cout << std::endl << "Detected EOF, exiting..." << std::endl;
            throw utils::exception::NoneException(utils::exception::InternalCode::Exit);
        }

        // Ctrl+D handling
        if (c == 4) {
            if (isatty(STDOUT_FILENO))
                std::cout << std::endl << "Detected Ctrl+D, exiting..." << std::endl;
            throw utils::exception::NoneException(utils::exception::InternalCode::Exit);
        }

        // Auto completion
        else if (this->_flags & utils::cli::Flag::AUTO_COMPLETION && c == '\t' && trim(input).find(" ", 0) == std::string::npos) {
            std::shared_lock lock(this->_commandsLock);

            // Setup the commands
            commands.clear();
            if (this->_flags & utils::cli::Flag::PARSED) {
                for (const auto &[cmd, _]: this->_parsedCommands)
                    commands.push_back(cmd);
            } else {
                for (const auto &[cmd, _]: this->_rawCommands)
                    commands.push_back(cmd);
            }

            // Get the hint (nothing to complete: input unchanged)
            if (!commands.empty()) {
                input = get_hint<2>(commands, input);
                indexBuffer = input.size();
            }
        }

        // Arrow left, write
        else if (this->_flags & utils::cli::Flag::ARROW && escape && (indexBuffer > 0 && input[indexBuffer - 1] == '[') && (c == 'C' || c == 'D')) {
            input.erase(--indexBuffer, 1);
            if (c == 'C' && indexBuffer < input.size()) // right
                ++indexBuffer;
            else if (c == 'D' && indexBuffer > 0) // left
                --indexBuffer;
        }

        // History with arrow up, down
        else if (this->_flags & utils::cli::Flag::HISTORY && escape && (indexBuffer > 0 && input[indexBuffer - 1] == '[') && (c == 'A' || c == 'B')) {
            input.erase(--indexBuffer, 1);
            std::shared_lock historyLock(this->_historyLock);
            onInput = (indexHistory == this->_history.size());
            if (c == 'A') { // Up
                if (indexHistory == 0) indexHistory = this->_history.size();
                else --indexHistory;
            }
            else if (c == 'B') { // Down
                if (indexHistory >= this->_history.size()) indexHistory = 0;
                else ++indexHistory;
            }
            if (indexHistory == this->_history.size()) {
                if (!onInput) std::swap(input, inputBuff);
            } else if (onInput) {
                inputBuff = this->_history[indexHistory];
                std::swap(inputBuff, input);
            } else input = this->_history[indexHistory];
            indexBuffer = input.size();
        }

        // Delete char
        else if (c == 127 || c == '\b') {
            if (indexBuffer > 0)
                input.erase(--indexBuffer, 1);
        }

        // End of input
        else if (c == this->_inputDelimitor) {
            if (isatty(STDOUT_FILENO)) std::cout << std::endl;
            break;
        }

        // Add char to the input
        else if (std::isprint(static_cast<unsigned char>(c))) _likely {
            input.insert(indexBuffer++, 1, c);
        }

        // Detect escape sequence
        if (c == '\x1b') {
            escape = true;
        } else if (c != '[') {
            escape = false;
        }

        if (echo && isatty(STDOUT_FILENO)) {
            // Reset the cursor place
            if (lastInputSize - lastIndexBuffer > 0)
                std::cout << utils::iomanip::right(lastInputSize - lastIndexBuffer);

            // Refresh input display
            delete_chars(lastInputSize);
            std::cout << input;
            lastInputSize = input.size();
            lastIndexBuffer = indexBuffer;

            // Place the cursor
            if (input.size() - indexBuffer > 0)
                std::cout << utils::iomanip::left(input.size() - indexBuffer);
        }

        // Update output
        std::cout << std::flush;
    }

    if (this->_killed) return "";
    else if (this->_interrupted) {
        if (isatty(STDOUT_FILENO)) std::cout << std::endl;
        this->_input = input;
        return "";
    }
    if (this->_flags & utils::cli::Flag::TRIM) return trim(input);
    else return input;
}

_hot utils::cli::ParsedData utils::cli::Cli::parse_(const std::string& input)
{
    this->parserMiddlewares.callBefore(input);
    utils::cli::ParsedData parsedInput;

    // Call the hook
    std::function<utils::cli::ParsedData(const std::string&, const bool, const bool, const bool)> hook;
    {std::lock_guard lock(this->_hooksLock); hook = this->_parserHook;} // called without the lock
    if (hook) _likely {
        try {
            parsedInput = hook(input,
                this->_flags & utils::cli::Flag::TRIM,
                this->_flags & utils::cli::Flag::LOGIC,
                this->_flags & utils::cli::Flag::PARSED
            );
        } catch (const std::exception& e) {
            throw utils::exception::ErrorException(utils::exception::InternalCode::CliHook, e.what());
        }
    } else _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::CliHook, "Missing hook for parsing");
    }

    this->parserMiddlewares.callAfter(parsedInput);
    return parsedInput;
}

_hot void utils::cli::Cli::exec_(const utils::cli::ParsedData& parsedInput)
{
    this->execMiddlewares.callBefore(parsedInput);
    std::string lastExceptionInfo;
    std::uint8_t status = 0, lastStatus = 0;

    // For each commands
    for (std::size_t i = 0; i < parsedInput.size(); ++i) {
        const std::vector<std::string>& command = parsedInput[i];

        // Check the logic (bash like): the operator after the previous command decide if this one is executed
        if (i > 0) {
            const std::string& op = parsedInput[i - 1].back();
            if ((op == "&&" && lastStatus != 0) || (op == "||" && lastStatus == 0)) continue; // skipped, the last status is kept
        }

        // Try to exec the command
        status = 0;
        bool middlewareError = false;
        try {
            // Copy the command under the lock and call it without (a command can edit the commands)
            std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)> parsedFn;
            std::function<void(const utils::cli::Cli&, const std::string&)> rawFn;
            std::int16_t min = -1, max = -1;
            bool parsedFound = false, rawFound = false;
            std::vector<std::string> commands;
            {
                std::shared_lock lock(this->_commandsLock);
                auto itParsed = this->_parsedCommands.find(command.front());
                auto itRaw = this->_rawCommands.find(command.front());
                if (this->_flags & utils::cli::Flag::PARSED && itParsed != this->_parsedCommands.end()) {
                    parsedFound = true;
                    std::tie(parsedFn, min, max) = itParsed->second;
                } else if (!(this->_flags & utils::cli::Flag::PARSED) && itRaw != this->_rawCommands.end()) {
                    rawFound = true;
                    rawFn = itRaw->second;
                } else if (this->_flags & utils::cli::Flag::HINT) {
                    if (this->_flags & utils::cli::Flag::PARSED) {
                        for (const auto &[cmd, _]: this->_parsedCommands)
                            commands.push_back(cmd);
                    } else {
                        for (const auto &[cmd, _]: this->_rawCommands)
                            commands.push_back(cmd);
                    }
                }
            }

            // Parsed
            if (parsedFound) {
                this->commandMiddlewares.callBefore(command.front());
                if (parsedFn) { // Check the command existense
                    // Check commands arguments number (-1 = no limit), size = <command> <command> [args...] <separator>
                    const std::int16_t args = static_cast<std::int16_t>(command.size()) - 1 - 2;
                    if (min != -1 && args < min)
                        throw utils::exception::ErrorException(utils::exception::InternalCode::CliParser, "Not enough arguments");
                    else if (max != -1 && args > max)
                        throw utils::exception::ErrorException(utils::exception::InternalCode::CliParser, "Too many arguments");
                    parsedFn(*this, std::vector<std::string>(command.begin() + 1, command.end() - 1));
                } else
                    throw utils::exception::ErrorException(utils::exception::InternalCode::CliExecution, "Command not implemented");
                this->commandMiddlewares.callAfter(command.front());
            }

            // Raw
            else if (rawFound) {
                this->commandMiddlewares.callBefore(command.front());
                if (rawFn) // Check the command existense
                    rawFn(*this, command[1]);
                else
                    throw utils::exception::ErrorException(utils::exception::InternalCode::CliExecution, "Command not implemented");
                this->commandMiddlewares.callAfter(command.front());
            }

            // Error
            else {
                if (this->_flags & utils::cli::Flag::HINT) {
                    std::cout << "Did you mean '" << get_hint<3>(commands, command.front()) << "'?" << std::endl;
                    status = 4;
                    lastExceptionInfo = "Unknown command";
                } else {
                    throw utils::exception::ErrorException(utils::exception::InternalCode::CliExecution, "Unknown command");
                }
            }
        } catch (const utils::exception::ErrorException& e) {
            lastExceptionInfo = e.info();
            if (!(this->_flags & utils::cli::Flag::CATCH)) throw;
            if (e.getCode() == utils::exception::InternalCode::CliParser) status = 1;
            else if (lastExceptionInfo == "Unknown command" || lastExceptionInfo == "Command not implemented") status = 2;
            else if (e.getCode() == utils::exception::InternalCode::MiddlewareCall) { // a middleware isn't a callback (code 3)
                status = 3;
                middlewareError = true;
                lastExceptionInfo = std::string(e.what()) + ": " + lastExceptionInfo;
            } else {
                status = 3;
                lastExceptionInfo = std::string("Callback exception: ") + lastExceptionInfo;
            }
        } catch (const utils::exception::NoneException&) {
            throw;
        } catch (const std::exception& e) {
            lastExceptionInfo = std::string("Callback exception: ") + e.what();
            if (!(this->_flags & utils::cli::Flag::CATCH))
                throw utils::exception::ErrorException(utils::exception::InternalCode::CliExecution, lastExceptionInfo);
            status = 3;
        }

        // Error handling
        if (status != 0) {
            // Display error
            switch (status) {
                case 1: std::cerr << lastExceptionInfo << std::endl; break; // Parser
                case 2: std::cerr << lastExceptionInfo << std::endl; break; // Execution
                case 3: std::cerr << lastExceptionInfo << std::endl; break; // Other (already prefixed)
                case 4: break; // For error with no display
                default: // Unknown
                    throw utils::exception::ErrorException(utils::exception::InternalCode::CliExecution, "Callback exception: can't determine the error");
            }

            // Update internal code
            if (middlewareError) this->_code = 3;
            else if (lastExceptionInfo == "Not enough arguments") this->_code = 124;
            else if (lastExceptionInfo == "Too many arguments") this->_code = 125;
            else if (lastExceptionInfo == "Unclosed escape sequence") this->_code = 126;
            else if (lastExceptionInfo == "Unknown command") this->_code = 128;
            else if (lastExceptionInfo == "Command not implemented") this->_code = 129;
            else if (lastExceptionInfo.starts_with("Callback exception: ")) this->_code = 130;
            else _unlikely {this->_code = 255;}
        } else this->_code = 0;
        lastStatus = status;
    }

    this->execMiddlewares.callAfter(parsedInput);
}

_cold _nodiscard std::string utils::cli::Cli::strcode(std::uint8_t code) const
{
    switch (code) {
        case 0:   return "OK";
        case 1:   return "Internal error";
        case 2:   return "Hook internal error or missing";
        case 3:   return "Middleware internal error";
        case 34:  return "There is allways a r34";
        case 42:  return "case 42";
        case 124: return "Not enough arguments";
        case 125: return "Too many arguments";
        case 126: return "Unclosed escape sequence";
        case 127: return "Empty input";
        case 128: return "Unknown command";
        case 129: return "Command not implemented";
        case 130: return "Callback exception";
        case 255: return "Undefined error";
        default:  return "No errors are associated with this code";
    }

    // Uuhhhhh?????????????
    return "Some dark shit is happening here";
}
