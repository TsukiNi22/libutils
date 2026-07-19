/**************************************************************\
Edition:
##  @date 19/07/2026 by @author Tsukini

File Name:
##  @file Attribute-fallback.hpp

File Description:
##  Different attribute used for optimisation & other thing
##  Fallback if the version used is inferior to c++14
\**************************************************************/

#ifndef ATTRIBUTE_FALLBACK_H
    #define ATTRIBUTE_FALLBACK_H

    //----------------------------------------------------------------//
    /* DEFINE */

    /* attributes */
    #define _nodiscard
    #define _noinline
    #define _unused
    #define _hidden
    #define _ctor
    #define _dtor
    #define _deprecated(info)
    #define _fallthrough

    /* branch prediction */
    #define _likely
    #define _unlikely
    #define likely_c(c)     (c)
    #define unlikely_c(c)   (c)
    #define expect(c, v)    (c)

    /* optimisation */
    #define _assume(expr)
    #define _cold
    #define _hot

    /* binary layout */
    #define _noaddress
    #define _packed
    #define _alignas(n)

    /* allocation hints */
    #define _alloc_size(i_size)
    #define _alloc_size_mul(i_mul, i_size)
    #define _write_only(i_ptr, i_size)
    #define _read_only(i_ptr, i_size)
    #define _nonnull(i_ptr)

#endif /* ATTRIBUTE_FALLBACK_H */
