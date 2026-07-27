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
##  @file ArgParser.cpp

File Description:
##  ArgParser methods definition
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/NoneException.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/basic/WarningException.hpp"
#include "utils/arguments/ArgParser.hpp"
#include "utils/arguments/ArgParserType.hpp"
#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>
#include <string>

utils::arguments::ArgParser::ArgParser(const std::string& binary, const std::string& description)
: _binary{binary}, _description{description}
{
    // Setup initial values
    this->resetHelpHook();
}

void utils::arguments::ArgParser::removeUsage(const std::string& id)
{
    if (!this->_usages.contains(id)) {
        utils::exception::WarningException e(utils::exception::InternalCode::UnknowId, id);
        std::cerr << e.formated() << std::endl;
        return;
    }
    this->_usages.erase(id);
}

void utils::arguments::ArgParser::removeUsages(const std::vector<std::string>& ids)
{
    for (const std::string& id: ids)
        this->removeUsage(id);
}

void utils::arguments::ArgParser::removeOption(const std::string& id)
{
    if (!this->_options.contains(id)) {
        utils::exception::WarningException e(utils::exception::InternalCode::UnknowId, id);
        std::cerr << e.formated() << std::endl;
        return;
    }
    this->_options.erase(id);
}

void utils::arguments::ArgParser::removeOptions(const std::vector<std::string>& ids)
{
    for (const std::string& id: ids)
        this->removeOption(id);
}

void utils::arguments::ArgParser::removeFlag(const std::string& id)
{
    if (!this->_flags.contains(id)) {
        utils::exception::WarningException e(utils::exception::InternalCode::UnknowId, id);
        std::cerr << e.formated() << std::endl;
        return;
    }
    this->_flags.erase(id);
}

void utils::arguments::ArgParser::removeFlags(const std::vector<std::string>& ids)
{
    for (const std::string& id: ids)
        this->removeFlag(id);
}

void utils::arguments::ArgParser::help(void) const
{
    try {
        this->_helpHook(*this);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::ArgParserHook, e.what());
    }
}

