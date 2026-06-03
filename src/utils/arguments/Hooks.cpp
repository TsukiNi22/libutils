/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 03/06/2026 by @author Tsukini

File Name:
##  @file Hooks.cpp

File Description:
##  Default hook used for the cli
\**************************************************************/

#define NO_OUTDATED_WARNING
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/custom/CustomException.hpp"
#include "utils/arguments/ArgParser.hpp"
#include "utils/arguments/ArgParserType.hpp"
#include "utils/write/format.hpp"
#include "utils/write/Color.hpp"
#include "utils/write/ANSI.hpp"
#include "utils/write/Style.hpp"
#include <unordered_map>
#include <filesystem>
#include <exception>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>

void utils::arguments::defaultHelpHook(const utils::arguments::ArgParser& parser)
{
    const std::unordered_map<std::string, utils::arguments::Usage>&  usages  = parser.getUsages();
    const std::unordered_map<std::string, utils::arguments::Flag>&   flags   = parser.getFlags();
    const std::unordered_map<std::string, utils::arguments::Option>& options = parser.getOptions();

    std::cout << utils::write::format("<strong>PROJECT<>") << std::endl;
    std::cout << utils::write::color(utils::write::Color::Cyan) << "\t" << parser.getDescription() << std::endl;
    std::cout << utils::write::reset() << std::endl;

    std::size_t maxNameLen = 0;
    for (const auto& [_, usage]: usages) {
        if (usage.name != "default")
            maxNameLen = std::max(maxNameLen, usage.name.size());
    }
    maxNameLen += 2;

    std::cout << utils::write::format("<strong>USAGE<>") << std::endl;
    std::cout << utils::write::color(utils::write::Color::Magenta);
    bool defaultUsage = false;
    for (const auto& [_, usage]: usages) {
        if (usage.name == "default") {
            defaultUsage = true;
            continue;
        }
        std::cout << "\t";
        if (!usage.name.empty()) std::cout << std::left << std::setw(maxNameLen) << std::format("({})", usage.name) << " -> ";
        std::cout << "./" << parser.getBinary();
        for (const auto& [id, mandatory]: usage.ids) {
            // Options
            if (options.contains(id)) {
                auto it = options.find(id);
                if (it == options.end() || it->second.name.empty()) continue;
                if (!mandatory) std::cout << "[";
                std::cout << "<" << it->second.name << ">";
                if (!mandatory) std::cout << "]";
            }

            // Flags
            else if (flags.contains(id)) {
                auto it = flags.find(id);
                if (it == flags.end()) continue;
                const auto& [fshort, fflag, flong] = it->second.flag;
                if (fshort.empty() && fflag.empty() && flong.empty()) continue;
                if (!mandatory) std::cout << "[";
                std::cout << " " << ((fshort.empty() && fflag.empty()) ? "--" : "-") << (fshort.empty() ? (fflag.empty() ? flong : fflag) : fshort);
                for (const auto& [name, fmandatory, _]: it->second.options)
                    std::cout << (fmandatory ? "" : "[") << " <" << name << ">" << (fmandatory ? "" : "]");
                if (!mandatory) std::cout << "]";
            }

            // Error
            else {
                throw utils::exception::CustomException(utils::exception::Type::Warning, utils::exception::Code::UnknowId, id);
            }
        }
        std::cout << std::endl;
    }
    if (defaultUsage || usages.size() == 0) { // Default usage (all flag authorized, dosen't know option position)
        std::cout << "\t" << std::left << std::setw(maxNameLen) << "(default)" << " -> ./" << parser.getBinary();
        for (const auto& [_, flag]: parser.getFlags()) {
            const auto& [fshort, fflag, flong] = flag.flag;
            if (fshort.empty() && fflag.empty() && flong.empty()) continue;
            std::cout << " " << ((fshort.empty() && fflag.empty()) ? "--" : "-") << (fshort.empty() ? (fflag.empty() ? flong : fflag) : fshort);
            for (const auto& [name, mandatory, _]: flag.options)
                std::cout << (mandatory ? "" : "[") << " <" << name << ">" << (mandatory ? "" : "]");
        }
        std::cout << std::endl;
    }
    std::cout << utils::write::reset() << std::endl;

    std::cout << utils::write::format("<strong>OPTIONS<>") << std::endl;
    for (const auto& [_, option]: options) {
        std::cout << utils::write::color(utils::write::Color::Green) << "\t" << option.name << utils::write::reset() << std::endl;
        std::cout << "\t\t" << option.description << std::endl;
    }
    if (options.size() == 0) std::cout << "\tNothing..." << std::endl;;
    std::cout << utils::write::reset() << std::endl;

    std::cout << utils::write::format("<strong>FLAGS<>") << std::endl;
    for (const auto& [_, flag]: flags) {
        const auto& [fshort, fflag, flong] = flag.flag;
        std::cout << utils::write::color(utils::write::Color::Green) << "\t";
        if (!fshort.empty()) std::cout << "-" << fshort;
        if (!fflag.empty())  std::cout << (fshort.empty() ? "" : ", ") << "-" << fflag;
        if (!flong.empty())  std::cout << ((fshort.empty() && fflag.empty()) ? "" : ", ") << "--" << flong;
        std::cout << utils::write::reset();
        for (const auto& [name, mandatory, _]: flag.options)
            std::cout << (mandatory ? "" : "[") << " <" << utils::write::color(utils::write::Color::Red) << name << utils::write::reset() << ">" << (mandatory ? "" : "]");
        std::cout << utils::write::reset() << std::endl;
        std::cout << "\t\t" << flag.description << std::endl;
    }
    if (flags.size() == 0) std::cout << "\tNothing..." << std::endl;;
    std::cout << utils::write::reset() << std::flush;
}

