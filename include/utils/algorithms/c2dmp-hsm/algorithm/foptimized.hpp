/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 29/05/2026 by @author Tsukini

File Name:
##  @file foptimized.cpp

File Description:
##  Algorithm used to determine the distance between 2 word
##
##  n = a.size()
##  m = a.size()
##  k = sizeof(UINT) → can be 1, 2, 4 or 8
##
##  Time:
##      bast → O(m + n)
##      moy  → O(m + n)
##      wort → O(m + n)
##
##  Memory:
##      best → O(1) → const (637)
##      moy  → O(1) → const (271 * k + 366)
##      wort → O(1) → const (2534)
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include <string_view>  // std::string_view
#include <cstdint>      // std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t
#include <cstddef>      // std::size_t
#include <cstring>      // std::memset
#include <array>        // std::array
#include <new>          // std::hardware_destructive_interference_size

namespace utils::algorithms::c2dmp { // namespace start

#ifndef C2DMP_HSM_NORMALIZE_LOOKUP_TABLE
    #define C2DMP_HSM_NORMALIZE_LOOKUP_TABLE
static consteval inline std::array<unsigned char, 256> make_lookup_table()
{
    std::array<unsigned char, 256> table;

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

// upper_limit pre computing
/*
 * float upper_limit = 2;
 * for (siz_t i = 0; i < prefix_depth; ++i)
 *     upper_limit *= (1 - (i / prefixDepthSearch));
*/
#ifndef C2DMP_HSM_UPPER_LIMIT_LOOKUP_TABLE
    #define C2DMP_HSM_UPPER_LIMIT_LOOKUP_TABLE
static constexpr inline float upper_limit_table[5][5] = {
    {2.0f, 0.0f,      0.0f,      0.0f,    0.0f},
    {2.0f, 1.0f,      0.0f,      0.0f,    0.0f},
    {2.0f, 1.333333f, 0.444444f, 0.0f,    0.0f},
    {2.0f, 1.5f,      0.75f,     0.1875f, 0.0f},
    {2.0f, 1.6f,      0.96f,     0.384f,  0.0768f},
};
#endif /* C2DMP_HSM_UPPER_LIMIT_LOOKUP_TABLE */

// case sensitive
#ifndef C2DMP_FHSM_OPTIMIZED
    #define C2DMP_FHSM_OPTIMIZED
template<std::uint32_t prefixDepthSearch = 3, typename UINTN = std::uint32_t>
float c2dmp_foptimized(const std::string_view a, const std::string_view b)
{
    // Check given type
    static_assert(std::unsigned_integral<UINTN>, "Must be an unsigned integral");
    static_assert(sizeof(UINTN) >= 1 && sizeof(UINTN) <= 8, "Must be between 1 bytes (uint8_t) & 8 bytes (uint64_t) (with limit)");
    static_assert(prefixDepthSearch >= 1 && prefixDepthSearch <= 5, "Must be between 1 & 5 (with limit)"); // Limits for optimisation, after the 5 first letter there is no usefull need to check this

    // Var init
    std::size_t as = a.size();
    std::size_t bs = b.size();
    std::size_t min = (as < bs) ? as : bs;
    std::size_t max = (as > bs) ? as : bs;
    set_padding(std::hardware_destructive_interference_size) UINTN cc[256];
    std::memset(cc, 0, sizeof(cc));
    UINTN missplaced_char = 0;
    UINTN prefix_size0 = 0, prefix_size1 = 0, prefix_size2 = 0, prefix_size3 = 0, prefix_size4 = 0;
    UINTN prefix_size = 0;
    UINTN prefix0 = 0, prefix1 = 0, prefix2 = 0, prefix3 = 0, prefix4 = 0; // boolean
    UINTN prefix_depth = 0;
    UINTN same = 0; // boolean
    UINTN tmp = 0; // boolean
    float dist = (max - min); // Remove already know char that are diff 'char' <> 'none'
    float coef = 1.0f;
    unsigned char ca = '\0';
    unsigned char cb = '\0';

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
        dist += !same;
        dist -= ((a[i] != ca) && (a[i] == b[i])) * 0.5f;

        // missplaced char computing
        tmp = (!same & (cc[ca] > 0));
        missplaced_char += tmp;
        cc[ca] -= tmp;
        missplaced_char -= (same & (cc[cb] == 0));
        cc[cb] -= (same & (cc[cb] > 0));

        // prefix depth for max size computing
        if constexpr (prefixDepthSearch >= 1) {
            prefix0 |= (i == 0);
            prefix0 &= (ca == normalize(b[prefix_size0]));
            prefix_size0 += prefix0;
            tmp = (prefix_size < prefix_size0);
            prefix_depth += tmp * ((i - (prefix_size0 - 1)) - prefix_depth);
            prefix_size += tmp * (prefix_size0 - prefix_size);
        }
        if constexpr (prefixDepthSearch >= 2) {
            prefix1 |= (i == 1);
            prefix1 &= (ca == normalize(b[prefix_size1]));
            prefix_size1 += prefix1;
            tmp = (prefix_size < prefix_size1);
            prefix_depth += tmp * ((i - (prefix_size1 - 1)) - prefix_depth);
            prefix_size += tmp * (prefix_size1 - prefix_size);
        }
        if constexpr (prefixDepthSearch >= 3) {
            prefix2 |= (i == 2);
            prefix2 &= (ca == normalize(b[prefix_size2]));
            prefix_size2 += prefix2;
            tmp = (prefix_size < prefix_size2);
            prefix_depth += tmp * ((i - (prefix_size2 - 1)) - prefix_depth);
            prefix_size += tmp * (prefix_size2 - prefix_size);
        }
        if constexpr (prefixDepthSearch >= 4) {
            prefix3 |= (i == 3);
            prefix3 &= (ca == normalize(b[prefix_size3]));
            prefix_size3 += prefix3;
            tmp = (prefix_size < prefix_size3);
            prefix_depth += tmp * ((i - (prefix_size3 - 1)) - prefix_depth);
            prefix_size += tmp * (prefix_size3 - prefix_size);
        }
        if constexpr (prefixDepthSearch >= 5) {
            prefix4 |= (i == 4);
            prefix4 &= (ca == normalize(b[prefix_size4]));
            prefix_size4 += prefix4;
            tmp = (prefix_size < prefix_size4);
            prefix_depth += tmp * ((i - (prefix_size4 - 1)) - prefix_depth);
            prefix_size += tmp * (prefix_size4 - prefix_size);
        }
    }

    // Compute missplaced char computing (full)
    for (std::size_t i = min; i < as; ++i) {
        // basic call & condition
        ca = normalize(a[i]);

        // missplaced char computing
        tmp = (cc[ca] > 0);
        missplaced_char += tmp;
        cc[ca] -= tmp;
    }

    // Compute missplaced char weigth
    // 1.01 <= coef <= 1.25
    // 1.01 -> 0 char diff
    // 1.25 -> 10 char diff
    coef = 1.01f + ((max - min) / 10.0f) * 0.25f;
    coef = (coef < 1.01f) ? 1.01f : coef;
    coef = (coef > 1.25f) ? 1.25f : coef;
    dist -= missplaced_char * coef;

    // Compute prefix weigth
    // 0 <= k <= 2
    // 0 <= coef <= k
    coef = (upper_limit_table[prefixDepthSearch - 1][prefix_depth] * (prefix_size / static_cast<float>(as)));
    dist -= prefix_size * coef;

    return dist;
}
#endif /* C2DMP_FHSM_OPTIMIZED */

} // namespace end
