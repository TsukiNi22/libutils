/**************************************************************\
Edition:
##  @date 13/11/2026 by @author Tsukini

File Name:
##  @file c++17.hpp

File Description:
##  Different attribute used for optimisation & other thing
##  Version for c++17 and above
\**************************************************************/

#ifndef ATTRIBUTE_CPP17_H
    #define ATTRIBUTE_CPP17_H

    //----------------------------------------------------------------//
    /* DEFINE */

    /* attributes */
    #define _nodiscard              [[nodiscard]]           // Warn for unused return
    #define _unused                 [[maybe_unused]]        // Signal an unused variable
    #define _noreturn               [[noreturn]]            // Signal a function that never return
    #define _fallthrough            [[fallthrough]]         // Ingore warn for no break in switch
    #if defined(DEPRECATED_WARNING) && !defined(NO_DEPRECATED_WARNING)
        #define _deprecated(info)   [[deprecated(info)]]    // Signal a deprecated function
    #else
        #define _deprecated(info)                           // Not defined with this flag
    #endif

    /* visibility */
    #ifdef _WIN32
        #define _hidden                                 // No ELF visibility on Windows (dllexport/dllimport model)
        #define _export __declspec(dllexport)           // Export a symbol from the dll (C API)
        #define _import __declspec(dllimport)           // Import a symbol from the dll (C API)
    #else
        #define _hidden [[gnu::visibility("hidden")]]   // Change the visibility on a shared lib
        #define _export [[gnu::visibility("default")]]  // Export a symbol from the shared lib (C API)
        #define _import                                 // Nothing to import with the ELF visibility model
    #endif

    /* lifetime */
    #define _ctor           [[gnu::constructor]]    // Execute before the main
    #define _dtor           [[gnu::destructor]]     // Execute after the main
    #define _cleanup(fn)    [[gnu::cleanup(fn)]]    // Call fn(&var) when the variable goes out of scope

    /* inlining */
    #define _always_inline  [[gnu::always_inline]]  // Always inline the function (with inline)
    #define _noinline       [[gnu::noinline]]       // Cancel any auto inline from the compiler
    #define _flatten        [[gnu::flatten]]        // Inline every call made inside the function

    /* optimisation */
    #define _hot    [[gnu::hot]]    // Signal a function that has a huge number of use
    #define _cold   [[gnu::cold]]   // Signal a function that has a small number of use
    #define _const  [[gnu::const]]  // Function without side effect that only reads its arguments (no memory)
    #define _pure   [[gnu::pure]]   // Function without side effect that can read the memory
    // Assume a given expr as true
    #if defined(__clang__)
        #define _assume(expr)   __builtin_assume(expr)
    #elif __GNUC__ >= 13 // gcc 13 and above
        #define _assume(expr)   __attribute__((assume(expr)))
    #else
        #define _assume(expr)   ((expr) ? (void)0 : __builtin_unreachable())    // expr evaluated
    #endif
    #define _unreachable()      __builtin_unreachable()                         // Signal a code that can never be reached

    /* branch prediction */
    #define _likely                                     // Not defined in this version
    #define _unlikely                                   // Not defined in this version
    #define _likely_c(c)    __builtin_expect(!!(c), 1)  // Signal a condition that has a bigger probability of appening
    #define _unlikely_c(c)  __builtin_expect(!!(c), 0)  // Signal a condition that has a smallest probability of appening
    #define _expect(c, v)   __builtin_expect(c, v)      // Signal a condition that has a high probability of having the given value

    /* diagnostics */
    #define _format(archetype, i_fmt, i_args)   [[gnu::format(archetype, i_fmt, i_args)]]   // Check the format string of the given argument (printf, scanf, ...) against the arguments from i_args
    #define _sentinel                           [[gnu::sentinel]]                           // Warn when a variadic function is not ended by a NULL

    /* binary layout */
    #define _noaddress                  // Not defined in this version
    #define _packed     [[gnu::packed]] // Remove the memory padding in a struct

    /* memory */
    #define _malloc                         [[gnu::malloc]]                             // Signal a function that return a new memory (no alias)
    #define _alloc_size(i_size)             [[gnu::alloc_size(i_size)]]                 // Signal a malloc of the given size in the argument
    #define _alloc_size_mul(i_mul, i_size)  [[gnu::alloc_size(i_mul, i_size)]]          // Signal a malloc of the given size in the argument multiply by another argument
    #define _nonnull(i_ptr)                 [[gnu::nonnull(i_ptr)]]                     // Warn on given null value on the given argument
    #define _returns_nonnull                [[gnu::returns_nonnull]]                    // Signal a function that never return a null pointer
    #ifdef __clang__
        #define _read_only(i_ptr, i_size)                                               // Not supported by clang
        #define _write_only(i_ptr, i_size)                                              // Not supported by clang
    #else
        #define _read_only(i_ptr, i_size)   [[gnu::access(read_only, i_ptr, i_size)]]   // Set the mode of a ptr in the argument so that it can only be readed in the limit of the size
        #define _write_only(i_ptr, i_size)  [[gnu::access(write_only, i_ptr, i_size)]]  // Set the mode of a ptr in the argument so that it can only be writed in the limit of the size
    #endif

#endif /* ATTRIBUTE_CPP17_H */