bool utils::arguments::ArgParser::parseFlags(utils::arguments::ParsedUsageFull& usageFull, const std::vector<std::string>& argv, std::size_t& i, bool& alreadyFailed, const bool failsafe) const
{
    std::vector<std::string> ids; // <id>
    std::string arg = argv[i], sarg; // sarg is used for temporary sub edition
    const std::string argOrigin = arg; // keep the orignal value
    std::string id;
    bool isLong = arg.starts_with("--"), isShort = false;
    bool unknow = true;
    std::size_t f = 0; // short counter
    std::size_t pos = 0;

    // Check for '='
    bool equalFound = false;
    std::string equal;
    if ((pos = arg.find('=')) != std::string::npos) {
        equalFound = true;
        equal = arg.substr(pos + 1);
        arg = arg.substr(0, pos);
    }

    // Long
    if (isLong) {
        arg.erase(0, 2); // Remove '--'

        // Is the flag know
        for (const auto &[fid, flag]: this->_flags) {
            const auto &[_, _, flong] = flag.flag;
            if (flong == arg) {ids.push_back(fid); unknow = false; break;}
        }
    }

    // Short & Flag
    else {
        arg.erase(0, 1); // Remove '-'
        sarg = arg; // Used for the short checking

        // Is the flag know (Flag have the priority)
        for (const auto &[fid, flag]: this->_flags) {
            const auto &[fshort, fflag, _] = flag.flag;
            if (fflag == arg) {ids.push_back(fid); unknow = false; break;}
            else if ((pos = sarg.find(fshort)) != std::string::npos) {
                ids.push_back(fid);
                sarg.erase(pos, fshort.size());
                if (++f == arg.size()) {isShort = true; unknow = false; break;};
            }
        }
    }

    // Empty flag detection
    if (arg.empty() || ids.empty())
        return true; // ignored

    // Check if the flag was found
    if (unknow) {
        if (alreadyFailed) return false;
        alreadyFailed = true;
        std::string s = ((!isLong && arg == sarg) ? ("-" + arg + ": " + sarg + " (unknow short)") : ((isLong ? "--" : "-") + arg));
        if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::UnknowFlag, s).formated() << std::endl; return false;}
        else throw utils::exception::ErrorException(utils::exception::InternalCode::UnknowFlag, s);
    }

    // Select the id who is the next one in the usage
    for (const std::string& subId: ids) {
        // Check if they are allowed in the left over ids of the usage
        if (!std::any_of(this->_usages.at(usageFull.id).ids.begin(), this->_usages.at(usageFull.id).ids.end(), [&](const auto& p) {return p.first == subId;}))
            continue;

        // Check in ordered case if the flag is the next one
        if (usageFull.ordered) {
            std::size_t index = 0;
            for (index = 0; usageFull.ids.at(index).first != subId && !usageFull.ids.at(index).second; ++index);
            // Fail to setup a mandatory argument before the flag
            if (usageFull.ids.at(index).first != subId) continue;
        }

        // Save the first valid optional or mandatory that fit
        id = subId;
        break;
    }
    if (id.empty()) return false;

    // Check the id settings and store them if valid
    std::vector<std::string> options;
    const utils::arguments::Flag& flag = this->_flags.at(id);

    // Check if the '=' is allowed
    bool single = (flag.options.size() == 1);
    if (equalFound && !single) {
        if (alreadyFailed) return false;
        alreadyFailed = true;
        std::string s = "Can't use '=' on flag that have only one option(s): '" + argOrigin + "'";
        if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::FlagOption, s).formated() << std::endl; return false;}
        else throw utils::exception::ErrorException(utils::exception::InternalCode::FlagOption, s);
    }

    // Check for redefinition
    bool redefined = false;
    for (const auto &[fid, type, _]: usageFull.arguments) {
        if (!type && fid == id) {
            if (!alreadyFailed) std::cerr << utils::exception::WarningException(utils::exception::InternalCode::DuplicatedFlag, (isLong ? std::get<2>(flag.flag) : (isShort ? std::get<0>(flag.flag) : std::get<1>(flag.flag)))).formated() << std::endl;
            alreadyFailed = true;
            redefined = true;
            break;
        }
    }

    // Check if they can be combined
    if (isShort && f > 1 && flag.options.size() != 0) {
        if (alreadyFailed) return false;
        alreadyFailed = true;
        std::string s = "Can't combine short flag that have option(s), '" + std::get<0>(flag.flag) + "' in '-" + arg + "'";
        if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::FlagCombinaison, s).formated() << std::endl; return false;}
        else throw utils::exception::ErrorException(utils::exception::InternalCode::FlagCombinaison, s);
    }

    // Check for the option(s)
    std::optional<std::string> res;
    bool breaked = false;
    for (std::size_t j = 0; j < flag.options.size(); ++j) {
        const auto &[_, mandatory, check] = flag.options[j];
        if (!equalFound && argv.size() <= i + 1) {
            if (!mandatory) continue;
            if (alreadyFailed) return false;
            alreadyFailed = true;
            std::string s = (isLong ? "--" : "-") + arg;
            if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::FlagOptionsNumber, s).formated() << std::endl; return false;}
            else throw utils::exception::ErrorException(utils::exception::InternalCode::FlagOptionsNumber, s);
        } else if (!equalFound && j + 1 >= flag.options.size() && flag.unlimited && argv[i + 1].front() == '-') { // Special case (unlimited can also accept no argument)
            breaked = true;
            break;
        } else if ((equalFound && (res = check(equal)).has_value()) || (!equalFound && (res = check(argv[i + 1])).has_value())) {
            if (!mandatory) continue;
            if (alreadyFailed) return false;
            alreadyFailed = true;
            std::string s = (isLong ? "--" : "-") + arg + ": " + *res;
            if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::FlagOption, s).formated() << std::endl; return false;}
            else throw utils::exception::ErrorException(utils::exception::InternalCode::FlagOption, s);
        } else {
            options.push_back(equalFound ? equal : argv[++i]);
        }
    }

    // For unlimited options (extend to infinite the last option)
    if (flag.unlimited && !breaked) {
        const auto &[_, _, check] = flag.options.back();
        while (true) {
            if (argv.size() <= i + 1) break; // End of arguments
            else if (argv[i + 1].front() == '-') break; // Other flag
            else if (check(argv[i + 1]).has_value()) break; // Non compliance
            else options.push_back(argv[++i]);
        }
    }

    // Exit and dosen't store the redefined one
    if (redefined) return true;

    // Store it
    usageFull.arguments.emplace_back(id, false, options);
    if (usageFull.ordered) {
        while (usageFull.ids.front().first != id) usageFull.ids.pop_front();
        usageFull.ids.pop_front();
    } else {
        auto it = std::find_if(usageFull.ids.begin(), usageFull.ids.end(), [&](const auto& p) {return p.first == id;});
        usageFull.ids.erase(it);
    }

    return true;
}

