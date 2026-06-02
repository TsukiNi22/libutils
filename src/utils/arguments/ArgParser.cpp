/**************************************************************\
Edition:
##  @date 02/06/2026 by @author Tsukini

File Name:
##  @file ArgParser.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define NO_OUTDATED_WARNING
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/custom/CustomException.hpp"
#include "utils/arguments/ArgParser.hpp"
#include "utils/arguments/ArgParserType.hpp"
#include <iostream>
#include <vector>
#include <string>

utils::arguments::ArgParser::ArgParser(const std::string& binary, const std::string& description)
: _binary{binary}, _description{description}
{
    // Setup initial values
    this->resetHelpHook();
}

void utils::arguments::ArgParser::delUsage(const std::string& id)
{
    if (this->_usages.contains(id)) {
        utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::UnknowId, id);
        std::cout << e.formated() << std::endl;
        return;
    }
    this->_usages.erase(id);
}

void utils::arguments::ArgParser::delUsages(const std::vector<std::string>& ids)
{
    for (const std::string& id: ids)
        this->delUsage(id);
}

void utils::arguments::ArgParser::delOption(const std::string& id)
{
    if (this->_options.contains(id)) {
        utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::UnknowId, id);
        std::cout << e.formated() << std::endl;
        return;
    }
    this->_options.erase(id);
}

void utils::arguments::ArgParser::delOptions(const std::vector<std::string>& ids)
{
    for (const std::string& id: ids)
        this->delOption(id);
}

void utils::arguments::ArgParser::delFlag(const std::string& id)
{
    if (this->_flags.contains(id)) {
        utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::UnknowId, id);
        std::cout << e.formated() << std::endl;
        return;
    }
    this->_flags.erase(id);
}

void utils::arguments::ArgParser::delFlags(const std::vector<std::string>& ids)
{
    for (const std::string& id: ids)
        this->delFlag(id);
}

void utils::arguments::ArgParser::help(void)
{
    try {
        this->_helpHook(*this);
    } catch (const std::exception& e) {
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::ArgParserHook, e.what());
    }
}

utils::arguments::ParsedData utils::arguments::ArgParser::parse(const int argc, const char *const argv[], bool failsafe)
{
    std::vector<std::string> args(argv, argv + argc);
    return this->parse(args, failsafe);
}

utils::arguments::ParsedData utils::arguments::ArgParser::parse(std::vector<std::string>& argv, bool failsafe)
{
    return {};
}
