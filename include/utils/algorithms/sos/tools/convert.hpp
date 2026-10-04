/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/07/2026 by @author Tsukini

File Name:
##  @file convert.hpp

File Description:
##  Include of the convertion tools
\**************************************************************/

#ifndef CONVERT_H
    #define CONVERT_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "utils/attribute/Attribute.hpp"
    #include "../sosType.hpp"   // utils::algorithms::sos::Byte
    #include <type_traits>      // std::is_trivially_copyable_v
    #include <stdexcept>        // std::invalid_argument
    #include <concepts>         // requires, std::unsigned_integral
    #include <cstring>          // std::memcpy
    #include <cstdint>          // std::uint8_t
    #include <cstddef>          // std::size_t
    #include <ranges>           // std::ranges::*
    #include <vector>           // std::vector

namespace utils::algorithms::sos::tools { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* convertion */
template<typename ByteT = utils::algorithms::sos::Byte, std::ranges::input_range Range>
_hot _nodiscard inline std::vector<ByteT> to_bytes(const Range& range)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte  = ByteT;
    using Bytes = std::vector<Byte>;

    using T = std::ranges::range_value_t<Range>;
    static_assert(std::is_trivially_copyable_v<T>, "Element type must be trivially copyable.");

    // Copy the raw memory of every element
    std::vector<std::uint8_t> raw;
    if constexpr (std::ranges::sized_range<Range>)
        raw.reserve(std::ranges::size(range) * sizeof(T));
    for (const T& value: range) {
        const std::uint8_t* ptr = reinterpret_cast<const std::uint8_t*>(&value);
        raw.insert(raw.end(), ptr, ptr + sizeof(T));
    }

    // Pad the raw memory to a whole number of Byte
    std::size_t byteCount = (raw.size() + sizeof(Byte) - 1) / sizeof(Byte);
    raw.resize(byteCount * sizeof(Byte), 0);

    Bytes bytes(byteCount);
    if (!raw.empty()) _likely // memcpy on a null pointer is UB, even for 0 byte
        std::memcpy(bytes.data(), raw.data(), raw.size());

    return bytes;
}

template<std::ranges::input_range Range, typename ByteT>
_hot _nodiscard inline Range bytes_to(const std::vector<ByteT>& bytes)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte = ByteT;

    using T = std::ranges::range_value_t<Range>;
    static_assert(std::is_trivially_copyable_v<T>, "Element type must be trivially copyable.");

    // Check if the raw memory can be split in whole elements
    std::size_t rawSize = bytes.size() * sizeof(Byte);
    if (rawSize % sizeof(T) != 0) _unlikely
        throw std::invalid_argument("Invalid byte count.");

    Range range;
    if constexpr (requires {range.reserve(0);})
        range.reserve(rawSize / sizeof(T));

    // Rebuild every element from the raw memory
    const std::uint8_t* raw = reinterpret_cast<const std::uint8_t*>(bytes.data());
    for (std::size_t i = 0; i < rawSize; i += sizeof(T)) {
        T value;
        std::memcpy(&value, raw + i, sizeof(T));
        if constexpr (requires {range.push_back(value);}) range.push_back(value);
        else range.insert(range.end(), value);
    }

    return range;
}

} // namespace end
#endif /* CONVERT_H */
