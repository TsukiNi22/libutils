/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 01/06/2026 by @author Tsukini

File Name:
##  @file Setup.cpp

File Description:
##  Setup functions of the cli
\**************************************************************/

#define NO_OUTDATED_WARNING
#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/NoneException.hpp"
#include "utils/write/ANSI.hpp"
#include "utils/write/format.hpp"
#include "utils/cli/Cli.hpp"
#include <termios.h>
#include <functional>
#include <iostream>
#include <cstdint>
#include <csignal>
#include <vector>
#include <format>
#include <string>
#include <tuple>

utils::cli::Cli::Cli(bool sig)
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
}

/* Default commands
 * help -> display commands help
 * bye == quit == exit -> exit the cli
*/
static void help()
{
    std::cout
    << "help:" << std::endl
    << " help\t- Display commands help" << std::endl
    << " bye\t- Exit the cli" << std::endl
    << " quit\t- Exit the cli" << std::endl
    << " exit\t- Exit the cli" << std::endl
    << " ?\t- Display the precedent return code" << std::endl;
}

static void exit()
{
    throw utils::exception::NoneException(utils::exception::Code::Exit);
}
static void bye() {exit();}
static void quit() {exit();}

static void displayCode(const utils::cli::Cli& cli)
{
    std::uint8_t code = cli.getCode();
    std::cout << utils::write::strong();
    if (code == 0) std::cout << utils::write::color_rgb(0, 255, 0) << "✔ ";
    else std::cout << utils::write::color_rgb(255, 80, 80) << "✖ ";
    std::cout << utils::write::format(std::format("<><strong>[{:03}]<>", code));
    std::cout << utils::write::format("<strong>➤ ");
    std::cout << utils::write::color_rgb(0, 200, 200) << cli.strcode(code) << utils::write::reset();
    std::cout << std::endl << std::flush;
}

void utils::cli::Cli::resetCommands()
{
    std::unique_lock lock(this->_commandsLock);
    this->_parsedCommands.clear();
    this->_rawCommands.clear();

    // Parsed commands
    using FnVec = std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>;
    this->_parsedCommands["help"] = std::make_tuple(FnVec([](unused const utils::cli::Cli& cli, unused const std::vector<std::string>& inputs){help();}), 0, 0);
    this->_parsedCommands["bye"]  = std::make_tuple(FnVec([](unused const utils::cli::Cli& cli, unused const std::vector<std::string>& inputs){bye();}), 0, 0);
    this->_parsedCommands["quit"] = std::make_tuple(FnVec([](unused const utils::cli::Cli& cli, unused const std::vector<std::string>& inputs){quit();}), 0, 0);
    this->_parsedCommands["exit"] = std::make_tuple(FnVec([](unused const utils::cli::Cli& cli, unused const std::vector<std::string>& inputs){exit();}), 0, 0);
    this->_parsedCommands["?"] = std::make_tuple(FnVec([](const utils::cli::Cli& cli, unused const std::vector<std::string>& inputs){displayCode(cli);}), 0, 0);

    // Raw commands
    using FnStr = std::function<void(const utils::cli::Cli&, const std::string&)>;
    this->_rawCommands["help"] = FnStr([](unused const utils::cli::Cli& cli, unused const std::string& input){help();});
    this->_rawCommands["bye"]  = FnStr([](unused const utils::cli::Cli& cli, unused const std::string& input){bye();});
    this->_rawCommands["quit"] = FnStr([](unused const utils::cli::Cli& cli, unused const std::string& input){quit();});
    this->_rawCommands["exit"] = FnStr([](unused const utils::cli::Cli& cli, unused const std::string& input){exit();});
    this->_rawCommands["?"] = FnStr([](const utils::cli::Cli& cli, unused const std::string& input){displayCode(cli);});
}

void utils::cli::Cli::resetHooks()
{
    this->resetPromptHook();
    this->resetParserHook();
    this->resetGetCHook();
}

void utils::cli::Cli::resetMiddlewares()
{
    this->cliMiddlewares.clear();
    this->errorMiddlewares.clear();
    this->promptMiddlewares.clear();
    this->inputMiddlewares.clear();
    this->parserMiddlewares.clear();
    this->execMiddlewares.clear();
    this->commandMiddlewares.clear();
}

void utils::cli::Cli::clearCommands()
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
