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
##  @file threshold.hpp

File Description:
##  Include of the threshold tools
\**************************************************************/

#ifndef THRESHOLD_H
    #define THRESHOLD_H

#include "utils/attribute/Attribute.hpp"
#include "../sosDefine.hpp" // utils::algorithms::sos::* (define)
#include "../sosType.hpp"   // utils::algorithms::sos::* (type)
#include <unordered_set>    // std::unordered_set
#include <stdexcept>        // std::* (exception)
#include <algorithm>        // std::count
#include <optional>         // std::optional
#include <cstdint>          // std::uint_fast32_t
#include <vector>           // std::vector
#include <string>           // std::to_string

namespace utils::algorithms::sos::tools { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

template<typename ByteT>
void getThresholdIndex(std::vector<std::uint_fast32_t>& index, const std::vector<ByteT>& bytes)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte = ByteT;

    index.clear();
    index.reserve(bytes.size());
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        if (bytes[i] >= THRESHOLD_MIN(Byte) && bytes[i] <= THRESHOLD_MAX(Byte)) likely {index.push_back(i);}
    }
}

template<typename ByteT>
void removeThreshold(std::vector<ByteT>& bytes)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte  = ByteT;

    // Check the limits
    std::unordered_set<Byte> seen;
    for (Byte byte: bytes) seen.insert(byte);
    std::size_t rangeUsed = seen.size();

    if (rangeUsed < RANGE_USED_MIN(Byte)) unlikely {
        throw std::out_of_range("Too few range used can't edit thresholds, the limit was reach: " + std::to_string(rangeUsed));
    } else if (rangeUsed > RANGE_USED_MAX(Byte)) unlikely {
        throw std::out_of_range("Too many range used can't edit thresholds, the limit was reach: " + std::to_string(rangeUsed));
    }

    // Remove those on threshold
    for (Byte& byte: bytes) {
        if (byte == THRESHOLD_MIN(Byte))      byte -= (byte == 0) ? -1 : 1;
        else if (byte == THRESHOLD_MAX(Byte)) byte += (byte == UINTN_MAX(Byte)) ? -1 : 1;
    }
}

} // namespace end
#endif /* THRESHOLD_H */
