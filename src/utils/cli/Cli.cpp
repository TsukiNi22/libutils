/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 31/05/2026 by @author Tsukini

File Name:
##  @file Cli.cpp

File Description:
##  Definition of the main cli functions
\**************************************************************/

#define NO_OUTDATED_WARNING
#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/WarningException.hpp"
#include "utils/exception/basic/NoneException.hpp"
#include "utils/exception/custom/CustomException.hpp"
#include "utils/algorithms/c2dmp-hsm/c2dmp-hsm.hpp"
#include "utils/write/ANSI.hpp"
#include "utils/cli/Cli.hpp"
#include "utils/cli/Flags.hpp"
#include <unistd.h>
#include <unordered_map>
#include <functional>
#include <exception>
#include <algorithm>
#include <optional>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <thread>
#include <vector>
#include <string>
#include <tuple>

static std::string trim(const std::string& s)
{
    std::size_t start = s.find_first_not_of(" ");
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(" ");
    return s.substr(start, end - start + 1);
}

static void deleteChars(std::size_t n)
{
    for (std::size_t i = 0; i < n; ++i)
        std::cout << "\b \b";
    std::cout << std::flush;
}

void utils::cli::Cli::launch(std::size_t call)
{
    try {this->cliMiddlewares.callBefore();}
    catch (const utils::exception::CustomException& e) {std::cout << e.what() << ": " << e.info() << std::endl;}
    std::string input;

    // Main loop
    try {
        for (std::size_t i = 0; (!(this->_flags & utils::cli::Flag::MANUAL) || i < call) && !this->_interrupted; ++i) {
            // Display prompt
            if (this->_flags & utils::cli::Flag::PROMPT) {
                try {
                    this->prompt();
                } catch (const utils::exception::CustomException& e) {
                    utils::exception::Code code = e.getCode();
                    if (code == utils::exception::Code::CliHook) this->_code = 2;
                    else unlikely {this->_code = 255;}
                    try {this->errorMiddlewares.callBefore(this->_code);}
                    catch (const utils::exception::CustomException& e) {std::cout << e.what() << ": " << e.info() << std::endl;}
                    if (this->_code <= 3 || this->_code == 255) std::cout << e.what() << ": " << e.info() << std::endl;
                    else std::cout << e.info() << std::endl;
                    try {this->errorMiddlewares.callAfter(this->_code);}
                    catch (const utils::exception::CustomException& e) {std::cout << e.what() << ": " << e.info() << std::endl;}
                    continue;
                }
            }

            // Get input & Check if it's empty
            try {
                input = this->getInput();
                if (this->_interrupted) { // Check for interrupt
                    this->_running = false;
                    return;
                }
                this->_history.push_back(input);
            } catch (const utils::exception::CustomException& e) {
                utils::exception::Code code = e.getCode();
                if (code == utils::exception::Code::CliInternal) this->_code = 1;
                else if (code == utils::exception::Code::CliHook) this->_code = 2;
                else if (code == utils::exception::Code::MiddlewareCall) this->_code = 3;
                else unlikely {this->_code = 255;}
                try {this->errorMiddlewares.callBefore(this->_code);}
                catch (const utils::exception::CustomException& e) {std::cout << e.what() << ": " << e.info() << std::endl;}
                if (this->_code <= 3 || this->_code == 255) std::cout << e.what() << ": " << e.info() << std::endl;
                else std::cout << e.info() << std::endl;
                try {this->errorMiddlewares.callAfter(this->_code);}
                catch (const utils::exception::CustomException& e) {std::cout << e.what() << ": " << e.info() << std::endl;}
                continue;
            }
            if (!(this->_flags & utils::cli::Flag::EMPTY_INPUT) && input.empty()) {
                this->_code = 127;
                try {this->errorMiddlewares.callBefore(this->_code);}
                catch (const utils::exception::CustomException& e) {std::cout << e.what() << ": " << e.info() << std::endl;}
                std::cout << "Empty input" << std::endl;
                try {this->errorMiddlewares.callAfter(this->_code);}
                catch (const utils::exception::CustomException& e) {std::cout << e.what() << ": " << e.info() << std::endl;}
                continue;
            }

            // Parse & Execute input
            try {
                this->exec(this->parse(input));
            } catch (const utils::exception::CustomException& e) {
                utils::exception::Code code = e.getCode();
                if (code == utils::exception::Code::CliHook) this->_code = 2;
                else if (code == utils::exception::Code::MiddlewareCall) this->_code = 3;
                else if (code == utils::exception::Code::CliParser) {
                    std::string info = e.info();
                    if (info == "Not enough arguments") this->_code = 124;
                    else if (info == "Too many arguments") this->_code = 125;
                    else if (info == "Unclosed escape sequence") this->_code = 126;
                } else if (code == utils::exception::Code::CliExecution) {
                    std::string info = e.info();
                    if (info == "Unknow command") this->_code = 128;
                    else if (info == "Command not implemented") this->_code = 129;
                    else if (info.starts_with("Callback exception: ")) {
                        if (!(this->_flags & utils::cli::Flag::CATCH)) {
                            this->_running = false;
                            throw;
                        }
                        this->_code = 130;
                    }
                } else unlikely {this->_code = 255;}
                try {this->errorMiddlewares.callBefore(this->_code);}
                catch (const utils::exception::CustomException& e) {std::cout << e.what() << ": " << e.info() << std::endl;}
                if (this->_code <= 3 || this->_code == 255) std::cout << e.what() << ": " << e.info() << std::endl;
                else std::cout << e.info() << std::endl;
                try {this->errorMiddlewares.callAfter(this->_code);}
                catch (const utils::exception::CustomException& e) {std::cout << e.what() << ": " << e.info() << std::endl;}
                continue;
            }
        }
    } catch (const utils::exception::NoneException& e) { // Catch exit throw
        if (e.getCode() != utils::exception::Code::Exit) {
            this->_running = false;
            throw;
        }
    }

    try {this->cliMiddlewares.callAfter();}
    catch (const utils::exception::CustomException& e) {std::cout << e.what() << ": " << e.info() << std::endl;}
    this->_running = false;
}

