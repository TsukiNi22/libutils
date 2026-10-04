/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 15/09/2026 by @author Tsukini

File Name:
##  @file Setup.cpp

File Description:
##  Setup functions of the cli
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/NoneException.hpp"
#include "utils/manip/iomanip/ANSI.hpp"
#include "utils/manip/smanip/format.hpp"
#include "utils/cli/Cli.hpp"
#include <termios.h>
#include <unistd.h>
#include <shared_mutex>
#include <functional>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <csignal>
#include <vector>
#include <format>
#include <string>
#include <tuple>

utils::cli::Cli::Cli(const bool sig)
: _sig{sig}
{
    // Setup initial values
    this->resetCommands();
    this->resetHooks();
    this->resetMiddlewares();

    // Setup the term
    if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &this->_orig) == 0) {
        termios raw = this->_orig;
        raw.c_lflag &= ~static_cast<tcflag_t>(ICANON | ECHO);
        this->_termios = (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0);
    }

    // Signal handling (keep the previous handlers to restore them)
    if (this->_sig) {
        this->_oldSigInt = std::signal(SIGINT, SIG_IGN); // ctrl+c
        this->_oldSigTstp = std::signal(SIGTSTP, SIG_IGN); // ctrl+z
    }

    // Load history from persistent storage
    this->loadHistory_();
}

utils::cli::Cli::~Cli()
{
    // Stop a running cli (thread) before destroying what it uses
    if (this->_running) {
        this->kill();
        this->join();
    }

    // Reset the term (only if it was setup)
    if (this->_termios)
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &this->_orig);

    // Reset signal (previous handlers)
    if (this->_sig) {
        std::signal(SIGINT, this->_oldSigInt == SIG_ERR ? SIG_DFL : this->_oldSigInt); // ctrl+c
        std::signal(SIGTSTP, this->_oldSigTstp == SIG_ERR ? SIG_DFL : this->_oldSigTstp); // ctrl+z
    }

    // Save history if persistent is enable
    if (this->_flags & utils::cli::Flag::PERSISTENT)
        this->saveHistory_();
}

_cold _nodiscard static std::filesystem::path get_history_file_path(void)
{
    const char* home = std::getenv("HOME");
    if (!home) home = ".";
    return std::filesystem::path(home) / HISTORY_FILE;
}

_cold void utils::cli::Cli::loadHistory_(void)
{
    std::unique_lock lock(this->_historyLock);
    const std::filesystem::path path = get_history_file_path();

    // Open the file
    std::ifstream file(path);
    if (!file.is_open())
        return; // Silent fail

    // Read the file and put them into the array
    std::string line;
    while (std::getline(file, line))
        if (!line.empty()) this->_history.push_back(line);
}

_cold void utils::cli::Cli::saveHistory_(void)
{
    std::shared_lock lock(this->_historyLock);
    const std::filesystem::path path = get_history_file_path();

    // Open the file
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open())
        return; // Silent fail

    // Write on the file from the history
    const std::size_t start = (this->_history.size() > HISTORY_LIMITS) ? this->_history.size() - HISTORY_LIMITS : 0;
    for (std::size_t i = start; i < this->_history.size(); ++i)
        file << this->_history[i] << '\n';
}

/* Default commands
 * help -> display commands help
 * bye == quit == exit -> exit the cli
*/
_cold static void help(void)
{
    std::cout
    << "help:" << std::endl
    << " help\t- Display commands help" << std::endl
    << " bye\t- Exit the cli" << std::endl
    << " quit\t- Exit the cli" << std::endl
    << " exit\t- Exit the cli" << std::endl
    << " ?\t- Display the precedent return code" << std::endl;
}

_cold _noreturn static void exit(void)
{
    throw utils::exception::NoneException(utils::exception::InternalCode::Exit);
}
_cold _noreturn static void bye(void)  {exit();};
_cold _noreturn static void quit(void) {exit();};

