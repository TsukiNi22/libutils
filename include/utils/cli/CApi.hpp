/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file CApi.hpp

File Description:
##  C++ side of the C API of utils::cli::Cli (handle & checks)
\**************************************************************/

// xstyle: ignore-file CPP-CLASS-NAME (C handle: C types are snake_case)

#ifndef CLI_CAPI_H
    #define CLI_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../exception/Common.hpp"  // UTILS_OK, UTILS_KO
    #include "../exception/CApi.hpp"    // utils_handle
    #include "Flags.hpp"                // utils::cli::Flag, utils::cli::flags::* (preset)
    #include "Cli.hpp"                  // utils::cli::Cli
    #include "CApi.h"                   // utils_cli_t, C_API_CLI_*
    #include <optional>                 // std::optional
    #include <cstdint>                  // std::uint32_t
    #include <thread>                   // std::thread

//----------------------------------------------------------------//
/* CHECK */

/* C & C++ flags must stay the same */
static_assert(C_API_CLI_FLAG_DEBUG           == static_cast<std::uint32_t>(utils::cli::Flag::DEBUG));
static_assert(C_API_CLI_FLAG_NOECHO          == static_cast<std::uint32_t>(utils::cli::Flag::NOECHO));
static_assert(C_API_CLI_FLAG_CATCH           == static_cast<std::uint32_t>(utils::cli::Flag::CATCH));
static_assert(C_API_CLI_FLAG_EMPTY_INPUT     == static_cast<std::uint32_t>(utils::cli::Flag::EMPTY_INPUT));
static_assert(C_API_CLI_FLAG_TRIM            == static_cast<std::uint32_t>(utils::cli::Flag::TRIM));
static_assert(C_API_CLI_FLAG_PARSED          == static_cast<std::uint32_t>(utils::cli::Flag::PARSED));
static_assert(C_API_CLI_FLAG_PROMPT          == static_cast<std::uint32_t>(utils::cli::Flag::PROMPT));
static_assert(C_API_CLI_FLAG_LOGIC           == static_cast<std::uint32_t>(utils::cli::Flag::LOGIC));
static_assert(C_API_CLI_FLAG_ARROW           == static_cast<std::uint32_t>(utils::cli::Flag::ARROW));
static_assert(C_API_CLI_FLAG_HISTORY         == static_cast<std::uint32_t>(utils::cli::Flag::HISTORY));
static_assert(C_API_CLI_FLAG_PERSISTENT      == static_cast<std::uint32_t>(utils::cli::Flag::PERSISTENT));
static_assert(C_API_CLI_FLAG_HINT            == static_cast<std::uint32_t>(utils::cli::Flag::HINT));
static_assert(C_API_CLI_FLAG_AUTO_COMPLETION == static_cast<std::uint32_t>(utils::cli::Flag::AUTO_COMPLETION));
static_assert(C_API_CLI_FLAG_MANUAL          == static_cast<std::uint32_t>(utils::cli::Flag::MANUAL));
static_assert(C_API_CLI_FLAG_THREAD          == static_cast<std::uint32_t>(utils::cli::Flag::THREAD));
static_assert(C_API_CLI_FLAG_DETACHED        == static_cast<std::uint32_t>(utils::cli::Flag::DETACHED));
static_assert(C_API_CLI_FLAG_NO_TTY          == static_cast<std::uint32_t>(utils::cli::Flag::NO_TTY));

/* C & C++ presets must stay the same */
static_assert(C_API_CLI_FLAGS_ALL             == utils::cli::flags::ALL);
static_assert(C_API_CLI_FLAGS_DEFAULT         == utils::cli::flags::DEFAULT);
static_assert(C_API_CLI_FLAGS_DUMB            == utils::cli::flags::DUMB);
static_assert(C_API_CLI_FLAGS_TERM1           == utils::cli::flags::TERM1);
static_assert(C_API_CLI_FLAGS_TERM2           == utils::cli::flags::TERM2);
static_assert(C_API_CLI_FLAGS_TERM3           == utils::cli::flags::TERM3);
static_assert(C_API_CLI_FLAGS_LOG             == utils::cli::flags::LOG);
static_assert(C_API_CLI_FLAGS_DEV             == utils::cli::flags::DEV);
static_assert(C_API_CLI_FLAGS_MULTI_THREADING == utils::cli::flags::MULTI_THREADING);

/* callbacks return can't collide with the status */
static_assert(C_API_CLI_EXIT != UTILS_OK && C_API_CLI_EXIT != UTILS_KO);
static_assert(C_API_CLI_NO_INPUT != UTILS_OK && C_API_CLI_NO_INPUT != UTILS_KO && C_API_CLI_NO_INPUT != C_API_CLI_EXIT);

//----------------------------------------------------------------//
/* STRUCT */

/* handle of the C API (utils_cli_t), usable in c++ to get the cli */
struct utils_cli: public utils_handle {
    utils::cli::Cli cli;
    std::optional<std::thread> thread; // returned by start with THREAD (joined by utils_cli_join / utils_cli_destroy)

    // ---------- Constructor --------- //
    utils_cli(const bool sig): cli{sig} { /* Nothing */ }; // Enable/Disable catch of ctrl-c & ctrl-z signal
};

#endif /* CLI_CAPI_H */
