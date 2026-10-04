/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 27/07/2026 by @author Tsukini

File Name:
##  @file noise.hpp

File Description:
##  Include of the noise generation tools
\**************************************************************/

#ifndef NOISE_H
    #define NOISE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "utils/attribute/Attribute.hpp"
    #include "../sosDefine.hpp" // UINTN_MIN, UINTN_MAX, RMS_LIMIT, NOISE_COEF
    #include <stdexcept>        // std::out_of_range
    #include <concepts>         // std::unsigned_integral
    #include <cstdint>          // std::uint_fast32_t
    #include <random>           // std::random_device, std::mt19937, std::normal_distribution
    #include <vector>           // std::vector
    #include <cmath>            // std::sqrt

namespace utils::algorithms::sos::tools { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* tools */
template<typename ByteT> // Clamp in the Byte range (double(UINTN_MAX) can be out of range, ex: uint64_t -> 2^64)
_hot _nodiscard inline ByteT clamp_to_byte(const double value)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte = ByteT;

    if (!(value > 0.0)) return UINTN_MIN(Byte); // NaN included
    if (value >= static_cast<double>(UINTN_MAX(Byte))) return UINTN_MAX(Byte);
    return static_cast<Byte>(value);
}

/* global */
template<typename ByteT>
_hot inline void noise(std::vector<ByteT>& bytes)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte = ByteT;

    // Check given values (avoid a NaN RMS)
    if (bytes.empty()) _unlikely
        throw std::out_of_range("No values to apply noise on");

    // Compute signal amplitudes RMS
    double rms = 0.0;
    for (Byte byte: bytes) rms += static_cast<double>(byte) * static_cast<double>(byte);
    rms = std::sqrt(rms / static_cast<double>(bytes.size()));

    // Check if the noise won't literally become the content
    if (rms < RMS_LIMIT(Byte)) _unlikely
        throw std::out_of_range("RMS is too small");

    // Noise generation setup
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::normal_distribution<double> values(0.0, rms * NOISE_COEF);

    // Apply random values
    for (Byte& byte: bytes)
        byte = utils::algorithms::sos::tools::clamp_to_byte<Byte>(static_cast<double>(byte) + values(gen));
}

/* local */
template<typename ByteT>
_hot inline void noise(std::vector<ByteT>& bytes, const std::vector<std::uint_fast32_t>& index)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte = ByteT;

    // Check given values (avoid a NaN RMS)
    if (index.empty()) _unlikely
        throw std::out_of_range("No values to apply noise on");

    // Compute signal amplitudes RMS
    double rms = 0.0;
    for (std::uint_fast32_t i: index) rms += static_cast<double>(bytes[i]) * static_cast<double>(bytes[i]);
    rms = std::sqrt(rms / static_cast<double>(index.size()));

    // Check if the noise won't literally become the content
    if (rms < RMS_LIMIT(Byte)) _unlikely
        throw std::out_of_range("RMS is too small");

    // Noise generation setup
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::normal_distribution<double> values(0.0, rms * NOISE_COEF);

    // Apply random values
    for (std::uint_fast32_t i: index)
        bytes[i] = utils::algorithms::sos::tools::clamp_to_byte<Byte>(static_cast<double>(bytes[i]) + values(gen));
}

} // namespace end
#endif /* NOISE_H */
