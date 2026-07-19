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
##  @file embed_optimized.hpp

File Description:
##  Optimized embed version of the s.o.s algorithm
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "../sosDefine.hpp"         // utils::algorithms::sos::* (define)
#include "../sosType.hpp"           // utils::algorithms::sos::* (type)
#include "../tools/threshold.hpp"   // utils::algorithms::sos::tools::getThresholdIndex, utils::algorithms::sos::tools::removeThreshold
#include "../tools/noise.hpp"       // utils::algorithms::sos::tools::noise
#include "../tools/hash.hpp"        // utils::algorithms::sos::tools::hash, utils::algorithms::sos::tools::make_generator
#include <stdexcept>                // std::* (exception)
#include <algorithm>                // std::shuffle
#include <optional>                 // std::optional
#include <cstdint>                  // std::uint8_t, std::uint_fast32_t
#include <random>                   // std::mt19937
#include <vector>                   // std::vector

namespace utils::algorithms::sos::algorithm { // namespace start

#ifndef SOS_EMBED_OPTIMIZED
    #define SOS_EMBED_OPTIMIZED
template<utils::algorithms::sos::Option options = utils::algorithms::sos::Option::None, std::uint8_t magic = MAGIC, typename ByteT>
void sos_embed_optimized(std::vector<ByteT>& carrier, const std::vector<ByteT>& payload, const std::optional<std::vector<ByteT>>& key = std::nullopt)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte  = ByteT;
    using Bytes = std::vector<Byte>;

    std::vector<std::uint_fast32_t> index;
    Bytes bytes;

    // Setup message (header data + payload)
    std::size_t size = payload.size();
    bytes.reserve(1 + std::max(std::size_t{1}, sizeof(size) / sizeof(Byte)) + payload.size());
    bytes.push_back(magic);
    for (std::size_t i = 0; i < std::max(std::size_t{1}, sizeof(size) / sizeof(Byte)); ++i) bytes.push_back((size >> (sizeof(Byte) * 8 * i)) & UINTN_MAX(Byte));
    bytes.insert(bytes.end(), payload.begin(), payload.end());

    // On noise generation (global)
    if constexpr (options & utils::algorithms::sos::Option::GlobalNoise) {
        // Get the valid index within the accepted amplitude
        utils::algorithms::sos::tools::getThresholdIndex(index, carrier);

        // Generate noise on already valid values
        utils::algorithms::sos::tools::noise(carrier, index);
    }

    // On noise generation (local)
    else if constexpr (options & utils::algorithms::sos::Option::Noise) {
        // Generate noise on all values
        utils::algorithms::sos::tools::noise(carrier);
    }

    // Remove the values on the brink of the accepted amplitude
    utils::algorithms::sos::tools::removeThreshold(carrier);

    // Get the valid index within the accepted amplitude
    utils::algorithms::sos::tools::getThresholdIndex(index, carrier);

    // Check if the payload can be hiden in the carrier
    if (index.size() == 0) _unlikely {
        throw std::out_of_range("Too few valide bytes that allow data storage, none where found!");
    }
    double percentage = static_cast<double>(sizeof(Byte) * 8 * bytes.size()) / static_cast<double>(index.size());
    if (percentage > PAYLOAD_PERCENTAGE_LIMIT * 100.0) _unlikely {
        throw std::out_of_range("Too few valide bytes that allow data storage, the payload percentage limit was reach: " + std::to_string(percentage / 100.0) + "%");
    }

    // Check if there is place for the element used for the seed
    if (index.size() < sizeof(Byte) * 8 * bytes.size() + SEED_ELEMENT_COUNT) _unlikely {
        throw std::out_of_range("Too few valide bytes that allow data storage, the limit was reach: " + std::to_string(sizeof(Byte) * 8 * bytes.size() + SEED_ELEMENT_COUNT));
    }

    // Generate a seed
    std::uint_fast32_t seed = utils::algorithms::sos::tools::hash(index, carrier);
    index.resize(index.size() - SEED_ELEMENT_COUNT);

    // Shuffle the index using the generated seed
    std::mt19937 gen = utils::algorithms::sos::tools::make_generator(seed, key);
    std::shuffle(index.begin(), index.end(), gen);

    // Store the payload
    std::size_t idx = 0;
    for (std::size_t i = 0; i < bytes.size(); ++i)
    for (std::size_t j = 0; j < sizeof(Byte) * 8; ++j) {
        Byte bit = (bytes[i] >> j) & 1;
        std::size_t pos = index[idx++];
        carrier[pos] = (carrier[pos] & ~1) | bit;
    }
}
#endif /* SOS_EMBED_OPTIMIZED */

} // namespace end
