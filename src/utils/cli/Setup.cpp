/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 25/07/2026 by @author Tsukini

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
#include <shared_mutex>
#include <functional>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
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
    if (isatty(STDIN_FILENO)) {
        tcgetattr(STDIN_FILENO, &this->_orig);
        termios raw = this->_orig;
        raw.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    // Signal handling
    if (this->_sig) {
        std::signal(SIGINT, SIG_IGN); // ctrl+c
        std::signal(SIGTSTP, SIG_IGN); // ctrl+z
    }

    // Load history from persitent storage
    this->loadHistory();
}

utils::cli::Cli::~Cli()
{
    // Reset the term
    if (isatty(STDIN_FILENO))
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &this->_orig);

    // Reset signal
    if (this->_sig) {
        std::signal(SIGINT, SIG_DFL); // ctrl+c
        std::signal(SIGTSTP, SIG_DFL); // ctrl+z
    }

    // Save history if persitent is enable
    if (this->_flags & utils::cli::Flag::PERSISTENT)
        this->saveHistory();
}

static std::filesystem::path getHistoryFilePath(void)
{
    const char* home = std::getenv("HOME");
    if (!home) home = ".";
    return std::filesystem::path(home) / HISTORY_FILE;
}

void utils::cli::Cli::loadHistory(void)
{
    std::unique_lock lock(this->_historyLock);
    const std::filesystem::path path = getHistoryFilePath();

    // Open the file
    std::ifstream file(path);
    if (!file.is_open())
        return; // Silent fail

    // Read the file and put them into the array
    std::string line;
    while (std::getline(file, line))
        if (!line.empty()) this->_history.push_back(line);
}

void utils::cli::Cli::saveHistory(void)
{
    std::shared_lock lock(this->_historyLock);
    const std::filesystem::path path = getHistoryFilePath();

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
static void help(void)
{
    std::cout
    << "help:" << std::endl
    << " help\t- Display commands help" << std::endl
    << " bye\t- Exit the cli" << std::endl
    << " quit\t- Exit the cli" << std::endl
    << " exit\t- Exit the cli" << std::endl
    << " ?\t- Display the precedent return code" << std::endl;
}

static void exit(void)
{
    throw utils::exception::NoneException(utils::exception::Code::Exit);
}
static void bye(void) {exit();}
static void quit(void) {exit();}

static void displayCode(const utils::cli::Cli& cli)
{
    std::uint8_t code = cli.getCode();
    std::cout << utils::iomanip::strong();
    if (code == 0) std::cout << utils::iomanip::color_rgb(0, 255, 0) << "✔ ";
    else std::cout << utils::iomanip::color_rgb(255, 80, 80) << "✖ ";
    std::cout << utils::iomanip::format(std::format("<><strong>[{:03}]<>", code));
    std::cout << utils::iomanip::format("<strong>➤ ");
    std::cout << utils::iomanip::color_rgb(0, 200, 200) << cli.strcode(code) << utils::iomanip::reset();
    std::cout << std::endl << std::flush;
}

void utils::cli::Cli::resetCommands(void)
{
    std::unique_lock lock(this->_commandsLock);
    this->_parsedCommands.clear();
    this->_rawCommands.clear();

    // Parsed commands
    using FnVec = std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>;
    this->_parsedCommands["help"] = std::make_tuple(FnVec([](_unused const utils::cli::Cli& cli, _unused const std::vector<std::string>& inputs){help();}), 0, 0);
    this->_parsedCommands["bye"]  = std::make_tuple(FnVec([](_unused const utils::cli::Cli& cli, _unused const std::vector<std::string>& inputs){bye();}), 0, 0);
    this->_parsedCommands["quit"] = std::make_tuple(FnVec([](_unused const utils::cli::Cli& cli, _unused const std::vector<std::string>& inputs){quit();}), 0, 0);
    this->_parsedCommands["exit"] = std::make_tuple(FnVec([](_unused const utils::cli::Cli& cli, _unused const std::vector<std::string>& inputs){exit();}), 0, 0);
    this->_parsedCommands["?"] = std::make_tuple(FnVec([](const utils::cli::Cli& cli, _unused const std::vector<std::string>& inputs){displayCode(cli);}), 0, 0);

    // Raw commands
    using FnStr = std::function<void(const utils::cli::Cli&, const std::string&)>;
    this->_rawCommands["help"] = FnStr([](_unused const utils::cli::Cli& cli, _unused const std::string& input){help();});
    this->_rawCommands["bye"]  = FnStr([](_unused const utils::cli::Cli& cli, _unused const std::string& input){bye();});
    this->_rawCommands["quit"] = FnStr([](_unused const utils::cli::Cli& cli, _unused const std::string& input){quit();});
    this->_rawCommands["exit"] = FnStr([](_unused const utils::cli::Cli& cli, _unused const std::string& input){exit();});
    this->_rawCommands["?"] = FnStr([](const utils::cli::Cli& cli, _unused const std::string& input){displayCode(cli);});
}

void utils::cli::Cli::resetHooks(void)
{
    this->resetPromptHook();
    this->resetParserHook();
    this->resetGetCHook();
}

void utils::cli::Cli::resetMiddlewares(void)
{
    this->cliMiddlewares.clear();
    this->errorMiddlewares.clear();
    this->promptMiddlewares.clear();
    this->inputMiddlewares.clear();
    this->parserMiddlewares.clear();
    this->execMiddlewares.clear();
    this->commandMiddlewares.clear();
}

void utils::cli::Cli::clearCommands(void)
{
    std::unique_lock lock(this->_commandsLock);
    this->_parsedCommands.clear();
    this->_rawCommands.clear();
}

void utils::cli::Cli::delCommand(const std::string& command)
{
    std::unique_lock lock(this->_commandsLock);
    this->_parsedCommands.erase(command);
    this->_rawCommands.erase(command);
}

void utils::cli::Cli::delCommands(const std::vector<std::string>& commands)
{
    std::unique_lock lock(this->_commandsLock);
    for (const std::string& command: commands) {
        this->_parsedCommands.erase(command);
        this->_rawCommands.erase(command);
    }
}
