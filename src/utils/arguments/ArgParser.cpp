/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 24/09/2026 by @author Tsukini

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
#include <cstdlib>
#include <vector>
#include <deque>
#include <string>

utils::arguments::ArgParser::ArgParser(const std::string& binary, const std::string& description)
: _binary{binary}, _description{description}
{
    // Setup initial values
    this->resetHelpHook();
}

_cold void utils::arguments::ArgParser::removeUsage(const std::string& id)
{
    if (!this->_usages.contains(id)) {
        utils::exception::WarningException e(utils::exception::InternalCode::UnknownId, id);
        std::cerr << e.formated() << std::endl;
        return;
    }
    this->_usages.erase(id);
}

_cold void utils::arguments::ArgParser::removeUsages(const std::vector<std::string>& ids)
{
    for (const std::string& id: ids)
        this->removeUsage(id);
}

_cold void utils::arguments::ArgParser::removeOption(const std::string& id)
{
    if (!this->_options.contains(id)) {
        utils::exception::WarningException e(utils::exception::InternalCode::UnknownId, id);
        std::cerr << e.formated() << std::endl;
        return;
    }
    this->_options.erase(id);
}

_cold void utils::arguments::ArgParser::removeOptions(const std::vector<std::string>& ids)
{
    for (const std::string& id: ids)
        this->removeOption(id);
}

_cold void utils::arguments::ArgParser::removeFlag(const std::string& id)
{
    if (!this->_flags.contains(id)) {
        utils::exception::WarningException e(utils::exception::InternalCode::UnknownId, id);
        std::cerr << e.formated() << std::endl;
        return;
    }
    this->_flags.erase(id);
}

_cold void utils::arguments::ArgParser::removeFlags(const std::vector<std::string>& ids)
{
    for (const std::string& id: ids)
        this->removeFlag(id);
}

_cold void utils::arguments::ArgParser::help(void) const
{
    try {
        this->_helpHook(*this);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::ArgParserHook, e.what());
    }
}

