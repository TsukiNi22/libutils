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
    #include "../attribute/Attribute.hpp"   // _hot
    #include <iostream>                     // std::cout, std::endl
    #include <cstddef>                      // std::size_t
    #include <atomic>                       // std::atomic
    #include <mutex>                        // std::lock_guard, std::recursive_mutex

    //----------------------------------------------------------------//
    /* MACRO */

    /* verbose edition */
    #define set_verbose(v) {utils::verbose::verbose = utils::verbose::Verbose::v;}

    /* multi-threading */
    // recursive: a verbose call can be done inside a *Fn macro
    #define LOCK_OUTPUT std::lock_guard<std::recursive_mutex> lock_(utils::verbose::output_lock)

    /* verbose display */
    #define onBasicVerbose(info)    {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(utils::verbose::Verbose::Basic))    std::cout << info << std::endl;});}
    #define onAdvancedVerbose(info) {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(utils::verbose::Verbose::Advanced)) std::cout << info << std::endl;});}
    #define onDebugVerbose(info)    {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(utils::verbose::Verbose::Debug))    std::cout << "debug: " << info << std::endl;});}
    #define onVerbose(level, info)  {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(level))                             std::cout << info << std::endl;});}

    /* verbose display custom output */
    #define onBasicVerboseC(output, info)    {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(utils::verbose::Verbose::Basic))    output << info << std::endl;});}
    #define onAdvancedVerboseC(output, info) {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(utils::verbose::Verbose::Advanced)) output << info << std::endl;});}
    #define onDebugVerboseC(output, info)    {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(utils::verbose::Verbose::Debug))    output << "debug: " << info << std::endl;});}
    #define onVerboseC(output, level, info)  {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(level))                             output << info << std::endl;});}

    /* verbose execution */
    #define onBasicVerboseFn(fn)    {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(utils::verbose::Verbose::Basic))    {fn}});}
    #define onAdvancedVerboseFn(fn) {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(utils::verbose::Verbose::Advanced)) {fn}});}
    #define onDebugVerboseFn(fn)    {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(utils::verbose::Verbose::Debug))    {fn}});}
    #define onVerboseFn(level, fn)  {utils::verbose::locked([&](void) {if (static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(level))                             {fn}});}

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

//----------------------------------------------------------------//
/* PROTOTYPE */

// Security on stdout writing
extern std::recursive_mutex output_lock;

// Execute the function with the output locked (used by the verbose macros)
template<typename Fn>
_hot inline void locked(Fn&& fn)
{
    std::lock_guard<std::recursive_mutex> lock(utils::verbose::output_lock);
    fn();
}

// Global verbose declaration
extern std::atomic<utils::verbose::Verbose> verbose; // atomic: read by every thread that logs

} // namespace end
#endif /* VERBOSE_H */
