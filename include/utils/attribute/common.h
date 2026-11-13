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
##  @file common.h

File Description:
##  Keywords & values that are not attributes, common to every c version (same as common.hpp)
##  (thread_local, restrict, alignas, hardware interference size)
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (ATTRIBUTE_COMMON_H is the guard of common.hpp)

#ifndef ATTRIBUTE_COMMON_C_H
    #define ATTRIBUTE_COMMON_C_H

    //----------------------------------------------------------------//
    /* DEFINE */

    /* storage */
    // One instance of the variable per thread
    #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L // c23 and above
        #define _thread_local   thread_local
    #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L // c11 and above
        #define _thread_local   _Thread_local
    #elif defined(__GNUC__)
        #define _thread_local   __thread
    #elif defined(_MSC_VER)
        #define _thread_local   __declspec(thread)
    #else
        #define _thread_local // Not available with this compiler: not thread local
    #endif

    /* pointer */
    // Signal a pointer without alias (no other pointer to the same memory)
    #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L // c99 and above
        #define _restrict   restrict
    #elif defined(__GNUC__)
        #define _restrict   __restrict__
    #elif defined(_MSC_VER)
        #define _restrict   __restrict
    #else
        #define _restrict // Not available with this compiler
    #endif

    /* binary layout */
    // Set the memory padding
    #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L // c23 and above
        #define _alignas(n) alignas(n)
    #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L // c11 and above
        #define _alignas(n) _Alignas(n)
    #elif defined(__GNUC__)
        #define _alignas(n) __attribute__((aligned(n)))
    #elif defined(_MSC_VER)
        #define _alignas(n) __declspec(align(n))
    #else
        #define _alignas(n) // Not available with this compiler
    #endif

//----------------------------------------------------------------//
/* GLOBAL */

// fallback (x86-64): c version of std::hardware_*_interference_size (enum: constant usable in _alignas)
enum {
    hardware_destructive_interference_size = 64,    // min offset between two objects to avoid false sharing
    hardware_constructive_interference_size = 64    // max size of a contiguous memory to promote true sharing
};

#endif /* ATTRIBUTE_COMMON_C_H */