_hot _nodiscard bool utils::arguments::ArgParser::parseFlags_(utils::arguments::ParsedUsageFull& usageFull, const std::vector<std::string>& argv, std::size_t& i, bool& alreadyFailed, const bool failsafe) const
{
    std::vector<std::string> ids, idsChecked; // <id>
    std::string arg = argv[i], sarg; // sarg is used for temporary sub edition
    const std::string argOrigin = arg; // keep the original value
    bool isLong = arg.starts_with("--"), isShort = false;
    bool unknown = true;
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

        // Is the flag known
        for (const auto &[fid, flag]: this->_flags) {
            const auto &[_, _, flong, _] = flag.flag;
            if (flong == arg) {ids.push_back(fid); unknown = false; break;}
        }
    }

    // Short & Flag
    else {
        std::size_t size = 0;
        arg.erase(0, 1); // Remove '-'
        sarg = arg; // Used for the short checking

        // Is the flag known (Flag have the priority)
        for (const auto &[fid, flag]: this->_flags) {
            const auto &[fshort, fflag, _, _] = flag.flag;
            if (fflag == arg) {ids.clear(); ids.push_back(fid); isShort = false; unknown = false; break;}
            else if (!fshort.empty() && size < arg.size() && (pos = sarg.find(fshort)) != std::string::npos) {
                ids.push_back(fid);
                sarg.erase(pos, fshort.size());
                size += fshort.size();
                ++f;
                if (size == arg.size()) {isShort = true; unknown = false;}
            }
        }
    }

    // Check if the flag was found
    if (unknown) {
        if (alreadyFailed) return false;
        alreadyFailed = true;
        std::string s = ((!isLong && arg == sarg) ? ("-" + arg + ": " + sarg + " (unknown short)") : ((isLong ? "--" : "-") + arg));
        if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::UnknownFlag, s).formated() << std::endl; return false;}
        else throw utils::exception::ErrorException(utils::exception::InternalCode::UnknownFlag, s);
        return false;
    }

    // Empty flag detection (should have exited at unknown)
    if (arg.empty() || ids.empty())
        return true; // ignored

    // Select the id who is the next one in the usage
    for (const std::string& id: ids) {
        // Check the id settings and store them if valid
        std::vector<std::string> options;
        const utils::arguments::Flag& flag = this->_flags.at(id);

        // Check if they are allowed by the usage ('default' allow all flags)
        // Not an error: only this usage is invalid, the other usages can still accept the flag
        const utils::arguments::Usage& usage = this->_usages.at(usageFull.id);
        if (usage.name != "default" && !std::any_of(usage.ids.begin(), usage.ids.end(), [&](const std::pair<std::string, bool>& p) {return p.first == id;}))
            return false;

        // Check for redefinition (its option(s) are still consumed below, but it isn't stored)
        bool redefined = false;
        for (const auto &[fid, type, _]: usageFull.arguments) {
            if (!type && fid == id) {
                if (!alreadyFailed) std::cerr << utils::exception::WarningException(utils::exception::InternalCode::DuplicatedFlag, (isLong ? std::get<2>(flag.flag) : (isShort ? std::get<0>(flag.flag) : std::get<1>(flag.flag)))).formated() << std::endl;
                alreadyFailed = true;
                redefined = true;
                break;
            }
        }

        // Check in ordered case if the flag is the next one (out of order: this usage is invalid)
        if (usageFull.ordered && !redefined) {
            std::size_t index = 0;
            for (index = 0; index < usageFull.ids.size() && usageFull.ids.at(index).first != id && !usageFull.ids.at(index).second; ++index);
            // Fail to setup a mandatory argument before the flag (or no more place for it)
            if (index >= usageFull.ids.size() || usageFull.ids.at(index).first != id) return false;
        }

        // Check if they can be combined
        if (isShort && f > 1 && flag.options.size() != 0) {
            if (alreadyFailed) return false;
            alreadyFailed = true;
            std::string s = "Can't combine short flag that have option(s), '" + std::get<0>(flag.flag) + "' in '-" + arg + "'";
            if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::FlagCombinaison, s).formated() << std::endl; return false;}
            else throw utils::exception::ErrorException(utils::exception::InternalCode::FlagCombinaison, s);
            return false;
        }

        // Check if the '=' is allowed
        bool single = (flag.options.size() == 1);
        if (equalFound && !single) {
            if (alreadyFailed) return false;
            alreadyFailed = true;
            std::string s = "Can't use '=' on flag that have only one option(s): '" + argOrigin + "'";
            if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::FlagOption, s).formated() << std::endl; return false;}
            else throw utils::exception::ErrorException(utils::exception::InternalCode::FlagOption, s);
            return false;
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
                return false;
            } else if (!equalFound && j + 1 >= flag.options.size() && flag.unlimited.first && !flag.unlimited.second && argv[i + 1].starts_with('-')) { // Special case (unlimited can also accept no argument)
                breaked = true;
                break;
            } else if ((equalFound && (res = check(equal)).has_value()) || (!equalFound && (res = check(argv[i + 1])).has_value())) {
                if (!mandatory) continue;
                if (alreadyFailed) return false;
                alreadyFailed = true;
                std::string s = (isLong ? "--" : "-") + arg + ": " + *res;
                if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::FlagOption, s).formated() << std::endl; return false;}
                else throw utils::exception::ErrorException(utils::exception::InternalCode::FlagOption, s);
                return false;
            } else {
                options.push_back(equalFound ? equal : argv[++i]);
            }
        }

        // For unlimited options (extend to infinite the last option)
        if (flag.unlimited.first && !flag.options.empty() && !breaked) {
            const auto &[_, _, check] = flag.options.back();
            while (true) {
                if (argv.size() <= i + 1) break; // End of arguments
                else if (!flag.unlimited.second && argv[i + 1].starts_with('-')) break; // Other flag
                else if (check(argv[i + 1]).has_value()) break; // Non compliance
                else options.push_back(argv[++i]);
            }
        }

        // Exit and dosen't store the redefined one (its option(s) were consumed)
        if (redefined) continue;

        // Store it
        usageFull.arguments.emplace_back(id, false, options);
        auto it = std::find_if(usageFull.ids.begin(), usageFull.ids.end(), [&](const std::pair<std::string, bool>& p) {return p.first == id;});
        if (it != usageFull.ids.end()) {
            if (usageFull.ordered) usageFull.ids.erase(usageFull.ids.begin(), it + 1); // the ids before are skipped (not mandatory)
            else usageFull.ids.erase(it);
        }
    }

    return true;
}

_hot _nodiscard bool utils::arguments::ArgParser::parseOption_(utils::arguments::ParsedUsageFull& usageFull, const std::vector<std::string>& argv, const std::size_t i, bool& alreadyFailed, const bool failsafe) const
{
    const std::string& option = argv[i];
    std::optional<std::string> res;
    std::vector<std::string> validIds;
    std::string id;

    // Try to find the possible corresponding ids of the option
    //const bool alreadyFailedLocal = alreadyFailed;
    for (const auto &[oid, opt]: this->_options) {
        if (opt.exact && option == opt.name) validIds.push_back(oid);
        else if (!opt.exact) {
            if ((res = opt.check(option)).has_value()) {
                /*if (alreadyFailedLocal) continue;
                alreadyFailed = true;
                std::string s = option + ": " + *res;
                if (!std::any_of(usageFull.ids.begin(), usageFull.ids.end(), [&](const auto& p) {return oid == p.first;})) continue;
                if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::OptionIngored, s).formated() << std::endl; return false;}
                else throw utils::exception::ErrorException(utils::exception::InternalCode::OptionIngored, s);
                */
                continue;
            } else validIds.push_back(oid);
        } else continue;
    }

    // Check if the option is valid in any way
    if (validIds.size() == 0) {
        if (alreadyFailed) return false;
        alreadyFailed = true;
        std::string s = option;
        if (failsafe) {std::cerr << utils::exception::WarningException(utils::exception::InternalCode::OptionIngored, s).formated() << std::endl; return false;}
        else throw utils::exception::ErrorException(utils::exception::InternalCode::OptionIngored, s);
        return false;
    }

    // Check if the actual usage allow the id (first valid correspondence win)
    for (const auto &[subId, mandatory]: usageFull.ids) {
        // Is the id in the valid id found
        if (std::any_of(validIds.begin(), validIds.end(), [&](const std::string& vid) {return vid == subId;})) {
            id = subId;
            break;
        }

        // Fail to setup a mandatory argument before the option (only the ordered usages care about the order)
        if (usageFull.ordered && mandatory) return false;
    }

    // Check if no option where found
    if (id.empty()) return false;

    // Store it
    usageFull.arguments.emplace_back(id, true, std::vector<std::string>{argv[i]});
    auto it = std::find_if(usageFull.ids.begin(), usageFull.ids.end(), [&](const std::pair<std::string, bool>& p) {return p.first == id;});
    if (usageFull.ordered) usageFull.ids.erase(usageFull.ids.begin(), it + 1); // the ids before are skipped (not mandatory)
    else usageFull.ids.erase(it);

    return true;
}

