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
##  @file simplified.hpp

File Description:
##  Algorithm used to determine the distance between 2 words
##
##  n = a.size()
##  m = b.size()
##  k = sizeof(UIntT) → can be 1, 2, 4 or 8
##
##  Time:
##      best  → O(m + min(n, m))
##      moy   → O(m + min(n, m))
##      worst → O(m + min(n, m))
##
##  Memory:
##      best  → O(1) → const (637)
##      moy   → O(1) → const (271 * k + 366)
##      worst → O(1) → const (2534)
\**************************************************************/

#ifndef C2DMP_SIMPLIFIED_H
    #define C2DMP_SIMPLIFIED_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "utils/attribute/Attribute.hpp" // _hot, _nodiscard, _unused, _unlikely, _deprecated, _alignas
    #include <new>          // std::hardware_destructive_interference_size
    #include <algorithm>    // std::clamp, std::min, std::max
    #include <type_traits>  // std::is_same_v
    #include <string_view>  // std::string_view
    #include <concepts>     // std::unsigned_integral
    #include <limits>       // std::numeric_limits
    #include <cstdint>      // std::uint_fast8_t
    #include <cstddef>      // std::size_t
    #include <array>        // std::array

namespace utils::algorithms::c2dmp { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* normalize */
#ifndef C2DMP_HSM_NORMALIZE_LOOKUP_TABLE
    #define C2DMP_HSM_NORMALIZE_LOOKUP_TABLE
_nodiscard static consteval inline std::array<unsigned char, 256> makeLookupTable_(void)
{
    std::array<unsigned char, 256> table{};

    // Default char
    for (std::size_t i = 0; i < 256; ++i)
        table[i] = static_cast<unsigned char>(i);

    // To lower case
    for (unsigned char x = 'A'; x <= 'Z'; ++x)
        table[x] = x + 32;

    // Special char (latin-1 accents)
    table[0xE2] = 'a'; table[0xE4] = 'a'; table[0xE3] = 'a'; table[0xE5] = 'a';
    table[0xC0] = 'a'; table[0xC1] = 'a'; table[0xC2] = 'a'; table[0xC4] = 'a'; table[0xC3] = 'a'; table[0xC5] = 'a';
    table[0xE7] = 'c'; table[0xC7] = 'c';
    table[0xE9] = 'e'; table[0xE8] = 'e'; table[0xEA] = 'e'; table[0xEB] = 'e';
    table[0xC9] = 'e'; table[0xC8] = 'e'; table[0xCA] = 'e'; table[0xCB] = 'e';
    table[0xEE] = 'i'; table[0xEF] = 'i'; table[0xED] = 'i'; table[0xEC] = 'i';
    table[0xCE] = 'i'; table[0xCF] = 'i'; table[0xCD] = 'i'; table[0xCC] = 'i';
    table[0xF1] = 'n'; table[0xD1] = 'n';
    table[0xF4] = 'o'; table[0xF6] = 'o'; table[0xF2] = 'o'; table[0xF3] = 'o'; table[0xF5] = 'o';
    table[0xD4] = 'o'; table[0xD6] = 'o'; table[0xD2] = 'o'; table[0xD3] = 'o'; table[0xD5] = 'o';
    table[0xF9] = 'u'; table[0xFA] = 'u'; table[0xFB] = 'u'; table[0xFC] = 'u';
    table[0xD9] = 'u'; table[0xDA] = 'u'; table[0xDB] = 'u'; table[0xDC] = 'u';
    table[0xFD] = 'y'; table[0xFF] = 'y'; table[0xDD] = 'y';

    return table;
}

_alignas(std::hardware_destructive_interference_size) static constexpr inline std::array<unsigned char, 256> lookupTable = makeLookupTable_(); // case insensitive lookup table
_unused _hot _nodiscard static inline unsigned char normalize_(const unsigned char c) {return lookupTable[c];}; // unused warning removed: always inlined, never really called
#endif /* C2DMP_HSM_NORMALIZE_LOOKUP_TABLE */

/* distance */
template<std::uint_fast8_t prefixDepthSearch = 3, typename UIntT = std::uint_fast8_t>
_deprecated("This version isn't the most optimized one, you should use c2dmp_optimized or c2dmp") _hot _nodiscard
inline float c2dmp_simplified(const std::string_view a, const std::string_view b)
{
    // Check given type
    static_assert(std::unsigned_integral<UIntT>, "Must be an unsigned integral");
    static_assert(sizeof(UIntT) >= 1 && sizeof(UIntT) <= 8, "Must be between 1 bytes (uint8_t) & 8 bytes (uint64_t) (with limit)");
    static_assert(prefixDepthSearch >= 1 && prefixDepthSearch <= 5, "Must be between 1 & 5 (with limit)"); // Limits for optimisation, after the 5 first letters there is no useful need to check this

    // Var init
    const std::size_t as = a.size();
    const std::size_t bs = b.size();

    // Empty 'a': only the length difference remains (avoid the NaN of 'prefixSize / as')
    if (as == 0) _unlikely
        return static_cast<float>(bs);

    // Counters overflow protection: fallback on a wider type when a size exceeds UIntT
    if constexpr (!std::is_same_v<UIntT, std::size_t>) {
        if (as > std::numeric_limits<UIntT>::max() || bs > std::numeric_limits<UIntT>::max()) _unlikely {
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wdeprecated-declarations" // self call of the deprecated version
            return c2dmp_simplified<prefixDepthSearch, std::size_t>(a, b);
            #pragma GCC diagnostic pop
        }
    }

    const std::size_t min = std::min(as, bs);
    const std::size_t max = std::max(as, bs);
    _alignas(std::hardware_destructive_interference_size) UIntT cc[256] = {0}; // <char -> count> of 'b'
    UIntT misplacedChar = 0;
    UIntT prefixSizes[prefixDepthSearch] = {0};
    UIntT prefixes[prefixDepthSearch] = {0}; // boolean
    UIntT prefixSize = 0;
    UIntT prefixDepth = 0;
    float dist = static_cast<float>(max - min); // Remove already known char that are diff 'char' <> 'none'
    float coef = 1.f;
    float upperLimit = 2.f;
    unsigned char ca = '\0';
    unsigned char cb = '\0';
    bool same = false;

    // Init the char count
    for (std::size_t i = 0; i < bs; ++i)
        ++(cc[normalize_(b[i])]);

    // Compute difference weight & other counters
    for (std::size_t i = 0; i < min; ++i) {
        // basic call & condition
        ca = normalize_(a[i]);
        cb = normalize_(b[i]);
        same = (ca == cb);

        // difference computing
        if (!same)
            ++dist;
        else if (a[i] == b[i] && static_cast<unsigned char>(a[i]) != ca && static_cast<unsigned char>(b[i]) != cb)
            dist -= .5f;

        // misplaced char computing
        if (!same && cc[ca] > 0) {
            ++misplacedChar;
            --(cc[ca]);
        } else if (same && cc[cb] == 0) {
            --misplacedChar;
        } else if (same && cc[cb] > 0) {
            --(cc[cb]);
        }

        // prefix depth for max size computing
        for (std::size_t j = 0; j < prefixDepthSearch; ++j) {
            prefixes[j] |= (i == j);
            prefixes[j] &= (ca == normalize_(b[prefixSizes[j]]));
            prefixSizes[j] += prefixes[j];
            if (prefixSize < prefixSizes[j]) { // new max
                prefixDepth = (i - (prefixSizes[j] - 1));
                prefixSize = prefixSizes[j];
            }
        }
    }

    // Compute misplaced char weight
    // 1.01 <= coef <= 1.25
    // 1.01 -> 0 char diff
    // 1.25 -> 10 char diff
    coef = 1.01f + ((max - min) / 10.f) * .25f;
    coef = std::clamp(coef, 1.01f, 1.25f);
    dist -= misplacedChar * coef;

    // Compute prefix weight
    // 0 <= k <= 2
    // 0 <= coef <= k
    upperLimit = 2;
    for (std::size_t i = 1; i <= prefixDepth; ++i)
        upperLimit *= (1.f - (static_cast<float>(i) / prefixDepthSearch));
    coef = (upperLimit * (prefixSize / static_cast<float>(as)));
    dist -= prefixSize * coef;

    return dist;
}

} // namespace end
#endif /* C2DMP_SIMPLIFIED_H */
