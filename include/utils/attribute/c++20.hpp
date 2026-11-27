/**************************************************************\
Edition:
##  @date 13/11/2026 by @author Tsukini

File Name:
##  @file c++20.hpp

File Description:
##  Different attribute used for optimisation & other thing
##  Version for c++20 and above
\**************************************************************/

#ifndef ATTRIBUTE_CPP20_H
    #define ATTRIBUTE_CPP20_H

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
    #define _hot     [[gnu::hot]]     // Signal a function that has a huge number of use
    #define _cold    [[gnu::cold]]    // Signal a function that has a small number of use
    #define _const   [[gnu::const]]   // Function without side effect that only reads its arguments (no memory)
    #define _pure    [[gnu::pure]]    // Function without side effect that can read the memory
    #define _nothrow [[gnu::nothrow]] // Signal a function that never throws an exception (the C API)
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
    #define _likely         [[likely]]                  // Signal a condition that has a bigger probability of appening
    #define _unlikely       [[unlikely]]                // Signal a condition that has a smallest probability of appening
    #define _likely_c(c)    __builtin_expect(!!(c), 1)  // Signal a condition that has a bigger probability of appening
    #define _unlikely_c(c)  __builtin_expect(!!(c), 0)  // Signal a condition that has a smallest probability of appening
    #define _expect(c, v)   __builtin_expect(c, v)      // Signal a condition that has a high probability of having the given value

    /* diagnostics */
    #define _format(archetype, i_fmt, i_args) [[gnu::format(archetype, i_fmt, i_args)]] // Check the format string of the given argument (printf, scanf, ...) against the arguments from i_args
    #define _sentinel                         [[gnu::sentinel]]                         // Warn when a variadic function is not ended by a NULL
    #define _warning(msg)                     [[gnu::warning(msg)]]                     // Warn at the compilation when the function is called
    #define _error(msg)                       [[gnu::error(msg)]]                       // Fail the compilation when the function is called
    #if !defined(__clang__) && __GNUC__ >= 13 // gcc 13 and above (checked by -fanalyzer)
        #define _fd_arg(i_fd)       [[gnu::fd_arg(i_fd)]]       // Signal an open file descriptor in the given argument
        #define _fd_arg_read(i_fd)  [[gnu::fd_arg_read(i_fd)]]  // Signal a file descriptor open for reading in the given argument
        #define _fd_arg_write(i_fd) [[gnu::fd_arg_write(i_fd)]] // Signal a file descriptor open for writing in the given argument
    #else
        #define _fd_arg(i_fd)       // Not supported by clang and gcc < 13
        #define _fd_arg_read(i_fd)  // Not supported by clang and gcc < 13
        #define _fd_arg_write(i_fd) // Not supported by clang and gcc < 13
    #endif
    #if !defined(__clang__) && __GNUC__ >= 14 // gcc 14 and above (checked by -fanalyzer)
        #define _null_terminated_string_arg(i_str) [[gnu::null_terminated_string_arg(i_str)]] // Signal a '\0' terminated string in the given argument
    #else
        #define _null_terminated_string_arg(i_str) // Not supported by clang and gcc < 14
    #endif

    /* binary layout */
    #define _noaddress  [[no_unique_address]]   // Remove unique address for elements
    #define _packed     [[gnu::packed]]         // Remove the memory padding in a struct

    /* memory */
    #define _malloc                        [[gnu::malloc]]                    // Signal a function that return a new memory (no alias)
    #define _alloc_size(i_size)            [[gnu::alloc_size(i_size)]]        // Signal a malloc of the given size in the argument
    #define _alloc_size_mul(i_mul, i_size) [[gnu::alloc_size(i_mul, i_size)]] // Signal a malloc of the given size in the argument multiply by another argument
    #define _nonnull(i_ptr)                [[gnu::nonnull(i_ptr)]]            // Warn on given null value on the given argument
    #define _returns_nonnull               [[gnu::returns_nonnull]]           // Signal a function that never return a null pointer
    #define _assume_aligned(i_align)       [[gnu::assume_aligned(i_align)]]   // Signal a returned pointer aligned on i_align bytes
    #ifdef __clang__
        #define _read_only(i_ptr, i_size)                                               // Not supported by clang
        #define _write_only(i_ptr, i_size)                                              // Not supported by clang
        #define _read_write(i_ptr, i_size)                                              // Not supported by clang
    #else
        #define _read_only(i_ptr, i_size)   [[gnu::access(read_only, i_ptr, i_size)]]   // Set the mode of a ptr in the argument so that it can only be readed in the limit of the size
        #define _write_only(i_ptr, i_size)  [[gnu::access(write_only, i_ptr, i_size)]]  // Set the mode of a ptr in the argument so that it can only be writed in the limit of the size
        #define _read_write(i_ptr, i_size)  [[gnu::access(read_write, i_ptr, i_size)]]  // Set the mode of a ptr in the argument so that it can be readed & writed in the limit of the size
    #endif
    #ifdef __clang__
        #define _lifetimebound  [[clang::lifetimebound]]    // Warn when the returned reference outlives the object (after the cv of a method) or the given argument
    #else
        #define _lifetimebound                              // Only supported by clang
    #endif

#endif /* ATTRIBUTE_CPP20_H */
