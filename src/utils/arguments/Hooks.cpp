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
##  @file Hooks.cpp

File Description:
##  Default hook used for the cli
\**************************************************************/

#define NO_OUTDATED_WARNING
#include "utils/arguments/ArgParser.hpp"
#include <string>

void utils::arguments::defaultHelpHook(const utils::arguments::ArgParser& parser)
{
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
