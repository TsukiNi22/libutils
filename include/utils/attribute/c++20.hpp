/**************************************************************\
Edition:
##  @date 20/07/2026 by @author Tsukini

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
    #define _nodiscard      [[nodiscard]]                   // Warn for unused return
    #define _noinline       [[noinline]]                    // Cancel any auto inline from the compiler
    #define _unused         [[maybe_unused]]                // Signal an unused variable
    #define _hidden         [[gnu::visibility("hidden")]]   // Change the visibility on a shared lib
    #define _ctor           [[gnu::constructor]]            // Execute before the main
    #define _dtor           [[gnu::destructor]]             // Execute after the main
    #define _fallthrough    [[fallthrough]]                 // Ingore warn for no break in switch
    #ifndef NO_DEPRECATED_WARNING
        #define _deprecated(info)   [[deprecated(info)]]    // Signal a deprecated function
    #else
        #define _deprecated(info)                           // Not defined with this flag
    #endif

    /* branch prediction */
    #define _likely         [[likely]]                  // Signal a condition that has a bigger probability of appening
    #define _unlikely       [[unlikely]]                // Signal a condition that has a smallest probability of appening
    #define likely_c(c)     __builtin_expect(!!(c), 1)  // Signal a condition that has a bigger probability of appening
    #define unlikely_c(c)   __builtin_expect(!!(c), 0)  // Signal a condition that has a smallest probability of appening
    #define expect(c, v)    __builtin_expect(c, v)      // Signal a condition that has a high probability of having the given value

    /* optimisation */
    #define _assume(expr)   [[assume(expr)]]    // Assume a given expr as true
    #define _cold           [[gnu::cold]]       // Signal a function that has a small number of use
    #define _hot            [[gnu::hot]]        // Signal a function that has a huge number of use

    /* binary layout */
    #define _noaddress  [[no_unique_address]]   // Remove unique address for elements
    #define _packed     [[gnu::packed]]         // Remove the memory padding in a struct
    #define _alignas(n) alignas(n)              // Set the memory padding

    /* allocation hints */
    #define _alloc_size(i_size)             [[gnu::alloc_size(i_size)]]                 // Signal a malloc of the given size in the argument
    #define _alloc_size_mul(i_mul, i_size)  [[gnu::alloc_size(i_mul, i_size)]]          // Signal a malloc of the given size in the argument multiply by another argument
    #define _write_only(i_ptr, i_size)      [[gnu::access(write_only, i_ptr, i_size)]]  // Set the mode of a ptr in the argument so that it can only be writed in the limit of the size
    #define _read_only(i_ptr, i_size)       [[gnu::access(read_only, i_ptr, i_size)]]   // Set the mode of a ptr in the argument so that it can only be readed in the limit of the size
    #define _nonnull(i_ptr)                 [[gnu::nonnull(i_ptr)]]                     // Warn on given null value on the given argument

#endif /* ATTRIBUTE_CPP20_H */