bool utils::arguments::ArgParser::parseOption(utils::arguments::ParsedUsageFull& usageFull, const std::vector<std::string>& argv, const std::size_t i, bool& alreadyFailed, const bool failsafe) const
{
    const std::string& option = argv[i];
    std::vector<std::string> validIds;
    std::string id;

    // Try to find the possible corresponding ids of the option
    for (const auto &[oid, opt]: this->_options)
        if (!opt.check(option).has_value()) validIds.push_back(oid);

    // Check if the option is valid in any way
    if (validIds.size() == 0) {
        if (alreadyFailed) return false;
        alreadyFailed = true;
        std::string s = option;
        if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::OptionIngored, s).formated() << std::endl; return false;}
        else throw utils::exception::ErrorException(utils::exception::InternalCode::OptionIngored, s);
    }

    // Check if the actual usage allow the id (first valid correspondence win)
    for (const auto &[subId, mandatory]: usageFull.ids) {
        // Is the id in the valid id found
        if (std::any_of(validIds.begin(), validIds.end(), [&](const auto& vid) {return vid == subId;})) {
            id = subId;
            break;
        }

        // Fail to setup a mandatory argument before the option
        if (mandatory) return false;
    }

    // Check if no option where found
    if (id.empty()) return false;

    // Store it
    usageFull.arguments.emplace_back(id, true, std::vector<std::string>{argv[i]});
    while (usageFull.ids.front().first != id) usageFull.ids.pop_front();
    usageFull.ids.pop_front();

    return true;
}

_nodiscard utils::arguments::ParsedUsages utils::arguments::ArgParser::parse(const int argc, const char *const argv[], const bool failsafe) const
{
    std::vector<std::string> args(argv, argv + argc);
    return this->parse(args, failsafe);
}

_nodiscard utils::arguments::ParsedUsages utils::arguments::ArgParser::parse(const std::vector<std::string>& argv, const bool failsafe) const
{
    std::vector<utils::arguments::ParsedUsageFull> usagesFull;
    utils::arguments::ParsedUsages usages;

    // Minimalist check
    if (argv.size() == 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::ArgumentsNumber, "The arguments should start with the binary name, with a size of 1 at least, got: 0");

    // Check for hardcoded flag: -h, -help, --help
    if (this->_help && std::any_of(argv.begin(), argv.end(), [&](const std::string& arg) {return (arg == "-h" || arg == "-help" || arg == "--help");})) {
        this->help();
        throw utils::exception::NoneException(utils::exception::InternalCode::Exit);
    }

    // Build the full usages
    for (const auto &[id, usage]: this->_usages) {
        usagesFull.emplace_back(id, true, usage.ordered,
            std::deque<std::pair<std::string, bool>>(usage.ids.begin(), usage.ids.end()),
            std::vector<std::tuple<std::string, bool, std::vector<std::string>>>{}
        );
    }

    // For each arguments
    for (std::size_t i = 1; i < argv.size(); ++i) {
        const std::string& arg = argv[i];
        bool failed = false;

        // For each usage
        std::size_t subIndex, saveIndex = i;
        for (utils::arguments::ParsedUsageFull& usageFull: usagesFull) {
            subIndex = saveIndex;

            // Flag dectection
            if (arg.size() > 0 && arg.front() == '-') {
                usageFull.valid &= this->parseFlags(usageFull, argv, subIndex, failed, failsafe);
            }

            // Option
            else {
                usageFull.valid &= this->parseOption(usageFull, argv, subIndex, failed, failsafe);
            }

            i = std::max(i, subIndex);
        }

        // Remove invalid usage
        usagesFull.erase(
            std::remove_if(usagesFull.begin(), usagesFull.end(), [&](const auto& usageFull) {return !usageFull.valid;}),
            usagesFull.end()
        );
    }

    // Remove thoses who aren't fully done (still mandatory thing to parse)
    usagesFull.erase(
        std::remove_if(usagesFull.begin(), usagesFull.end(),
            [&](const auto& usageFull) {
                return std::any_of(usageFull.ids.begin(), usageFull.ids.end(), [&](const auto& p) {return p.second;});
            }
        ),
        usagesFull.end()
    );

    // Convert the fusages -> usages
    for (const utils::arguments::ParsedUsageFull& usageFull: usagesFull)
        usages.emplace_back(usageFull.id, usageFull.arguments);

    // No compliant usage where found
    if (usages.size() == 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::NoCompliantUsage);

    return usages;
}
