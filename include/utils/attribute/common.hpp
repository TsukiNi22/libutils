/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 14/11/2026 by @author Tsukini

File Name:
##  @file common.hpp

File Description:
##  Keywords & values that are not attributes, common to every c++ version
##  (thread_local, restrict, alignas, hardware interference size)
\**************************************************************/

#ifndef ATTRIBUTE_COMMON_H
    #define ATTRIBUTE_COMMON_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    #include <new> // std::hardware_destructive_interference_size, std::hardware_constructive_interference_size, __cpp_lib_hardware_interference_size

    //----------------------------------------------------------------//
    /* DEFINE */

    /* storage */
    // One instance of the variable per thread
    #if __cplusplus >= 201103L // c++11 and above
        #define _thread_local   thread_local
    #elif defined(__GNUC__)
        #define _thread_local   __thread
    #elif defined(_MSC_VER)
        #define _thread_local   __declspec(thread)
    #else
        #define _thread_local // Not available with this compiler: not thread local
    #endif

    /* pointer */
    // Signal a pointer without alias (no other pointer to the same memory)
    #if defined(__GNUC__)
        #define _restrict   __restrict__
    #elif defined(_MSC_VER)
        #define _restrict   __restrict
    #else
        #define _restrict // Not available with this compiler
    #endif

    /* binary layout */
    // Set the memory padding
    #if __cplusplus >= 201103L // c++11 and above
        #define _alignas(n) alignas(n)
    #elif defined(__GNUC__)
        #define _alignas(n) __attribute__((aligned(n)))
    #elif defined(_MSC_VER)
        #define _alignas(n) __declspec(align(n))
    #else
        #define _alignas(n) // Not available with this compiler
    #endif

    //----------------------------------------------------------------//
    /* GLOBAL */

    // fallback (x86-64)
    #if !defined(__cpp_lib_hardware_interference_size)
        #warning "fallback (x86-64) - hardware interference size = 64"
        #include <cstddef>  // std::size_t
        namespace std { // namespace start
        #if __cplusplus >= 201703L // c++17 and above
            inline constexpr std::size_t hardware_destructive_interference_size = 64; // xstyle: ignore CPP-NAMESPACE-INDENT (fallback in the include region)
            inline constexpr std::size_t hardware_constructive_interference_size = 64; // xstyle: ignore CPP-NAMESPACE-INDENT
        #else
            static constexpr std::size_t hardware_destructive_interference_size = 64; // xstyle: ignore CPP-NAMESPACE-INDENT (no inline variable before c++17)
            static constexpr std::size_t hardware_constructive_interference_size = 64; // xstyle: ignore CPP-NAMESPACE-INDENT
        #endif
        } // namespace end
    #endif

#endif /* ATTRIBUTE_COMMON_H */
