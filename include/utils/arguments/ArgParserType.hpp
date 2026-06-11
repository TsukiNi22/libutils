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
##  @file ArgParsers.hpp

File Description:
##  Declaration of the ArgParser type for void & non void function
\**************************************************************/

#ifndef ARGPARSERSTYPE_H
    #define ARGPARSERSTYPE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <functional>   // std::function
    #include <vector>       // std::vector
    #include <string>       // std::string
    #include <tuple>        // std::tuple

namespace utils::arguments { // namespace
//----------------------------------------------------------------//
/* STRUCT */

// Return of the parser
struct ParsedData {
    std::string usage; // Usage id, empty -> nothing
    std::vector<std::tuple<std::string, bool, std::vector<std::string>>> arguments; // <id, option(true)|flag(false), {option}>
};

struct Usage {
    std::string name; // special name: default -> allow all flag (like no usage defined)
    bool ordered = false; // The order of the option will always matter, Raw option will be forced in mandatory
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
    std::vector<std::tuple<std::string, bool, std::function<bool(const std::string&)>>> options; // <name, mandatory, check>, the order matter
    std::string description = "[None]";
};

} // namespace end
#endif /* ARGPARSERSTYPE_H */
