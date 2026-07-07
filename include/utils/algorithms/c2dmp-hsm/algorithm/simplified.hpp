/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 08/07/2026 by @author Tsukini

File Name:
##  @file simplified.cpp

File Description:
##  Algorithm used to determine the distance between 2 word
##
##  n = a.size()
##  m = a.size()
##  k = sizeof(UINT) → can be 1, 2, 4 or 8
##
##  Time:
##      bast → O(m + min(n, m))
##      moy  → O(m + min(n, m))
##      wort → O(m + min(n, m))
##
##  Memory:
##      best → O(1) → const (637)
##      moy  → O(1) → const (271 * k + 366)
##      wort → O(1) → const (2534)
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include <string_view>  // std::string_view
#include <algorithm>    // std::clamp, std::min, std::max
#include <cstdint>      // std::uint_fast8_t
#include <cstddef>      // std::size_t
#include <array>        // std::array
#include <new>          // std::hardware_destructive_interference_size

namespace utils::algorithms::c2dmp { // namespace start

#ifndef C2DMP_HSM_NORMALIZE_LOOKUP_TABLE
    #define C2DMP_HSM_NORMALIZE_LOOKUP_TABLE
static consteval inline std::array<unsigned char, 256> make_lookup_table()
{
    std::array<unsigned char, 256> table{};

    // Default char
    for (std::size_t i = 0; i < 256; ++i)
        table[i] = static_cast<unsigned char>(i);

    // To lower case
    for (unsigned char x = 'A'; x <= 'Z'; ++x)
        table[x] = x + 32;

    // Special char
    table[0xE2]='a'; table[0xE4]='a'; table[0xE3]='a'; table[0xE5]='a';
    table[0xC0]='a'; table[0xC1]='a'; table[0xC2]='a'; table[0xC4]='a'; table[0xC3]='a'; table[0xC5]='a';
    table[0xE7]='c'; table[0xC7]='c';
    table[0xE9]='e'; table[0xE8]='e'; table[0xEA]='e'; table[0xEB]='e';
    table[0xC9]='e'; table[0xC8]='e'; table[0xCA]='e'; table[0xCB]='e';
    table[0xEE]='i'; table[0xEF]='i'; table[0xED]='i'; table[0xEC]='i';
    table[0xCE]='i'; table[0xCF]='i'; table[0xCD]='i'; table[0xCC]='i';
    table[0xF1]='n'; table[0xD1]='n';
    table[0xF4]='o'; table[0xF6]='o'; table[0xF2]='o'; table[0xF3]='o'; table[0xF5]='o';
    table[0xD4]='o'; table[0xD6]='o'; table[0xD2]='o'; table[0xD3]='o'; table[0xD5]='o';
    table[0xF9]='u'; table[0xFA]='u'; table[0xFB]='u'; table[0xFC]='u';
    table[0xD9]='u'; table[0xDA]='u'; table[0xDB]='u'; table[0xDC]='u';
    table[0xFD]='y'; table[0xFF]='y'; table[0xDD]='y';

    return table;
}

// case insensitive lookup table
set_padding(std::hardware_destructive_interference_size) static constexpr inline std::array<unsigned char, 256> lookup_table = make_lookup_table();
unused // remove warning, due to inline the funtion isn't really used directly
static inline unsigned char normalize(unsigned char c) {return lookup_table[c];}
#endif /* C2DMP_HSM_NORMALIZE_LOOKUP_TABLE */

// case sensitive
#ifndef C2DMP_HSM_SIMPLIFIED
    #define C2DMP_HSM_SIMPLIFIED
template<std::uint_fast8_t prefixDepthSearch = 3, typename UINTN = std::uint_fast8_t>
[[deprecated("This version isn't the most optimized one, you should use c2dmp_optimized or c2dmp")]]
float c2dmp_simplified(const std::string_view a, const std::string_view b)
{
    // Check given type
    static_assert(std::unsigned_integral<UINTN>, "Must be an unsigned integral");
    static_assert(sizeof(UINTN) >= 1 && sizeof(UINTN) <= 8, "Must be between 1 bytes (uint8_t) & 8 bytes (uint64_t) (with limit)");
    static_assert(prefixDepthSearch >= 1 && prefixDepthSearch <= 5, "Must be between 1 & 5 (with limit)"); // Limits for optimisation, after the 5 first letter there is no usefull need to check this

    // Var init
    std::size_t as = a.size();
    std::size_t bs = b.size();
    std::size_t min = std::min(as, bs);
    std::size_t max = std::max(as, bs);
    set_padding(std::hardware_destructive_interference_size) UINTN cc[256] = {0};
    UINTN missplaced_char = 0;
    UINTN prefix_sizes[prefixDepthSearch] = {0};
    UINTN prefixs[prefixDepthSearch] = {0};
    UINTN prefix_size = 0;
    UINTN prefix_depth = 0;
    float dist = (max - min); // Remove already know char that are diff 'char' <> 'none'
    float coef = 1.f;
    float upper_limit = 2.f;
    unsigned char ca = '\0';
    unsigned char cb = '\0';
    bool same = 0;

    // Init the char count
    for (std::size_t i = 0; i < bs; ++i)
        ++(cc[normalize(b[i])]);

    // Compute difference weigth & other counter
    for (std::size_t i = 0; i < min; ++i) {
        // basic call & condition
        ca = normalize(a[i]);
        cb = normalize(b[i]);
        same = (ca == cb);

        // difference computing
        if (!same)
            ++dist;
        else if (a[i] == b[i] && a[i] != ca && b[i] != cb)
            dist -= .5f;

        // missplaced char computing
        if (!same && cc[ca] > 0) {
            ++missplaced_char;
            --(cc[ca]);
        }
        else if (same && cc[cb] == 0)
            --missplaced_char;
        else if (same && cc[cb] > 0)
            --(cc[cb]);

        // prefix depth for max size computing
        for (std::size_t j = 0; j < prefixDepthSearch; ++j) {
            prefixs[j] |= (i == j);
            prefixs[j] &= (ca == normalize(b[prefix_sizes[j]]));
            prefix_sizes[j] += prefixs[j];
            if (prefix_size < prefix_sizes[j]) { // new max
                prefix_depth = (i - (prefix_sizes[j] - 1));
                prefix_size = prefix_sizes[j];
            }
        }
    }

    // Compute missplaced char weigth
    // 1.01 <= coef <= 1.25
    // 1.01 -> 0 char diff
    // 1.25 -> 10 char diff
    coef = 1.01f + ((max - min) / 10.f) * .25f;
    coef = std::clamp(coef, 1.01f, 1.25f);
    dist -= missplaced_char * coef;

    // Compute prefix weigth
    // 0 <= k <= 2
    // 0 <= coef <= k
    upper_limit = 2;
    for (std::size_t i = 0; i < prefix_depth; ++i)
        upper_limit *= (1 - (i / prefixDepthSearch));
    coef = (upper_limit * (prefix_size / static_cast<float>(as)));
    dist -= prefix_size * coef;

    return dist;
}
#endif /* C2DMP_HSM_SIMPLIFIED */

} // namespace end
