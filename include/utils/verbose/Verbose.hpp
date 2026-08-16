/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 16/08/2026 by @author Tsukini

File Name:
##  @file Verbose.hpp

File Description:
##  Marco & Define used for verbose usage
\**************************************************************/

#ifndef VERBOSE_H
    #define VERBOSE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <iostream> // std::cout, std::endl
    #include <cstddef>  // std::size_t
    #include <mutex>    // std::lock_guard, std::mutex

    //----------------------------------------------------------------//
    /* MACRO */

    /* verbose edtion */
    #define set_verbose(v) {utils::verbose::verbose = utils::verbose::Verbose::v;}

    /* multi-threading */
    #define LOCK_OUTPUT std::lock_guard<std::mutex> lock_(utils::verbose::output_lock)

    /* verbose display */
    #define onBasicVerbose(info)    {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(utils::verbose::Verbose::Basic))    std::cout << info << std::endl;}
    #define onAdvancedVerbose(info) {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(utils::verbose::Verbose::Advanced)) std::cout << info << std::endl;}
    #define onDebugVerbose(info)    {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(utils::verbose::Verbose::Debug))    std::cout << "debug: " << info << std::endl;}
    #define onVerbose(level, info)  {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(level))                             std::cout << info << std::endl;}

    /* verbose display custom output */
    #define onBasicVerboseC(output, info)    {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(utils::verbose::Verbose::Basic))    output << info << std::endl;}
    #define onAdvancedVerboseC(output, info) {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(utils::verbose::Verbose::Advanced)) output << info << std::endl;}
    #define onDebugVerboseC(output, info)    {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(utils::verbose::Verbose::Debug))    output << "debug: " << info << std::endl;}
    #define onVerboseC(output, level, info)  {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(level))                             output << info << std::endl;}

    /* verbose execution */
    #define onBasicVerboseFn(fn)    {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(utils::verbose::Verbose::Basic))    {fn}}
    #define onAdvancedVerboseFn(fn) {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(utils::verbose::Verbose::Advanced)) {fn}}
    #define onDebugVerboseFn(fn)    {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(utils::verbose::Verbose::Debug))    {fn}}
    #define onVerboseFn(level, fn)  {LOCK_OUTPUT; if (static_cast<std::size_t>(utils::verbose::verbose) >= static_cast<std::size_t>(level))                             {fn}}

namespace utils::verbose { // namespace start
//----------------------------------------------------------------//
/* ENUM */

// Order of enum definition matter: less (0) -> most (+inf)
enum class Verbose: std::size_t {
    None = 0,
    Basic,
    Advanced,
    Debug,
};

// Security on stdout writing
extern std::mutex output_lock;

// Global verbose declaration
extern volatile utils::verbose::Verbose verbose;

} // namespace end
#endif /* VERBOSE_H */
