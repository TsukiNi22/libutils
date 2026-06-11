/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 11/06/2026 by @author Tsukini

File Name:
##  @file ArgParser.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define NO_OUTDATED_WARNING
#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/NoneException.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/custom/CustomException.hpp"
#include "utils/arguments/ArgParser.hpp"
#include "utils/arguments/ArgParserType.hpp"
#include <algorithm>
#include <iostream>
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
        utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::UnknowId, id);
        std::cout << e.formated() << std::endl;
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
        utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::UnknowId, id);
        std::cout << e.formated() << std::endl;
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
        utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::UnknowId, id);
        std::cout << e.formated() << std::endl;
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
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::ArgParserHook, e.what());
    }
}

void utils::arguments::ArgParser::parseFlags(utils::arguments::ParsedData& data, const std::vector<std::string>& argv, std::size_t& i, const bool failsafe) const
{
    std::vector<std::string> ids; // <id>
    std::string arg = argv[i], sarg; // sarg is used for temporary sub edition
    bool isLong = arg.starts_with("--"), isShort = false;
    bool unknow = true;
    std::size_t f = 0; // short counter

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
        std::size_t pos = 0;

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

    // Check if the flag was found
    if (unknow) {
        std::string s = ((!isLong && arg == sarg) ? ("-" + arg + ": " + sarg + " (unknow short)") : ((isLong ? "--" : "-") + arg));
        if (failsafe) {std::cout << utils::exception::CustomException(utils::exception::Type::Warning, utils::exception::Code::UnknowFlag, s).formated() << std::endl; return;}
        else throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::UnknowFlag, s);
    }

    // Check the ids settings and store them if valid
    std::vector<std::string> options;
    for (const std::string& id: ids) {
        const utils::arguments::Flag& flag = this->_flags.at(id);

        // Check if they can be combined
        if (isShort && f > 1 && flag.options.size() != 0) {
            std::string s = "Can't combine short flag that have option(s), '" + std::get<0>(flag.flag) + "' in '-" + arg + "'";
            if (failsafe) {std::cout << utils::exception::CustomException(utils::exception::Type::Warning, utils::exception::Code::FlagCombinaison, s).formated() << std::endl; return;}
            else throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::FlagCombinaison, s);
        }

        // Check for the option(s)
        if (flag.options.size() != 0) {
            for (const auto &[_, mandatory, check]: flag.options) {
                if (argv.size() <= i + 1) {
                    if (!mandatory) continue;
                    std::string s = (isLong ? "--" : "-") + arg;
                    if (failsafe) {std::cout << utils::exception::CustomException(utils::exception::Type::Warning, utils::exception::Code::FlagOptionsNumber, s).formated() << std::endl; return;}
                    else throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::FlagOptionsNumber, s);
                } else if (!check(argv[i + 1])) {
                    if (!mandatory) continue;
                    std::string s = (isLong ? "--" : "-") + arg + ": " + argv[i + 1];
                    if (failsafe) {std::cout << utils::exception::CustomException(utils::exception::Type::Warning, utils::exception::Code::FlagOption, s).formated() << std::endl; return;}
                    else throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::FlagOption, s);
                } else {
                    options.push_back(argv[++i]);
                }
            }
        }

        // Check for redefinition
        for (const auto &[fid, _, _]: data.arguments) {
            if (fid == id) {
                std::cout << utils::exception::CustomException(utils::exception::Type::Warning, utils::exception::Code::DuplicatedFlag, (isLong ? std::get<2>(flag.flag) : (isShort ? std::get<0>(flag.flag) : std::get<1>(flag.flag)))).formated() << std::endl;
                continue;
            }
        }

        // Store it
        data.arguments.emplace_back(id, false, options);
    }
}

static bool removeUsedIds(const utils::arguments::ParsedData& data, std::vector<std::pair<std::string, bool>>& ids, const utils::arguments::Usage& usage, const std::unordered_map<std::string, utils::arguments::Option>& options, const std::unordered_map<std::string, utils::arguments::Flag>& flags)
{
    // For each existing arguments
    for (const auto &[idA, type, arguments]: data.arguments) {
        // Check if it's allowed in the usage
        if (!std::any_of(ids.begin(), ids.end(), [&](const auto& p) {return p.first == idA;}))
            return true; // Dosen't contains the ids
        if (!type && !usage.ordered) continue; // Just checking the flag existance in unordered mode

        // Check for the ordering
        std::vector<std::string> toRemove;
        for (const auto &[id, mandatory]: ids) {
            if (type) { // Option
                const auto it = options.find(id);
                if (it == options.end()) continue;
            } else if (usage.ordered) { // Flag (only with ordered)
                const auto it = flags.find(id);
                if (it == flags.end()) continue;
            }

            // Check if it's the same id has what we want
            if (id != idA) {
                if (mandatory) return true; // Invalid first option
                else toRemove.push_back(id); // Option non mandatory ignored
            } else toRemove.push_back(id);
            break;
        }

        // Remove ignored non mandatory option
        ids.erase(
            std::remove_if(ids.begin(), ids.end(),
            [&](const auto& p) {
                return std::find(toRemove.begin(), toRemove.end(), p.first) != toRemove.end();
            }),
            ids.end()
        );
    }

    // Usage is good
    return false;
}

