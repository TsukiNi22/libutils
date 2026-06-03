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
##  @file ArgParsers.hpp

File Description:
##  Declaration of the ArgParser type for void & non void function
\**************************************************************/

#ifndef ARGPARSERSTYPE_H
    #define ARGPARSERSTYPE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <unordered_map>    // std::unordered_map
    #include <functional>       // std::function
    #include <vector>           // std::vector
    #include <string>           // std::string
    #include <tuple>            // std::tuple

namespace utils::arguments { // namespace
//----------------------------------------------------------------//
/* STRUCT */

// Return of the parser
using ParsedData = std::unordered_map<
    std::string, // id
    std::vector<std::string> // options
>;

struct Usage {
    std::string name; // special name: default -> allow all flag (like no usage defined)
    bool ordered = false; // The order of the option is mandatory, Raw option will be forced in mandatory
    std::vector<std::pair<std::string, bool>> ids; // List of ids allowed by this usage <id, mandatory>
    std::string description = "[None]";
};

struct Option {
    std::string name;
    std::function<bool(const std::string&)> check;
    std::string description = "[None]";
};

struct Flag {
    std::tuple<std::string, std::string, std::string> flag; // <short, flag, long>
    std::vector<std::tuple<std::string, bool, std::function<bool(const std::string&)>>> options; // <name, mandatory, check>
    std::string description = "[None]";
};

} // namespace end
#endif /* ARGPARSERSTYPE_H */