std::optional<std::thread> utils::cli::Cli::start(std::size_t call)
{
    // Check status
    if (this->_running)
        throw utils::exception::WarningException(utils::exception::Code::CliAlreadyRunning);
    this->_interrupted = false; // Reset interrupt status
    this->_running = true;

    // Launch in a thread
    if (this->_flags & utils::cli::Flag::THREAD) {
        // Create the thread
        std::thread t([this, call] {
            this->launch(call);
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
    this->launch(call);
    return std::nullopt;
}

std::optional<std::thread> utils::cli::Cli::start(const std::string& input, std::size_t call)
{
    this->_initInput.push(input);
    return this->start(call);
}

std::optional<std::thread> utils::cli::Cli::start(const std::vector<std::string>& inputs, std::size_t call)
{
    for (const std::string& input: inputs)
        this->_initInput.push(input);
    return this->start(call);
}

hot void utils::cli::Cli::prompt()
{
    this->promptMiddlewares.callBefore();

    // Call the hook
    if (isatty(STDOUT_FILENO)) {
        if (this->_promptHook) likely {
            try {
                std::lock_guard lock(this->_hooksLock);
                this->_promptHook(*this, this->_code);
            } catch (const std::exception& e) {
                throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliHook, e.what());
            }
        } else unlikely {
            throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliHook, "Missing hook for prompting");
        }
    }

    this->promptMiddlewares.callAfter();
}

template<std::size_t PREFIX_DEPTH>
static std::string getHint(const std::vector<std::string>& list, const std::string& s)
{
    float min_dist = 0.0f, dist = 0.0f;
    bool first = true;
    std::string hint;

    // Check for each
    for (const std::string& actual: list) {
        dist = utils::algorithms::c2dmp::c2dmp<PREFIX_DEPTH>(s, actual);
        if (first || dist < min_dist) {
            min_dist = dist;
            hint = actual;
            first = false;
        }
    }

    return hint;
}

hot nodiscard std::string utils::cli::Cli::getInput()
{
    std::vector<std::string> commands;
    bool echo = !(this->_flags & utils::cli::Flag::NOECHO);
    std::size_t indexBuffer = 0, indexHistory = this->_history.size();
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
    while (true && !this->_interrupted) {
        // Get the char
        if (this->_getcHook) likely {
            try {
                this->inputMiddlewares.callBefore();
                std::lock_guard lock(this->_hooksLock);
                while (!this->_getcHook(c) && !this->_interrupted)
                    std::this_thread::yield();
                this->inputMiddlewares.callAfter(c);
            } catch (const std::exception& e) {
                //throw utils::exception::NoneException(utils::exception::Code::Exit);
                throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliHook, e.what());
            }
        } else unlikely {
            throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliHook, "Missing hook for char getter");
        }
        if (this->_interrupted) break; // Check for interrupt

        // EOF handling
        if (c == 0) {
            if (isatty(STDOUT_FILENO))
                std::cout << std::endl << "Detected EOF, exiting..." << std::endl;
            throw utils::exception::NoneException(utils::exception::Code::Exit);
        }

        // Ctrl+D handling
        if (c == 4) {
            if (isatty(STDOUT_FILENO))
                std::cout << std::endl << "Detected Ctrl+D, exiting..." << std::endl;
            throw utils::exception::NoneException(utils::exception::Code::Exit);
        }

        // Auto completion
        else if (this->_flags & utils::cli::Flag::AUTO_COMPLETION && c == '\t' && trim(input).find(" ", 0) == std::string::npos) {
            std::shared_lock lock(this->_commandsLock);

            // Setup the commands
            commands.clear();
            if (this->_flags & utils::cli::Flag::PARSED) {
                for (const auto& [cmd, tup] : this->_parsedCommands)
                    commands.push_back(cmd);
            } else {
                for (const auto& [cmd, fn] : this->_rawCommands)
                    commands.push_back(cmd);
            }

            // Get the hint
            input = getHint<2>(commands, input);
            indexBuffer = input.size();
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
        else if (std::isprint(c)) likely {
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
                std::cout << utils::write::right(lastInputSize - lastIndexBuffer);

            // Refresh input display
            deleteChars(lastInputSize);
            std::cout << input;
            lastInputSize = input.size();
            lastIndexBuffer = indexBuffer;

            // Place the cursor
            if (input.size() - indexBuffer > 0)
                std::cout << utils::write::left(input.size() - indexBuffer);
        }

        // Update output
        std::cout << std::flush;
    }

    if (this->_interrupted) {
        if (isatty(STDOUT_FILENO)) std::cout << std::endl;
        this->_input = input;
        return "";
    }
    if (this->_flags & utils::cli::Flag::TRIM) return trim(input);
    else return input;
}

