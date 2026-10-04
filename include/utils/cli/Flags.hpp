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
##  @file Flags.hpp

File Description:
##  Definition of the flags used to customize the cli
\**************************************************************/

#ifndef FLAGS_H
    #define FLAGS_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _migration
    #include <cstdint>                      // std::uint32_t

namespace utils::cli { // namespace start
//----------------------------------------------------------------//
/* ENUM */

enum Flag {
    DEBUG           = 1 << 0, // Active verbose for internal action (Nothing for now)
    NOECHO          = 1 << 1, // Disable echo of the input
    CATCH           = 1 << 2, // Enable error catching on execution
    EMPTY_INPUT     = 1 << 3, // Ignore empty input (default: error)
    TRIM            = 1 << 4, // Enable trim on input
    PARSED          = 1 << 5, // Active parser for the input and send vector<std::string> (default: std::string)
    PROMPT          = 1 << 6, // Active the prompt
    LOGIC           = 1 << 7, // Enable logic with '&&', '||' and ';'
    ARROW           = 1 << 8, // Activate left, right
    HISTORY         = 1 << 9, // Activate history, up and down arrow
    PERSISTENT      = 1 << 10, // Allow persistent memory between session (history, ...)
    HINT            = 1 << 11, // Display hint when a command fail
    AUTO_COMPLETION = 1 << 12, // Active auto completion with `\t` (only work on the first command for now)
    MANUAL          = 1 << 13, // Enable manual call for each new input handling
    THREAD          = 1 << 14, // Run in a thread
    DETACHED        = 1 << 15, // Detach the thread execution (by default return the thread at start)
    NO_TTY          = 1 << 16, // Allow usage even without tty
};

/*
// ------------ Operator ---------- //
constexpr utils::cli::Flag operator|(utils::cli::Flag lhs, utils::cli::Flag rhs)
{return static_cast<utils::cli::Flag>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));}
constexpr utils::cli::Flag operator&(utils::cli::Flag lhs, utils::cli::Flag rhs)
{return static_cast<utils::cli::Flag>(static_cast<std::uint32_t>(lhs) & static_cast<std::uint32_t>(rhs));}
constexpr utils::cli::Flag operator^(utils::cli::Flag lhs, utils::cli::Flag rhs)
{return static_cast<utils::cli::Flag>(static_cast<std::uint32_t>(lhs) ^ static_cast<std::uint32_t>(rhs));}
constexpr utils::cli::Flag operator~(utils::cli::Flag f)
{return static_cast<utils::cli::Flag>(~static_cast<std::uint32_t>(f));}

inline utils::cli::Flag& operator|=(utils::cli::Flag& lhs, utils::cli::Flag rhs) {return lhs = lhs | rhs;}
inline utils::cli::Flag& operator&=(utils::cli::Flag& lhs, utils::cli::Flag rhs) {return lhs = lhs & rhs;}
inline utils::cli::Flag& operator^=(utils::cli::Flag& lhs, utils::cli::Flag rhs) {return lhs = lhs ^ rhs;}
*/
} // namespace end

namespace utils::cli::flags { // namespace start
//----------------------------------------------------------------//
/* MACRO */

/* flags preset */
constexpr std::uint32_t ALL     = DEBUG | CATCH | NOECHO | TRIM | EMPTY_INPUT | PARSED | PROMPT | LOGIC | ARROW | HISTORY | HINT | AUTO_COMPLETION | MANUAL | THREAD | DETACHED;
constexpr std::uint32_t DEFAULT = CATCH | EMPTY_INPUT | TRIM | PROMPT | ARROW;
constexpr std::uint32_t DUMB    = 0;
constexpr std::uint32_t TERM1   = CATCH | EMPTY_INPUT | TRIM | PARSED | PROMPT | LOGIC | ARROW | HISTORY;
constexpr std::uint32_t TERM2   = TERM1 | HINT | AUTO_COMPLETION;
constexpr std::uint32_t TERM3   = TERM2 | THREAD;
constexpr std::uint32_t LOG     = TERM3 | DETACHED | NO_TTY;
constexpr std::uint32_t DEV     = TERM2 | DEBUG;
constexpr std::uint32_t MULTI_THREADING = THREAD | DETACHED;
/*
 * DEFAULT -> Basic term
 * TERM1   -> Advanced term
 * TERM2   -> Completion on advanced term
 * TERM3   -> Multi threading advanced term
*/

} // namespace end

//----------------------------------------------------------------//
/* MIGRATION */
namespace utils::cli::Flags {
    _migration(4, 0, 0) inline constexpr std::uint32_t ALL = utils::cli::flags::ALL;
    _migration(4, 0, 0) inline constexpr std::uint32_t DEFAULT = utils::cli::flags::DEFAULT;
    _migration(4, 0, 0) inline constexpr std::uint32_t DUMB = utils::cli::flags::DUMB;
    _migration(4, 0, 0) inline constexpr std::uint32_t TERM1 = utils::cli::flags::TERM1;
    _migration(4, 0, 0) inline constexpr std::uint32_t TERM2 = utils::cli::flags::TERM2;
    _migration(4, 0, 0) inline constexpr std::uint32_t TERM3 = utils::cli::flags::TERM3;
    _migration(4, 0, 0) inline constexpr std::uint32_t LOG = utils::cli::flags::LOG;
    _migration(4, 0, 0) inline constexpr std::uint32_t DEV = utils::cli::flags::DEV;
    _migration(4, 0, 0) inline constexpr std::uint32_t MULTI_THREADING = utils::cli::flags::MULTI_THREADING;
}

#endif /* FLAGS_H */
