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
##  @file noise.hpp

File Description:
##  Include of the noise generation tools
\**************************************************************/

#ifndef NOISE_H
    #define NOISE_H

#include "utils/attribute/Attribute.hpp"
#include "../sosDefine.hpp" // utils::algorithms::sos::* (define)
#include "../sosType.hpp"   // utils::algorithms::sos::* (type)
#include <stdexcept>        // std::* (exception)
#include <algorithm>        // std::clamp
#include <optional>         // std::optional
#include <cstdint>          // std::uint_fast32_t
#include <random>           // std::random_device, std::mt19937, std::normal_distribution
#include <vector>           // std::vector

namespace utils::algorithms::sos::tools { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* global */
template<typename ByteT>
void noise(std::vector<ByteT>& bytes)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    using Byte  = ByteT;
    using Bytes = std::vector<Byte>;

    // Compute signal amplitudes RMS
    double rms = 0.0;
    for (Byte byte: bytes) rms += static_cast<double>(byte) * static_cast<double>(byte);
    rms = std::sqrt(rms / static_cast<double>(bytes.size()));

    // Check if the noise won't litteraly become the content
    if (rms < RMS_LIMIT(Byte))
        throw std::out_of_range("RMS is too small");

    // Noise generation setup
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::normal_distribution<double> values(0.0, rms * NOISE_COEF);

    // Apply random values
    for (Byte& byte: bytes)
        byte = std::clamp(byte + values(gen), 0.0, static_cast<double>(UINTN_MAX(Byte)));
}

/* local */
template<typename ByteT>
void noise(std::vector<ByteT>& bytes, const std::vector<std::uint_fast32_t>& index)
{
    // Check given type
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");

    // Compute signal amplitudes RMS
    double rms = 0.0;
    for (std::uint_fast32_t i: index) rms += static_cast<double>(bytes[i]) * static_cast<double>(bytes[i]);
    rms = std::sqrt(rms / static_cast<double>(index.size()));

    // Check if the noise won't litteraly become the content
    if (rms < RMS_LIMIT(Byte))
        throw std::out_of_range("RMS is too small");

    // Noise generation setup
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::normal_distribution<double> values(0.0, rms * NOISE_COEF);

    // Apply random values
    for (std::uint_fast32_t i: index)
        bytes[i] = std::clamp(bytes[i] + values(gen), 0.0, static_cast<double>(UINTN_MAX(Byte)));
}

} // namespace end
#endif /* NOISE_H */
