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
##  @file extract_optimized.hpp

File Description:
##  Optimized extract version of the s.o.s algorithm
\**************************************************************/

#ifndef EXTRACTOPTIMIZED_H
    #define EXTRACTOPTIMIZED_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "utils/attribute/Attribute.hpp"
    #include "../tools/threshold.hpp"   // utils::algorithms::sos::tools::get_threshold_index
    #include "../tools/hash.hpp"        // utils::algorithms::sos::tools::hash, utils::algorithms::sos::tools::make_generator
    #include "../sosDefine.hpp"         // MAGIC, SEED_ELEMENT_COUNT
    #include <stdexcept>                // std::out_of_range, std::invalid_argument
    #include <algorithm>                // std::shuffle, std::max
    #include <concepts>                 // std::unsigned_integral
    #include <optional>                 // std::optional, std::nullopt
    #include <cstdint>                  // std::uint8_t, std::uint_fast32_t
    #include <cstddef>                  // std::size_t
    #include <random>                   // std::mt19937
    #include <vector>                   // std::vector
    #include <string>                   // std::to_string

namespace utils::algorithms::sos::algorithm { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* extract */
template<std::uint8_t magic = MAGIC, typename ByteT>
_hot _nodiscard inline std::vector<ByteT> sos_extract_optimized(const std::vector<ByteT>& carrier, const std::optional<std::vector<ByteT>>& key = std::nullopt)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte  = ByteT;
    using Bytes = std::vector<Byte>;

    std::vector<std::uint_fast32_t> index;
    Bytes bytes;

    // Get the valid index within the accepted amplitude
    utils::algorithms::sos::tools::get_threshold_index(index, carrier);

    // Check for the minimum space that is required (magic + size) + index used for the seed
    constexpr std::size_t sizeCount = std::max(std::size_t{1}, sizeof(std::size_t) / sizeof(Byte)); // number of Byte used to store the size
    if (index.size() < (1 + sizeCount) * sizeof(Byte) * 8 + SEED_ELEMENT_COUNT) _unlikely
        throw std::out_of_range("Too few valide bytes that allow data storage, no hidden message");

    // Generate a seed
    std::uint_fast32_t seed = utils::algorithms::sos::tools::hash(index, carrier);
    index.resize(index.size() - SEED_ELEMENT_COUNT);

    // Shuffle the index using the generated seed
    std::mt19937 gen = utils::algorithms::sos::tools::make_generator(seed, key);
    std::shuffle(index.begin(), index.end(), gen);

    // Reading byte method (one bit per LSB)
    std::size_t idx = 0;
    auto readByte = [&](Byte& byte) {
        byte = 0;
        for (std::size_t b = 0; b < sizeof(Byte) * 8; ++b) {
            std::size_t pos = index[idx++];
            Byte bit = static_cast<Byte>(carrier[pos] & 1);
            byte |= static_cast<Byte>(bit << b);
        }
    };

    // Check the header (magic)
    Byte identifier = 0;
    readByte(identifier);
    if (identifier != magic) _unlikely
        throw std::invalid_argument("Invalid MAGIC byte, no hidden message");

    // Check the header (size)
    std::size_t size = 0;
    for (std::size_t i = 0; i < sizeCount; ++i) {
        Byte byte = 0;
        readByte(byte);
        size |= (static_cast<std::size_t>(byte) << (sizeof(Byte) * 8 * i));
    }

    // Check if there is place for the payload (seed index already removed, header already read)
    if (size > (index.size() - idx) / (sizeof(Byte) * 8)) _unlikely
        throw std::out_of_range("Invalid carrier, there is less valide bytes that allow data storage than excepted: " + std::to_string(size));

    // Get the payload
    bytes.resize(size);
    for (std::size_t i = 0; i < size; ++i) readByte(bytes[i]);

    return bytes;
}

} // namespace end
#endif /* EXTRACTOPTIMIZED_H */