hot utils::cli::ParsedData utils::cli::Cli::parse(const std::string& input)
{
    this->parserMiddlewares.callBefore(input);
    utils::cli::ParsedData parsedInput;

    // Call the hook
    if (this->_parserHook) likely {
        try {
            std::lock_guard lock(this->_hooksLock);
            parsedInput = this->_parserHook(input,
                this->_flags & utils::cli::Flag::TRIM,
                this->_flags & utils::cli::Flag::LOGIC,
                this->_flags & utils::cli::Flag::PARSED
            );
        } catch (const std::exception& e) {
            throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliHook, e.what());
        }
    } else unlikely {
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliHook, "Missing hook for parsing");
    }

    this->parserMiddlewares.callAfter(parsedInput);
    return parsedInput;
}

hot void utils::cli::Cli::exec(const utils::cli::ParsedData& parsedInput)
{
    this->execMiddlewares.callBefore(parsedInput);
    std::unordered_map<std::string, std::tuple<std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>, std::int16_t, std::int16_t>>::iterator itParsed;
    std::unordered_map<std::string, std::function<void(const utils::cli::Cli&, const std::string&)>>::iterator itRaw;
    std::string lastExceptionInfo;
    std::vector<std::string> commands;
    std::uint8_t status = 0;

    // For each commands
    for (std::size_t i = 0; i < parsedInput.size(); ++i) {
        const std::vector<std::string>& command = parsedInput[i];

        // Try to exec the command
        status = 0;
        try {
            std::shared_lock lock(this->_commandsLock);

            // Try to find the command
            itParsed = this->_parsedCommands.find(command.front());
            itRaw = this->_rawCommands.find(command.front());

            // Parsed
            if (this->_flags & utils::cli::Flag::PARSED && itParsed != this->_parsedCommands.end()) {
                this->commandMiddlewares.callBefore(command.front());
                if (std::get<0>(itParsed->second)) { // Check the command existense
                    // Check commands arguments number
                    if (std::get<1>(itParsed->second) == -1 || static_cast<std::int16_t>(command.size()) < std::get<1>(itParsed->second) + 1 + 2)
                        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliParser, "Not enough arguments");
                    else if (std::get<2>(itParsed->second) == -1 || static_cast<std::int16_t>(command.size()) > std::get<2>(itParsed->second) + 1 + 2)
                        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliParser, "Too many arguments");
                    std::get<0>(itParsed->second)(*this, std::vector<std::string>(command.begin() + 1, command.end() - 1));
                } else
                    throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliExecution, "Command not implemented");
                this->commandMiddlewares.callAfter(command.front());
            }

            // Raw
            else if (!(this->_flags & utils::cli::Flag::PARSED) && itRaw != this->_rawCommands.end()) {
                this->commandMiddlewares.callBefore(command.front());
                if (itRaw->second) // Check the command existense
                    (itRaw->second)(*this, command[1]);
                else
                    throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliExecution, "Command not implemented");
                this->commandMiddlewares.callAfter(command.front());
            }

            // Error
            else {
                if (this->_flags & utils::cli::Flag::HINT) {
                    commands.clear();
                    if (this->_flags & utils::cli::Flag::PARSED) {
                        for (const auto& [cmd, tup] : this->_parsedCommands)
                            commands.push_back(cmd);
                    } else {
                        for (const auto& [cmd, fn] : this->_rawCommands)
                            commands.push_back(cmd);
                    }
                    std::cout << "Did you mean '" << getHint<3>(commands, command.front()) << "'?" << std::endl;
                    status = 4;
                    lastExceptionInfo = "Unknow command";
                } else {
                    throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliExecution, "Unknow command");
                }
            }
        } catch (const utils::exception::CustomException& e) {
            lastExceptionInfo = e.info();
            if (!(this->_flags & utils::cli::Flag::CATCH)) throw;
            if (e.getCode() == utils::exception::Code::CliParser) status = 1;
            else if (lastExceptionInfo == "Unknow command" || lastExceptionInfo == "Command not implemented") status = 2;
            else status = 3;
        } catch (const utils::exception::NoneException& e) {
            throw;
        } catch (const std::exception& e) {
            lastExceptionInfo = e.what();
            if (!(this->_flags & utils::cli::Flag::CATCH))
                throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliExecution, std::string("Callback exception: ") + lastExceptionInfo);
            status = 3;
        }

        // Error handling
        if (status != 0) {
            // Display error
            switch (status) {
                case 1: std::cout << lastExceptionInfo << std::endl; break; // Parser
                case 2: std::cout << lastExceptionInfo << std::endl; break; // Execution
                case 3: std::cout << "Callback exception: " << lastExceptionInfo << std::endl; break; // Other
                case 4: break; // For error with no display
                default: // Unknow
                    throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::CliExecution, "Callback exception: can't determine the error");
            }

            // Update internal code
            if (lastExceptionInfo == "Not enough arguments") this->_code = 124;
            else if (lastExceptionInfo == "Too many arguments") this->_code = 125;
            else if (lastExceptionInfo == "Unclosed escape sequence") this->_code = 126;
            else if (lastExceptionInfo == "Unknow command") this->_code = 128;
            else if (lastExceptionInfo == "Command not implemented") this->_code = 129;
            else if (lastExceptionInfo.starts_with("Callback exception: ")) this->_code = 130;
            else unlikely {this->_code = 255;}
        } else this->_code = 0;

        // Check the logic
        if (command.back() == "&&") {
            if (status == 0) continue;
            // Skip to the next ';' or ''
            for (; i < parsedInput.size() && (parsedInput[i].back() == "&&" || parsedInput[i].back() == "||"); ++i);
        } else if (command.back() == "||") {
            if (status != 0) continue;
            // Skip to the next ';' or ''
            for (; i < parsedInput.size() && (parsedInput[i].back() == "&&" || parsedInput[i].back() == "||"); ++i);
        }
    }

    this->execMiddlewares.callAfter(parsedInput);
}

std::string utils::cli::Cli::strcode(std::uint8_t code) const
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
        case 128: return "Unknow command";
        case 129: return "Command not implemented";
        case 130: return "Callback exception";
        case 255: return "Undefined error";
        default:  return "No errors are associated with this code";
    }

    // Uuhhhhh?????????????
    return "Some dark shit is happening here";
}
