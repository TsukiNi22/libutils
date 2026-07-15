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
    #include "../sosDefine.hpp" // utils::algorithms::sos::* (define)
    #include "../sosType.hpp"   // utils::algorithms::sos::* (type)
    #include <exception>        // std::invalid_argument
    #include <concepts>         // requires
    #include <cstring>          // std::memcpy
    #include <ranges>           // std::ranges::*

namespace utils::algorithms::utils::algorithms::sos::tools { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

template<typename ByteT = utils::algorithms::sos::Byte, std::ranges::input_range Range>
std::vector<ByteT> to_bytes(const Range& range)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte = ByteT;
    using Bytes = std::vector<Byte>;

    using T = std::ranges::range_value_t<Range>;
    static_assert(std::is_trivially_copyable_v<T>, "Element type must be trivially copyable.");

    std::vector<std::uint8_t> raw;
    raw.reserve(std::ranges::size(range) * sizeof(T));
    for (const T& value: range) {
        const auto* ptr = reinterpret_cast<const std::uint8_t*>(&value);
        raw.insert(raw.end(), ptr, ptr + sizeof(T));
    }

    std::size_t byte_count = (raw.size() + sizeof(Byte) - 1) / sizeof(Byte);
    raw.resize(byte_count * sizeof(Byte), 0);

    Bytes bytes(byte_count);
    std::memcpy(bytes.data(), raw.data(), raw.size());

    return bytes;
}

template<std::ranges::input_range Range, typename ByteT>
Range bytes_to(const std::vector<ByteT>& bytes)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte = ByteT;

    using T = std::ranges::range_value_t<Range>;
    static_assert(std::is_trivially_copyable_v<T>, "Element type must be trivially copyable.");

    std::size_t raw_size = bytes.size() * sizeof(Byte);
    if (raw_size % sizeof(T) != 0) unlikely {
        throw std::invalid_argument("Invalid byte count.");
    }

    Range range;
    if constexpr (requires {range.reserve(0);}) likely {
        range.reserve(raw_size / sizeof(T));
    }

    const std::uint8_t* raw = reinterpret_cast<const std::uint8_t*>(bytes.data());
    for (std::size_t i = 0; i < raw_size; i += sizeof(T)) {
        T value;
        std::memcpy(&value, raw + i, sizeof(T));
        if constexpr (requires {range.push_back(value);}) likely {
            range.push_back(value);
        } else {
            range.insert(range.end(), value);
        }
    }

    return range;
}

} // namespace end
#endif /* CONVERT_H */
