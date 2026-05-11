/**************************************************************\
Edition:
##  @date 27/04/2026 by @author Tsukini

File Name:
##  @file Attribute-c++20.hpp

File Description:
##  Different attribute used for optimisation & other thing
##  Version for c++20 and above
\**************************************************************/

#ifndef ATTRIBUTE_CPP20_H
    #define ATTRIBUTE_CPP20_H

    //----------------------------------------------------------------//
    /* DEFINE */

    /* attributes */
    #define nodiscard       [[nodiscard]]                   // Warn for unused return
    #define noinline        [[noinline]]                    // Cancel any auto inline from the compiler
    #define unused          [[maybe_unused]]                // Signal an unused variable
    #define hidden          [[gnu::visibility("hidden")]]   // Change the visibility on a shared lib
    #define ctor            [[gnu::constructor]]            // Execute before the main
    #define dtor            [[gnu::destructor]]             // Execute after the main
    #define fallthrough     [[fallthrough]]                 // Ingore warn for no break in switch
    #ifndef _NoWarning
        #define outdated(info)  [[deprecated(info)]]        // Signal a deprecated function
    #else
        #define outdated(info)                              // Not defined with this flag
    #endif

    /* branch prediction */
    #define likely          [[likely]]                  // Signal a condition that has a bigger probability of appening
    #define unlikely        [[unlikely]]                // Signal a condition that has a smallest probability of appening
    #define likely_c(c)     __builtin_expect(!!(c), 1)  // Signal a condition that has a bigger probability of appening
    #define unlikely_c(c)   __builtin_expect(!!(c), 0)  // Signal a condition that has a smallest probability of appening
    #define expect(c, v)    __builtin_expect(c, v)      // Signal a condition that has a high probability of having the given value

    /* optimisation */
    #define assume(expr)    [[assume(expr)]]    // Assume a given expr as true
    #define cold            [[gnu::cold]]       // Signal a function that has a small number of use
    #define hot             [[gnu::hot]]        // Signal a function that has a huge number of use

    /* binary layout */
    #define remove_address  [[no_unique_address]]   // Remove unique address for elements
    #define remove_padding  [[gnu::packed]]         // Remove the memory padding in a struct
    #define set_padding(n)  alignas(n)              // Set the memory padding

    /* allocation hints */
    #define alloc_size(i_size)              [[gnu::alloc_size(i_size)]]                 // Signal a malloc of the given size in the argument
    #define alloc_size_mul(i_mul, i_size)   [[gnu::alloc_size(i_mul, i_size)]]          // Signal a malloc of the given size in the argument multiply by another argument
    #define write_only(i_ptr, i_size)       [[gnu::access(write_only, i_ptr, i_size)]]  // Set the mode of a ptr in the argument so that it can only be writed in the limit of the size
    #define read_only(i_ptr, i_size)        [[gnu::access(read_only, i_ptr, i_size)]]   // Set the mode of a ptr in the argument so that it can only be readed in the limit of the size
    #define nonnull(i_ptr)                  [[gnu::nonnull(i_ptr)]]                     // Warn for null on the given argument

#endif /* ATTRIBUTE_CPP20_H */