void utils::arguments::ArgParser::parseOption(utils::arguments::ParsedData& data, const std::vector<std::string>& argv, const std::size_t i, const bool failsafe) const
{
    const std::string& option = argv[i];
    bool fail = false;
    std::vector<std::string> validIds, subValidIds;

    // Try to find the possible corresponding ids of the option
    for (const auto &[id, opt]: this->_options)
        if (opt.check(option)) subValidIds.push_back(id);

    // check for each usages
    for (const auto &[_, usage]: this->_usages) {
        std::vector<std::pair<std::string, bool>> ids = usage.ids;
        fail = removeUsedIds(data, ids, usage, this->_options, this->_flags);
        if (!fail) continue;

        // Keep only the id that are valid for the actual usage
        for (const auto &[id, mandatory]: ids) {
            // Is an option ?
            const auto it = this->_options.find(id);
            if (it == this->_options.end()) continue;

            // Check if it's the same id has what we want
            for (const std::string& sid: subValidIds)
                if (sid == id) validIds.push_back(sid);

            // Only exist when a mandatory option in encouter
            if (mandatory) break;
        }
    }

    // Check if no option where found
    if (validIds.size() == 0) {
        std::string s = option;
        if (failsafe) {std::cout << utils::exception::CustomException(utils::exception::Type::Warning, utils::exception::Code::OptionIngored, s).formated() << std::endl; return;}
        else throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::OptionIngored, s);
    }

    // Store it
    data.arguments.emplace_back(validIds.front(), true, std::vector<std::string>{argv[i]});
}

nodiscard std::string utils::arguments::ArgParser::checkUsageCompliance(const utils::arguments::ParsedData& data, const bool full = false) const
{
    bool fail = false;

    // check for each usages
    for (const auto &[idU, usage]: this->_usages) {
        std::vector<std::pair<std::string, bool>> ids = usage.ids;
        fail = removeUsedIds(data, ids, usage, this->_options, this->_flags);

        // Check for full compliance (missing mandatory)
        if (unlikely_c(full) && !fail) {
            for (const auto &[id, mandatory]: ids)
                if (mandatory) {std::cout << id << std::endl; fail = true; break;};
        }

        if (!fail) return idU;
    }

    // No compliant usage where found
    throw utils::exception::ErrorException(utils::exception::Code::NoCompliantUsage);
}

nodiscard utils::arguments::ParsedData utils::arguments::ArgParser::parse(const int argc, const char *const argv[], const bool failsafe) const
{
    std::vector<std::string> args(argv, argv + argc);
    return this->parse(args, failsafe);
}

nodiscard utils::arguments::ParsedData utils::arguments::ArgParser::parse(const std::vector<std::string>& argv, const bool failsafe) const
{
    utils::arguments::ParsedData data;

    // Minimalist check
    if (argv.size() == 0)
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::ArgumentsNumber, "The arguments should start with the binary name, with a size of 1 at least, got: 0");

    // Check for hardcoded flag: -h, -help, --help
    if (std::any_of(argv.begin(), argv.end(), [&](const std::string& arg) {return (arg == "-h" || arg == "-help" || arg == "--help");})) {
        this->help();
        throw utils::exception::NoneException(utils::exception::Code::Exit);
    }

    // For each arguments
    for (std::size_t i = 1; i < argv.size(); ++i) {
        const std::string& arg = argv[i];

        // Flag dectection
        if (arg.size() > 1 && arg.front() == '-') {
            this->parseFlags(data, argv, i, failsafe);
        }

        // Option
        else {
            this->parseOption(data, argv, i, failsafe);
        }

        // Check if it's correspond to any usage
        data.usage = this->checkUsageCompliance(data);
    }

    // Check if it's correspond exactly to a usage
    data.usage = this->checkUsageCompliance(data, true);
    return data;
}