_cold static void displayCode(const utils::cli::Cli& cli)
{
    std::uint8_t code = cli.getCode();
    std::cout << utils::iomanip::strong();
    if (code == 0) std::cout << utils::iomanip::color_rgb(0, 255, 0) << "✔ ";
    else std::cout << utils::iomanip::color_rgb(255, 80, 80) << "✖ ";
    std::cout << utils::smanip::format(std::format("<><strong>[{:03}]<>", code));
    std::cout << utils::smanip::format("<strong>➤ ");
    std::cout << utils::iomanip::color_rgb(0, 200, 200) << cli.strcode(code) << utils::iomanip::reset();
    std::cout << std::endl << std::flush;
}

_cold void utils::cli::Cli::resetCommands(void)
{
    std::unique_lock lock(this->_commandsLock);
    this->_parsedCommands.clear();
    this->_rawCommands.clear();

    // Parsed commands
    this->_parsedCommands["help"] = std::make_tuple(std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>([](_unused const utils::cli::Cli& cli, _unused const std::vector<std::string>& inputs) {help();}), 0, 0);
    this->_parsedCommands["bye"]  = std::make_tuple(std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>([](_unused const utils::cli::Cli& cli, _unused const std::vector<std::string>& inputs) {bye();}), 0, 0);
    this->_parsedCommands["quit"] = std::make_tuple(std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>([](_unused const utils::cli::Cli& cli, _unused const std::vector<std::string>& inputs) {quit();}), 0, 0);
    this->_parsedCommands["exit"] = std::make_tuple(std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>([](_unused const utils::cli::Cli& cli, _unused const std::vector<std::string>& inputs) {exit();}), 0, 0);
    this->_parsedCommands["?"]    = std::make_tuple(std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>([](const utils::cli::Cli& cli, _unused const std::vector<std::string>& inputs) {displayCode(cli);}), 0, 0);

    // Raw commands
    this->_rawCommands["help"] = std::function<void(const utils::cli::Cli&, const std::string&)>([](_unused const utils::cli::Cli& cli, _unused const std::string& input) {help();});
    this->_rawCommands["bye"]  = std::function<void(const utils::cli::Cli&, const std::string&)>([](_unused const utils::cli::Cli& cli, _unused const std::string& input) {bye();});
    this->_rawCommands["quit"] = std::function<void(const utils::cli::Cli&, const std::string&)>([](_unused const utils::cli::Cli& cli, _unused const std::string& input) {quit();});
    this->_rawCommands["exit"] = std::function<void(const utils::cli::Cli&, const std::string&)>([](_unused const utils::cli::Cli& cli, _unused const std::string& input) {exit();});
    this->_rawCommands["?"]    = std::function<void(const utils::cli::Cli&, const std::string&)>([](const utils::cli::Cli& cli, _unused const std::string& input) {displayCode(cli);});
}

_cold void utils::cli::Cli::resetHooks(void)
{
    this->resetPromptHook();
    this->resetParserHook();
    this->resetGetCHook();
}

_cold void utils::cli::Cli::resetMiddlewares(void)
{
    this->cliMiddlewares.clear();
    this->errorMiddlewares.clear();
    this->promptMiddlewares.clear();
    this->inputMiddlewares.clear();
    this->parserMiddlewares.clear();
    this->execMiddlewares.clear();
    this->commandMiddlewares.clear();
}

_cold void utils::cli::Cli::clearCommands(void)
{
    std::unique_lock lock(this->_commandsLock);
    this->_parsedCommands.clear();
    this->_rawCommands.clear();
}

_cold void utils::cli::Cli::delCommand(const std::string& command)
{
    std::unique_lock lock(this->_commandsLock);
    this->_parsedCommands.erase(command);
    this->_rawCommands.erase(command);
}

_cold void utils::cli::Cli::delCommands(const std::vector<std::string>& commands)
{
    std::unique_lock lock(this->_commandsLock);
    for (const std::string& command: commands) {
        this->_parsedCommands.erase(command);
        this->_rawCommands.erase(command);
    }
}