_hot _nodiscard static std::optional<std::string> get_env(const std::string& name)
{
    const char* value = std::getenv(name.c_str());
    if (value == nullptr) return std::nullopt;
    return std::string(value);
}

_hot void utils::arguments::ArgParser::parseEnvironement_(utils::arguments::ParsedUsageFull& usageFull) const noexcept
{
    // iterate on a copy: the ids found are removed from the usage
    const std::deque<std::pair<std::string, bool>> ids = usageFull.ids;
    for (const auto &[id, mandatory]: ids) {
        // try to find a corresponding flag
        if (!this->_flags.contains(id)) continue;

        // extract flag content and check it's requirement (only 1 arg is allowed)
        const utils::arguments::Flag& flag = this->_flags.at(id);
        if (flag.options.size() != 1) continue;
        auto [_, _, _, fenv] = flag.flag;
        auto [_, _, check] = flag.options.front();

        // check if it's in the env
        if (fenv.empty()) continue;
        std::optional<std::string> res = get_env(fenv);
        if (!res.has_value()) continue;

        // check the value (a throwing hook is an invalid value: this function is noexcept)
        try {
            if (check(*res).has_value()) continue;
        } catch (...) {continue;}

        // store the value
        usageFull.arguments.emplace_back(id, false, std::vector<std::string>{*res});
        auto it = std::find_if(usageFull.ids.begin(), usageFull.ids.end(), [&](const std::pair<std::string, bool>& p) {return p.first == id;});
        if (it != usageFull.ids.end()) usageFull.ids.erase(it);
    }
}

_hot _nodiscard utils::arguments::ParsedUsages utils::arguments::ArgParser::parse(const int argc, const char *const argv[], const bool failsafe) const
{
    std::vector<std::string> args(argv, argv + argc);
    return this->parse(args, failsafe);
}

_hot _nodiscard utils::arguments::ParsedUsages utils::arguments::ArgParser::parse(const std::vector<std::string>& argv, const bool failsafe) const
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
        std::deque<std::pair<std::string, bool>> ids(usage.ids.begin(), usage.ids.end());
        if (usage.name == "default") { // allow every flag & option (not mandatory)
            for (const auto &[fid, _]: this->_flags) ids.emplace_back(fid, false);
            for (const auto &[oid, _]: this->_options) ids.emplace_back(oid, false);
        }
        usagesFull.emplace_back(id, true, usage.ordered, std::move(ids),
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
                usageFull.valid &= this->parseFlags_(usageFull, argv, subIndex, failed, failsafe);
            }

            // Option
            else {
                usageFull.valid &= this->parseOption_(usageFull, argv, subIndex, failed, failsafe);
            }

            i = std::max(i, subIndex);
        }

        // Remove invalid usage
        usagesFull.erase(
            std::remove_if(usagesFull.begin(), usagesFull.end(), [&](const utils::arguments::ParsedUsageFull& usageFull) {return !usageFull.valid;}),
            usagesFull.end()
        );
    }

    // try to find the environement var still not found and asked (failsafe, no warning or error)
    for (utils::arguments::ParsedUsageFull& usageFull: usagesFull)
        this->parseEnvironement_(usageFull);

    // Remove thoses who aren't fully done (still mandatory thing to parse)
    usagesFull.erase(
        std::remove_if(usagesFull.begin(), usagesFull.end(),
            [&](const utils::arguments::ParsedUsageFull& usageFull) {
                return std::any_of(usageFull.ids.begin(), usageFull.ids.end(), [&](const std::pair<std::string, bool>& p) {return p.second;});
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

    // Sort the usage from the one who match the most option to the least
    std::stable_sort(usages.begin(), usages.end(), [](const utils::arguments::ParsedUsage& lhs, const utils::arguments::ParsedUsage& rhs) {return lhs.arguments.size() > rhs.arguments.size();});

    return usages;
}
