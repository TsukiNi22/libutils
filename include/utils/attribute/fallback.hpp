/**************************************************************\
Edition:
##  @date 19/05/2026 by @author Tsukini

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
    #define nodiscard
    #define noinline
    #define unused
    #define hidden
    #define ctor
    #define dtor
    #define outdated(info)
    #define fallthrough

    /* branch prediction */
    #define likely
    #define unlikely
    #define likely_c(c)     (c)
    #define unlikely_c(c)   (c)
    #define expect(c, v)    (c)

    /* optimisation */
    #define assume(expr)
    #define cold
    #define hot

    /* binary layout */
    #define remove_address
    #define remove_padding
    #define set_padding(n)

    /* allocation hints */
    #define alloc_size(i_size)
    #define alloc_size_mul(i_mul, i_size)
    #define write_only(i_ptr, i_size)
    #define read_only(i_ptr, i_size)
    #define nonnull(i_ptr)

#endif /* ATTRIBUTE_FALLBACK_H */
