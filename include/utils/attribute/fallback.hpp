/**************************************************************\
Edition:
##  @date 13/11/2026 by @author Tsukini

File Name:
##  @file fallback.hpp

File Description:
##  Different attribute used for optimisation & other thing
##  Fallback without a GNU compiler or with a too old version (c & c++)
\**************************************************************/

#ifndef ATTRIBUTE_FALLBACK_H
    #define ATTRIBUTE_FALLBACK_H

    //----------------------------------------------------------------//
    /* DEFINE */

    /* attributes */
    #define _nodiscard
    #define _unused
    #define _noreturn
    #define _fallthrough
    #define _deprecated(info)

    /* visibility */
    #define _hidden
    #ifdef _WIN32
        #define _export __declspec(dllexport)
        #define _import __declspec(dllimport)
    #else
        #define _export
        #define _import
    #endif

    /* lifetime */
    #define _ctor
    #define _dtor
    #define _cleanup(fn)

    /* inlining */
    #define _always_inline
    #define _noinline
    #define _flatten

    /* optimisation */
    #define _hot
    #define _cold
    #define _const
    #define _pure
    #define _nothrow
    #define _assume(expr)
    #define _unreachable() ((void)0)

    /* branch prediction */
    #define _likely
    #define _unlikely
    #define _likely_c(c)    (c)
    #define _unlikely_c(c)  (c)
    #define _expect(c, v)   (c)

    /* diagnostics */
    #define _format(archetype, i_fmt, i_args)
    #define _sentinel
    #define _warning(msg)
    #define _error(msg)
    #define _fd_arg(i_fd)
    #define _fd_arg_read(i_fd)
    #define _fd_arg_write(i_fd)
    #define _null_terminated_string_arg(i_str)

    /* binary layout */
    #define _noaddress
    #define _packed

    /* memory */
    #define _malloc
    #define _alloc_size(i_size)
    #define _alloc_size_mul(i_mul, i_size)
    #define _nonnull(i_ptr)
    #define _returns_nonnull
    #define _assume_aligned(i_align)
    #define _read_only(i_ptr, i_size)
    #define _write_only(i_ptr, i_size)
    #define _read_write(i_ptr, i_size)
    #define _lifetimebound

#endif /* ATTRIBUTE_FALLBACK_H */
