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
##  @file sosDefine.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef SOSDEFINE_H
    #define SOSDEFINE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <limits>       // std::numeric_limits<T>, UINT16_MAX

    //----------------------------------------------------------------//
    /* DEFINE */

    /* const */
    #define MAGIC 0x22
    #define NOISE_COEF 0.0025 // 0.001 < Studio < 0.005 < Mic < 0.02 (noise coef for amplitude)
    #define SEED_ELEMENT_COUNT 256 // number of index used for the seed creation
    #define RANGE_PERCENTAGE 0.05 // percentage used for the range usage limits

    /* limits */
    #define PAYLOAD_PERCENTAGE_LIMIT 0.075 // The payload can only be x percent of the total signal at max

    //----------------------------------------------------------------//
    /* MACRO */

    /* values */
    #define UINTN_MIN(ByteT) std::numeric_limits<ByteT>::min()
    #define UINTN_MAX(ByteT) std::numeric_limits<ByteT>::max()

    /* limits */
    #define RMS_LIMIT(ByteT) 250.0 * (static_cast<double>(UINTN_MAX(ByteT)) / static_cast<double>(UINT16_MAX)) // 100 ~ 5000 normal (scaled on a base of uint16_t)
    #define THRESHOLD_MIN(ByteT) ((1ull << (sizeof(ByteT) * 8 / 2)) - 1ull)
    #define THRESHOLD_MAX(ByteT) (UINTN_MAX(ByteT) - THRESHOLD_MIN(ByteT))
    #define RANGE_USED_MIN(ByteT) std::min(2048.0, UINTN_MAX(ByteT) * RANGE_PERCENTAGE) // Need at least x percentage of the whole range to ensure some security
    #define RANGE_USED_MAX(ByteT) (UINTN_MAX(ByteT) * (1.0 - RANGE_PERCENTAGE)) // Need less than x percentage of the whole range to ensure some security

namespace utils::algorithms::sos { // namespace start
//----------------------------------------------------------------//
/* ENUM */

enum Option {
    None        = 0,
    Noise       = 1 << 0,
    GlobalNoise = 1 << 1,
};

} // namespace end
#endif /* SOSDEFINE_H */
