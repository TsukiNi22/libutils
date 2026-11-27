/**************************************************************\
Edition:
##  @date 13/11/2026 by @author Tsukini

File Name:
##  @file c99.h

File Description:
##  Different attribute used for optimisation & other thing
##  Version for c99 (or gnu89) with a GNU compiler (same macros as c++11.hpp)
\**************************************************************/

#ifndef ATTRIBUTE_C99_H
    #define ATTRIBUTE_C99_H

    //----------------------------------------------------------------//
    /* DEFINE */

    /* attributes */
    #define _nodiscard              __attribute__((warn_unused_result)) // Warn for unused return
    #define _unused                 __attribute__((unused))             // Signal an unused variable
    #define _noreturn               __attribute__((noreturn))           // Signal a function that never return
    #define _fallthrough            __attribute__((fallthrough))        // Ingore warn for no break in switch
    #if defined(DEPRECATED_WARNING) && !defined(NO_DEPRECATED_WARNING)
        #define _deprecated(info)   __attribute__((deprecated(info)))   // Signal a deprecated function
    #else
        #define _deprecated(info)                                       // Not defined with this flag
    #endif

    /* visibility */
    #ifdef _WIN32
        #define _hidden                                         // No ELF visibility on Windows (dllexport/dllimport model)
        #define _export __declspec(dllexport)                   // Export a symbol from the dll (C API)
        #define _import __declspec(dllimport)                   // Import a symbol from the dll (C API)
    #else
        #define _hidden __attribute__((visibility("hidden")))   // Change the visibility on a shared lib
        #define _export __attribute__((visibility("default")))  // Export a symbol from the shared lib (C API)
        #define _import                                         // Nothing to import with the ELF visibility model
    #endif

    /* lifetime */
    #define _ctor           __attribute__((constructor))    // Execute before the main
    #define _dtor           __attribute__((destructor))     // Execute after the main
    #define _cleanup(fn)    __attribute__((cleanup(fn)))    // Call fn(&var) when the variable goes out of scope

    /* inlining */
    #define _always_inline  __attribute__((always_inline))  // Always inline the function (with inline)
    #define _noinline       __attribute__((noinline))       // Cancel any auto inline from the compiler
    #define _flatten        __attribute__((flatten))        // Inline every call made inside the function

    /* optimisation */
    #define _hot     __attribute__((hot))     // Signal a function that has a huge number of use
    #define _cold    __attribute__((cold))    // Signal a function that has a small number of use
    #define _const   __attribute__((const))   // Function without side effect that only reads its arguments (no memory)
    #define _pure    __attribute__((pure))    // Function without side effect that can read the memory
    #define _nothrow __attribute__((nothrow)) // Signal a function that never throws an exception (the C API)
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
    #define _likely                                     // Not available in c (no statement attribute for it, use _likely_c)
    #define _unlikely                                   // Not available in c (no statement attribute for it, use _unlikely_c)
    #define _likely_c(c)    __builtin_expect(!!(c), 1)  // Signal a condition that has a bigger probability of appening
    #define _unlikely_c(c)  __builtin_expect(!!(c), 0)  // Signal a condition that has a smallest probability of appening
    #define _expect(c, v)   __builtin_expect(c, v)      // Signal a condition that has a high probability of having the given value

    /* diagnostics */
    #define _format(archetype, i_fmt, i_args) __attribute__((format(archetype, i_fmt, i_args))) // Check the format string of the given argument (printf, scanf, ...) against the arguments from i_args
    #define _sentinel                         __attribute__((sentinel))                         // Warn when a variadic function is not ended by a NULL
    #define _warning(msg)                     __attribute__((warning(msg)))                     // Warn at the compilation when the function is called
    #define _error(msg)                       __attribute__((error(msg)))                       // Fail the compilation when the function is called
    #if !defined(__clang__) && __GNUC__ >= 13 // gcc 13 and above (checked by -fanalyzer)
        #define _fd_arg(i_fd)       __attribute__((fd_arg(i_fd)))       // Signal an open file descriptor in the given argument
        #define _fd_arg_read(i_fd)  __attribute__((fd_arg_read(i_fd)))  // Signal a file descriptor open for reading in the given argument
        #define _fd_arg_write(i_fd) __attribute__((fd_arg_write(i_fd))) // Signal a file descriptor open for writing in the given argument
    #else
        #define _fd_arg(i_fd)       // Not supported by clang and gcc < 13
        #define _fd_arg_read(i_fd)  // Not supported by clang and gcc < 13
        #define _fd_arg_write(i_fd) // Not supported by clang and gcc < 13
    #endif
    #if !defined(__clang__) && __GNUC__ >= 14 // gcc 14 and above (checked by -fanalyzer)
        #define _null_terminated_string_arg(i_str) __attribute__((null_terminated_string_arg(i_str))) // Signal a '\0' terminated string in the given argument
    #else
        #define _null_terminated_string_arg(i_str) // Not supported by clang and gcc < 14
    #endif

    /* binary layout */
    #define _noaddress                          // Not available in c (c++ only)
    #define _packed     __attribute__((packed)) // Remove the memory padding in a struct

    /* memory */
    #define _malloc                        __attribute__((malloc))                    // Signal a function that return a new memory (no alias)
    #define _alloc_size(i_size)            __attribute__((alloc_size(i_size)))        // Signal a malloc of the given size in the argument
    #define _alloc_size_mul(i_mul, i_size) __attribute__((alloc_size(i_mul, i_size))) // Signal a malloc of the given size in the argument multiply by another argument
    #define _nonnull(i_ptr)                __attribute__((nonnull(i_ptr)))            // Warn on given null value on the given argument
    #define _returns_nonnull               __attribute__((returns_nonnull))           // Signal a function that never return a null pointer
    #define _assume_aligned(i_align)       __attribute__((assume_aligned(i_align)))   // Signal a returned pointer aligned on i_align bytes
    #ifdef __clang__
        #define _read_only(i_ptr, i_size)                                                       // Not supported by clang
        #define _write_only(i_ptr, i_size)                                                      // Not supported by clang
        #define _read_write(i_ptr, i_size)                                                      // Not supported by clang
    #else
        #define _read_only(i_ptr, i_size)   __attribute__((access(read_only, i_ptr, i_size)))   // Set the mode of a ptr in the argument so that it can only be readed in the limit of the size
        #define _write_only(i_ptr, i_size)  __attribute__((access(write_only, i_ptr, i_size)))  // Set the mode of a ptr in the argument so that it can only be writed in the limit of the size
        #define _read_write(i_ptr, i_size)  __attribute__((access(read_write, i_ptr, i_size)))  // Set the mode of a ptr in the argument so that it can be readed & writed in the limit of the size
    #endif
    #define _lifetimebound // Not available in c (c++ only)

#endif /* ATTRIBUTE_C99_H */
