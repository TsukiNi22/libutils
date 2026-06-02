/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 02/06/2026 by @author Tsukini

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
#include <iostream>
#include <string>

void utils::arguments::defaultHelpHook(const utils::arguments::ArgParser& parser)
{
    const std::unordered_map<std::string, utils::arguments::Usage>&  usages  = parser.getUsages();
    const std::unordered_map<std::string, utils::arguments::Flag>&   flags   = parser.getFlags();
    const std::unordered_map<std::string, utils::arguments::Option>& options = parser.getOptions();

    std::cout << utils::write::format("<strong>PROJECT<>") << std::endl;
    std::cout << utils::write::color(utils::write::Color::Cyan) << parser.getDescription() << std::endl;
    std::cout << utils::write::reset() << std::endl;

    std::cout << utils::write::format("<strong>USAGE<>") << std::endl;
    std::cout << utils::write::color(utils::write::Color::Magenta);
    std::cout << "\t./myteams_cli [ip|ip port]" << std::endl;
    std::cout << "\t./myteams_cli -fh" << std::endl;
    std::cout << "\t./myteams_cli -h" << std::endl;
    for (const auto& [_, usage]: usages) {
        std::cout << "\t./" << parser.getBinary();
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
                for (std::size_t i = 0; i < it->second.checks.size(); ++i)
                    std::cout << " <arg_" << i << ">";
                if (!mandatory) std::cout << "]";
            }

            // Error
            else {
                throw utils::exception::CustomException(utils::exception::Type::Warning, utils::exception::Code::UnknowId, id);
            }
        }
        std::cout << std::endl;
    }
    if (usages.size() == 0) { // Default usage (all authorized)
        std::cout << "\t./" << parser.getBinary();
        for (const auto& [_, flag]: parser.getFlags()) {
            const auto& [fshort, fflag, flong] = flag.flag;
            if (fshort.empty() && fflag.empty() && flong.empty()) continue;
            std::cout << " " << ((fshort.empty() && fflag.empty()) ? "--" : "-") << (fshort.empty() ? (fflag.empty() ? flong : fflag) : fshort);
            for (std::size_t i = 0; i < flag.checks.size(); ++i)
                std::cout << " <arg_" << i << ">";
        }
    }
    std::cout << utils::write::reset() << std::endl;

    std::cout << utils::write::format("<strong>OPTIONS<>") << std::endl;
    for (const auto& [_, option]: options) {
        std::cout << utils::write::color(utils::write::Color::Red) << "\t" << option.name << utils::write::reset() << std::endl;
        std::cout << "\t\t" << option.description << std::endl;
    }
    if (options.size() == 0) std::cout << "\tNothing...";
    std::cout << utils::write::reset() << std::endl;

    std::cout << utils::write::format("<strong>FLAGS<>") << std::endl;
    for (const auto& [_, flag]: flags) {
        const auto& [fshort, fflag, flong] = flag.flag;
        std::cout << utils::write::color(utils::write::Color::Red) << "\t";
        if (!fshort.empty()) std::cout << "-" << fshort;
        if (!fflag.empty())  std::cout << (fshort.empty() ? "" : " ") << "-" << fflag;
        if (!flong.empty())  std::cout << ((fshort.empty() && flong.empty()) ? "" : " ") << "--" << flong;
        std::cout << utils::write::reset() << std::endl;
        std::cout << "\t\t" << flag.description << std::endl;
    }
    if (flags.size() == 0) std::cout << "\tNothing...";
    std::cout << utils::write::reset() << std::flush;
}

bool utils::arguments::defaultBoolParsingHook(const std::string& option)
{
    return true;
}

bool utils::arguments::defaultInt32ParsingHook(const std::string& option)
{
    return true;
}

bool utils::arguments::defaultSizetParsingHook(const std::string& option)
{
    return true;
}

bool utils::arguments::defaultDoubleParsingHook(const std::string& option)
{
    return true;
}

bool utils::arguments::defaultFileParsingHook(const std::string& option)
{
    return true;
}

bool utils::arguments::defaultWritableParsingHook(const std::string& option)
{
    return true;
}
