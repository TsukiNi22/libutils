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
    #include "../sosDefine.hpp" // utils::algorithms::sos::* (define)
    #include "../sosType.hpp"   // utils::algorithms::sos::* (type)
    #include <stdexcept>        // std::* (exception)
    #include <optional>         // std::optional
    #include <cstdint>          // std::uint_fast32_t
    #include <random>           // std::mt19937, std::seed_seq
    #include <vector>           // std::vector
    #include <array>            // std::array
    #include <cmath>            // std::sin, std::abs
    #include <bit>              // std::rotl

namespace utils::algorithms::utils::algorithms::sos::tools { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

template<typename ByteT>
nodiscard std::uint_fast32_t hash(const std::vector<std::uint_fast32_t>& index, const std::vector<ByteT>& bytes)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");

    constexpr double magic = -1.460354508809587;
    std::uint_fast32_t seed = 0x811c9dc5;
    for (std::size_t i = 0; i < SEED_ELEMENT_COUNT; ++i) {
        double s = std::sin(bytes[index[index.size() - 1 - i]]) / magic;
        std::uint_fast32_t v = static_cast<std::uint_fast32_t>(std::abs(s) * 1e9);
        seed ^= v + (seed << (seed % 6)) + (seed >> (seed & 1)) + 0x9e3779b9;
    }
    return seed;
}

/* temporary object used to passe a sequence without modification unlike seed_seq */
struct DirectSeedSequence {
    using result_type = std::uint32_t;
    const std::array<std::uint32_t, std::mt19937::state_size>& data;
    nodiscard std::size_t size(void) const noexcept {return data.size();}
    template<typename It>
    void generate(It first, It last) const {std::copy_n(data.begin(), std::distance(first, last), first);}
};

template<typename ByteT>
nodiscard std::mt19937 make_generator(const std::uint_fast32_t base_seed, const std::optional<std::vector<ByteT>>& key)
{
    std::array<std::uint32_t, std::mt19937::state_size> seed_data{};
    constexpr std::uint32_t prime = 0x9E3779B1u; // Prime used to 'shake' the bits
    constexpr std::uint32_t phi = 7; // Used to dephase the World dependencies & the Key

    // Seed
    for (std::size_t i = 0; i < seed_data.size(); ++i)
        seed_data[i] = (base_seed + static_cast<std::uint32_t>(i)) * prime;

    // Key
    if (key.has_value()) unlikely {
        for (utils::algorithms::sos::Byte byte: *key) {
            for (std::size_t i = 0; i < seed_data.size(); ++i) {
                seed_data[i] ^= static_cast<std::uint32_t>(byte) * 2654435761u;
                seed_data[i] = std::rotl(seed_data[i], (i % 31) + 1) * prime;
            }
        }
    }

    // World dependencies - forward (0 -> 623)
    std::uint32_t carry = seed_data[seed_data.size() - 1];
    for (std::size_t i = 0; i < seed_data.size(); ++i) {
        seed_data[i] ^= std::rotl(carry, ((i * 13 + phi) % 31) + 1);
        carry = seed_data[i];
    }

    // World dependencies - backward (623 -> 0)
    carry = seed_data[0];
    for (std::size_t i = seed_data.size(); i-- > 0; ) {
        seed_data[i] ^= std::rotl(carry, ((i * 17 + phi) % 31) + 1);
        carry = seed_data[i];
    }

    // Create the generator (without seed_seq that reduce input possibility)
    utils::algorithms::sos::tools::DirectSeedSequence seq{seed_data};
    return std::mt19937(seq);
}

} // namespace end
#endif /* HASH_H */