bool utils::arguments::defaultBoolParsingHook(const std::string& option)
{
    try {
        if (option.empty())
            throw std::invalid_argument("empty");
        return (option == "0" || option == "1" || option == "false" || option == "true");
    } catch (...) {
        return false;
    }
}

bool utils::arguments::defaultInt32ParsingHook(const std::string& option)
{
    try {
        if (option.empty())
            throw std::invalid_argument("empty");
        if (!std::all_of(option.begin(), option.end(), ::isdigit))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        long value = std::stol(option, &pos);
        if (pos != option.size())
            throw std::invalid_argument("invalid number");
        if (value < std::numeric_limits<std::int32_t>::min() ||
            value > std::numeric_limits<std::int32_t>::max())
            throw std::out_of_range("int32 overflow");
        return true;
    } catch (...) {
        return false;
    }
}

bool utils::arguments::defaultSizetParsingHook(const std::string& option)
{
    try {
        if (option.empty())
            throw std::invalid_argument("empty");
        if (!std::all_of(option.begin(), option.end(), ::isdigit))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        (void)std::stoull(option, &pos);
        if (pos != option.size())
            throw std::invalid_argument("not a float");
        return true;
    } catch (...) {
        return false;
    }
}

bool utils::arguments::defaultDoubleParsingHook(const std::string& option)
{
    try {
        if (option.empty())
            throw std::invalid_argument("empty");
        std::size_t pos = 0;
        (void)std::stod(option, &pos);
        if (pos != option.size())
            throw std::invalid_argument("not a float");
        return true;
    } catch (...) {
        return false;
    }
}

bool utils::arguments::defaultFileParsingHook(const std::string& option)
{
    if (!std::filesystem::exists(option) || !std::filesystem::is_regular_file(option))
        return false;
    if (std::filesystem::file_size(option) == 0)
        return false;
    return true;
}

bool utils::arguments::defaultWritableParsingHook(const std::string& option)
{
    
    std::filesystem::path path(option);

    // Explicit directory
    bool isDirectory = option.back() == '/' || option.back() == '\\';
    try {
        // Check if the directory exist
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            isDirectory = true;

        // On directory case
        if (isDirectory) {
            std::filesystem::create_directories(path);
            std::filesystem::path testFile = path / ".TO_DELETE-permission_check_auto_generated_file";
            std::ofstream file(testFile.string());
            if (!file) return false;
            file.close();
            std::filesystem::remove(testFile);
            return true;
        }

        // File case
        std::filesystem::path parent = path.parent_path();
        if (!parent.empty()) std::filesystem::create_directories(parent);
        std::ofstream file(path.string(), std::ios::app);
        if (!file) return false;
        return true;
    } catch (...) { // Any error same as missing permission
        return false;
    }
}
