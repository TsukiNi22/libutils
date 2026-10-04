/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 15/07/2026 by @author Tsukini

File Name:
##  @file hash.hpp

File Description:
##  Include of the hash generation tools
\**************************************************************/

#ifndef HASH_H
    #define HASH_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "utils/attribute/Attribute.hpp"
    #include "../sosDefine.hpp" // SEED_ELEMENT_COUNT
    #include <algorithm>        // std::copy_n
    #include <concepts>         // std::unsigned_integral
    #include <iterator>         // std::distance
    #include <optional>         // std::optional
    #include <cstdint>          // std::uint_fast32_t, std::uint32_t
    #include <cstddef>          // std::size_t
    #include <random>           // std::mt19937
    #include <vector>           // std::vector
    #include <array>            // std::array
    #include <cmath>            // std::sin, std::abs
    #include <bit>              // std::rotl

namespace utils::algorithms::sos::tools { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

// Pass a sequence to the generator without modification (unlike std::seed_seq)
struct DirectSeedSequence {
    using result_type = std::uint32_t;
    const std::array<std::uint32_t, std::mt19937::state_size>& data;

    _hot _nodiscard inline std::size_t size(void) const noexcept {return this->data.size();};
    template<typename It>
    _hot inline void generate(const It first, const It last) const {(void)std::copy_n(this->data.begin(), std::distance(first, last), first);};
};

//----------------------------------------------------------------//
/* PROTOTYPE */

/* hash */
template<typename ByteT>
_hot _nodiscard inline std::uint_fast32_t hash(const std::vector<std::uint_fast32_t>& index, const std::vector<ByteT>& bytes)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");

    // Mix the last SEED_ELEMENT_COUNT valid values
    constexpr double magic = -1.460354508809587;
    std::uint_fast32_t seed = 0x811c9dc5;
    for (std::size_t i = 0; i < SEED_ELEMENT_COUNT; ++i) {
        double s = std::sin(bytes[index[index.size() - 1 - i]]) / magic;
        std::uint_fast32_t v = static_cast<std::uint_fast32_t>(std::abs(s) * 1e9);
        seed ^= v + (seed << (seed % 6)) + (seed >> (seed & 1)) + 0x9e3779b9;
    }
    return seed;
}

/* generator */
template<typename ByteT>
_hot _nodiscard inline std::mt19937 make_generator(const std::uint_fast32_t baseSeed, const std::optional<std::vector<ByteT>>& key)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte = ByteT;

    std::array<std::uint32_t, std::mt19937::state_size> seedData{};
    constexpr std::uint32_t prime = 0x9E3779B1u; // Prime used to 'shake' the bits
    constexpr std::uint32_t phi = 7; // Used to dephase the World dependencies & the Key

    // Seed
    for (std::size_t i = 0; i < seedData.size(); ++i)
        seedData[i] = (baseSeed + static_cast<std::uint32_t>(i)) * prime;

    // Key
    if (key.has_value()) _unlikely {
        for (Byte byte: *key) {
            for (std::size_t i = 0; i < seedData.size(); ++i) {
                seedData[i] ^= static_cast<std::uint32_t>(byte) * 2654435761u;
                seedData[i] = std::rotl(seedData[i], (i % 31) + 1) * prime;
            }
        }
    }

    // World dependencies - forward (0 -> 623)
    std::uint32_t carry = seedData[seedData.size() - 1];
    for (std::size_t i = 0; i < seedData.size(); ++i) {
        seedData[i] ^= std::rotl(carry, ((i * 13 + phi) % 31) + 1);
        carry = seedData[i];
    }

    // World dependencies - backward (623 -> 0)
    carry = seedData[0];
    for (std::size_t i = seedData.size(); i-- > 0;) {
        seedData[i] ^= std::rotl(carry, ((i * 17 + phi) % 31) + 1);
        carry = seedData[i];
    }

    // Create the generator (without seed_seq that reduce input possibility)
    utils::algorithms::sos::tools::DirectSeedSequence seq{seedData};
    return std::mt19937(seq);
}

} // namespace end
#endif /* HASH_H */
