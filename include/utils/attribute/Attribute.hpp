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
##  @file Attribute.hpp

File Description:
##  Include the right file from the c++ version used or selected
\**************************************************************/

#ifndef ATTRIBUTE_H
    #define ATTRIBUTE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    #if !defined(__cplusplus)
        #error "C++ compiler required"
    #elif !defined(__GNUC__)
        #include "fallback.hpp"
 
    #elif defined(FORCED_CXX_STANDARD) // Fored version

        #if FORCED_CXX_STANDARD >= 20
            #include "c++20.hpp"
        #elif FORCED_CXX_STANDARD >= 17
            #include "c++17.hpp"
        #elif FORCED_CXX_STANDARD >= 14
            #include "c++14.hpp"
        #else
            #include "fallback.hpp"
        #endif

    #else // No forced version 

        #if __cplusplus >= 202002L
            #include "c++20.hpp"
        #elif __cplusplus >= 201703L
            #include "c++17.hpp"
        #elif __cplusplus >= 201402L
            #include "c++14.hpp"
        #else
            #include "fallback.hpp"
        #endif
    
    #endif

    // fallback (x86-64)
    #include <new> // std::hardware_destructive_interference_size, std::hardware_constructive_interference_size
    #if !defined(__cpp_lib_hardware_interference_size)
        #warning "fallback (x86-64) - hardware interference size = 64"
        #include <cstddef>  // std::size_t
        namespace std {
            inline constexpr std::size_t hardware_destructive_interference_size = 64;
            inline constexpr std::size_t hardware_constructive_interference_size = 64;
        }
    #endif

#endif /* ATTRIBUTE_H */
