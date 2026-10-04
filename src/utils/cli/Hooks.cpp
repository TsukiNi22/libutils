/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 06/07/2026 by @author Tsukini

File Name:
##  @file Hooks.cpp

File Description:
##  Default hooks used by the cli
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/cli/Cli.hpp"
#include <unistd.h>
#include <poll.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <cerrno>
#include <string>
#include <regex>

_hot _nodiscard static std::string trim_string(const std::string& s)
{
    std::size_t start = s.find_first_not_of(" ");
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(" ");
    return s.substr(start, end - start + 1);
}

_hot void utils::cli::defaultPromptHook(_unused const utils::cli::Cli& cli, _unused std::uint8_t code)
{
    std::cout << "> " << std::flush;
}

_hot _nodiscard static std::vector<std::string> split_commands(const std::string& input)
{
    std::vector<std::string> commands;
    std::regex re(R"((.*?(?:&&|\|\||;)))");
    std::sregex_iterator begin(input.begin(), input.end(), re);
    std::size_t index = 0;

    // For each match
    for (auto it = begin; it != std::sregex_iterator(); ++it) {
        commands.push_back(it->str());
        index += static_cast<std::size_t>(it->length());
    }

    // Last commands in string
    if (index < input.size())
        commands.push_back(input.substr(index));

    return commands;
}

_hot _nodiscard static std::vector<std::string> split_command(const std::string& command)
{
    std::vector<std::string> splited;
    std::string token;

    for (std::size_t i = 0; i < command.size(); ++i) {
        char c = command[i];

        // White space
        if (c == ' ' || c == '\t') {
            if (!token.empty()) {
                splited.push_back(token);
                token.clear();
            }
        }

        // Separator "&&", "||", ";"
        else if (c == '&' && i + 1 < command.size() && command[i + 1] == '&') {
            if (!token.empty()) {
                splited.push_back(token);
                token.clear();
            }
            splited.push_back("&&");
            ++i; // skip second char
        } else if (c == '|' && i + 1 < command.size() && command[i + 1] == '|') {
            if (!token.empty()) {
                splited.push_back(token);
                token.clear();
            }
            splited.push_back("||");
            ++i; // skip second char
        } else if (c == ';') {
            if (!token.empty()) {
                splited.push_back(token);
                token.clear();
            }
            splited.push_back(";");
        }

        // Basic char
        else _likely {
            token += c;
        }
    }

    // Last part
    if (!token.empty())
        splited.push_back(token);

    return splited;
}

_hot _nodiscard utils::cli::ParsedData utils::cli::defaultParserHook(const std::string& input, const bool trim, const bool logic, const bool parse)
{
    utils::cli::ParsedData parsedInput;
    std::vector<std::string> commands;
    std::vector<std::string> splited;
    std::vector<std::string> data;
    std::string command;

    // Empty input
    if (input.empty()) return parsedInput;

    // Get the commands separated by '&&', '||' and ';'
    if (logic) commands = split_commands(trim_string(input));
    else commands = {input};

    // Separate each commands args
    for (const std::string& rawCommand: commands) {
        command = (trim ? trim_string(rawCommand) : rawCommand);
        splited = split_command(command);
        if (splited.size() == 0) continue;
        if (parse) {
            data.clear();
            data.push_back(splited.front());
            for (std::size_t i = 0; i < splited.size() - (splited.back() == "&&" || splited.back() == "||" || splited.back() == ";"); ++i)
                data.push_back(splited[i]);
            data.push_back((splited.back() == "&&" || splited.back() == "||" || splited.back() == ";") ? splited.back() : "");
            parsedInput.push_back(data);
        } else {
            if (splited.back() == "&&" || splited.back() == "||" || splited.back() == ";")
                command = command.erase(command.size() - splited.back().size());
            parsedInput.push_back({
                splited.front(),
                command,
                (splited.back() == "&&" || splited.back() == "||" || splited.back() == ";") ? splited.back() : ""
            });
        }
    }

    return parsedInput;
}

_hot _nodiscard bool utils::cli::defaultGetCHook(char& c)
{
    // poll config
    pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;

    // poll call
    int res = poll(&pfd, 1, 0);
    if (res == 0) return false;
    if (res < 0) throw std::runtime_error(strerror(errno));

    // poll -> thing to read
    ssize_t bytes = read(STDIN_FILENO, &c, 1);
    if (bytes < 0)
        throw std::runtime_error(strerror(errno));
    if (bytes == 0) c = 0; // EOF
    return true;
}
