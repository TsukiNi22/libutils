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

#include "utils/attribute/Attribute.hpp"
#include "../sosDefine.hpp"         // utils::algorithms::sos::* (define)
#include "../sosType.hpp"           // utils::algorithms::sos::* (type)
#include "../tools/threshold.hpp"   // utils::algorithms::sos::tools::getThresholdIndex
#include "../tools/noise.hpp"       // utils::algorithms::sos::tools::noise
#include "../tools/hash.hpp"        // utils::algorithms::sos::tools::hash, utils::algorithms::sos::tools::make_generator
#include <stdexcept>                // std::* (exception)
#include <optional>                 // std::optional
#include <cstdint>                  // std::uint8_t, std::uint_fast32_t
#include <random>                   // std::mt19937
#include <vector>                   // std::vector

namespace utils::algorithms::utils::algorithms::sos::algorithm { // namespace start

#ifndef SOS_EXTRACT_OPTIMIZED
    #define SOS_EXTRACT_OPTIMIZED
template<std::uint8_t magic = MAGIC, typename ByteT>
nodiscard std::vector<ByteT> sos_extract_optimized(const std::vector<ByteT>& carrier, const std::optional<std::vector<ByteT>>& key = std::nullopt)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte  = ByteT;
    using Bytes = std::vector<Byte>;

    std::vector<std::uint_fast32_t> index;
    Bytes bytes;

    // Get the valid index within the accepted amplitude
    utils::algorithms::sos::tools::getThresholdIndex(index, carrier);

    // Check for the minimum space that is required (magic + size) + index used for the seed
    if (index.size() < (sizeof(Byte) + std::max(std::size_t{1}, sizeof(std::size_t) / sizeof(Byte))) * 8 + SEED_ELEMENT_COUNT) unlikely {
        throw std::out_of_range("Too few valide bytes that allow data storage, no hidden message");
    }

    // Generate a seed
    std::uint_fast32_t seed = utils::algorithms::sos::tools::hash(index, carrier);
    index.resize(index.size() - SEED_ELEMENT_COUNT);

    // Shuffle the index using the generated seed
    std::mt19937 gen = utils::algorithms::sos::tools::make_generator(seed, key);
    std::shuffle(index.begin(), index.end(), gen);

    // Reading byte methode
    std::size_t idx = 0;
    auto read_byte = [&](Byte& byte)
    {
        byte = 0;
        for (std::size_t b = 0; b < sizeof(Byte) * 8; ++b) {
            std::size_t pos = index[idx++];
            Byte bit = carrier[pos] & 1;
            byte |= (bit << b);
        }
    };

    // Check the header (magic)
    Byte identifier = 0;
    read_byte(identifier);
    if (identifier != magic) unlikely {
        throw std::invalid_argument("Invalid MAGIC byte, no hidden message");
    }

    // Check the header (size)
    std::size_t size = 0;
    for (std::size_t i = 0; i < std::max(std::size_t{1}, sizeof(std::size_t) / sizeof(Byte)); ++i) {
        Byte byte = 0;
        read_byte(byte);
        size |= (static_cast<std::size_t>(byte) << (sizeof(Byte) * 8 * i));
    }

    // Check if there is place for the index used for the seed & payload
    if (index.size() < sizeof(Byte) * 8 * size + SEED_ELEMENT_COUNT) unlikely {
        throw std::out_of_range("Invalid carrier, there is less valide bytes that allow data storage than excepted: " + std::to_string(sizeof(Byte) * 8 * size + SEED_ELEMENT_COUNT));
    }

    // Get the payload
    bytes.resize(size);
    for (std::size_t i = 0; i < size; ++i) read_byte(bytes[i]);

    return bytes;
}
#endif /* SOS_EXTRACT_OPTIMIZED */

} // namespace end
