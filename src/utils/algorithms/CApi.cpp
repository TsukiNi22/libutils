/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file CApi.cpp

File Description:
##  C API of the algorithms (c2dmp-hsm, s.o.s)
\**************************************************************/

#include "utils/algorithms/c2dmp-hsm/c2dmp-hsm.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/algorithms/sos/sos.hpp"
#include "utils/algorithms/CApi.hpp"
#include "utils/exception/CApi.hpp"
#include "utils/algorithms/CApi.h"
#include <string_view>
#include <algorithm>
#include <optional>
#include <cstddef>
#include <cstdint>
#include <vector>

//----------------------------------------------------------------//
/* CHECK */

static_assert(C_API_SOS_MAGIC == MAGIC);
static_assert(C_API_SOS_OPTION_NOISE == static_cast<int>(utils::algorithms::sos::Option::Noise));
static_assert(C_API_SOS_OPTION_GLOBAL_NOISE == static_cast<int>(utils::algorithms::sos::Option::GlobalNoise));

//----------------------------------------------------------------//
/* TOOLS */

/* c2dmp: the template parameters chosen at runtime */
template<std::uint_fast8_t depth, typename UIntT>
_hot _nodiscard static float c2dmp_full(const std::string_view a, const std::string_view b, const bool full)
{
    if (full) return utils::algorithms::c2dmp::c2dmp<depth, UIntT, true>(a, b);
    return utils::algorithms::c2dmp::c2dmp<depth, UIntT, false>(a, b);
}

template<std::uint_fast8_t depth>
_hot _nodiscard static float c2dmp_counter(const std::string_view a, const std::string_view b, const std::size_t counter, const bool full)
{
    switch (counter) {
        case 1: return c2dmp_full<depth, std::uint8_t>(a, b, full);
        case 2: return c2dmp_full<depth, std::uint16_t>(a, b, full);
        case 4: return c2dmp_full<depth, std::uint32_t>(a, b, full);
        case 8: return c2dmp_full<depth, std::uint64_t>(a, b, full);
        default: throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The counter size must be 1, 2, 4 or 8 bytes");
    }
}

// String of the C side (NULL only with a size of 0)
_hot _nodiscard static std::string_view view(const char* str, const std::size_t size, const char* name)
{
    if (size == 0) return {};
    return std::string_view(utils::exception::c_api::not_null(str, name), size);
}

// Values of the C side (NULL only with a count of 0)
template<typename T>
_hot _nodiscard static std::vector<T> values(const T* data, const std::size_t count, const char* name)
{
    if (count == 0) return {};
    (void)utils::exception::c_api::not_null(data, name);
    return std::vector<T>(data, data + count);
}

/* s.o.s: the options chosen at runtime (the redirections of sos.hpp) */
template<utils::algorithms::sos::Option options, typename T>
_hot static void embed_with(std::vector<T>& carrier, const std::vector<T>& payload, const std::optional<std::vector<T>>& key)
{
    if (key) utils::algorithms::sos::sos_embed<options>(carrier, payload, *key);
    else utils::algorithms::sos::sos_embed<options>(carrier, payload);
}

template<typename T>
_hot static void embed(std::vector<T>& carrier, const std::vector<T>& payload, const std::optional<std::vector<T>>& key, const int options)
{
    constexpr utils::algorithms::sos::Option both = static_cast<utils::algorithms::sos::Option>(utils::algorithms::sos::Option::Noise | utils::algorithms::sos::Option::GlobalNoise);

    switch (options) {
        case C_API_SOS_OPTION_NONE:                                         embed_with<utils::algorithms::sos::Option::None>(carrier, payload, key);        return;
        case C_API_SOS_OPTION_NOISE:                                        embed_with<utils::algorithms::sos::Option::Noise>(carrier, payload, key);       return;
        case C_API_SOS_OPTION_GLOBAL_NOISE:                                 embed_with<utils::algorithms::sos::Option::GlobalNoise>(carrier, payload, key); return;
        case C_API_SOS_OPTION_NOISE | C_API_SOS_OPTION_GLOBAL_NOISE:        embed_with<both>(carrier, payload, key);                                       return;
        default: throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The options of s.o.s are invalid");
    }
}

template<typename T>
_hot _nodiscard static int sos_embed(T* carrier, const std::size_t count, const T* payload, const std::size_t payloadCount, const T* key, const std::size_t keyCount, const int options)
{
    return utils::exception::c_api::guard([&](void) {
        std::vector<T> bytes = values(carrier, count, "carrier");
        const std::optional<std::vector<T>> secret = (keyCount > 0 ? std::optional<std::vector<T>>(values(key, keyCount, "key")) : std::nullopt);

        embed(bytes, values(payload, payloadCount, "payload"), secret, options);
        if (bytes.size() != count) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "The carrier changed of size");
        }
        std::copy(bytes.begin(), bytes.end(), carrier);
    });
}

template<typename T>
_hot _nodiscard static std::size_t sos_extract(const T* carrier, const std::size_t count, const T* key, const std::size_t keyCount, T* out, const std::size_t max)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {
        const std::optional<std::vector<T>> secret = (keyCount > 0 ? std::optional<std::vector<T>>(values(key, keyCount, "key")) : std::nullopt);
        const std::vector<T> payload = (secret ? utils::algorithms::sos::sos_extract(values(carrier, count, "carrier"), *secret) : utils::algorithms::sos::sos_extract(values(carrier, count, "carrier")));

        if (max > 0) std::copy_n(payload.begin(), std::min(max, payload.size()), utils::exception::c_api::not_null(out, "output"));
        return payload.size();
    });
}

//----------------------------------------------------------------//
/* C2DMP */

_hot float utils_c2dmp(const char* a, size_t a_size, const char* b, size_t b_size, uint8_t prefix_depth, size_t counter_size, bool full)
{
    return utils::exception::c_api::guard_fallback(-1.0f, [&](void) -> float {
        const std::string_view first = view(a, a_size, "a");
        const std::string_view second = view(b, b_size, "b");

        switch (prefix_depth) {
            case 1: return c2dmp_counter<1>(first, second, counter_size, full);
            case 2: return c2dmp_counter<2>(first, second, counter_size, full);
            case 3: return c2dmp_counter<3>(first, second, counter_size, full);
            case 4: return c2dmp_counter<4>(first, second, counter_size, full);
            case 5: return c2dmp_counter<5>(first, second, counter_size, full);
            default: throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The prefix depth must be from 1 to 5");
        }
    });
}

_hot float utils_c2dmp_default(const char* a, const char* b)
{
    return utils::exception::c_api::guard_fallback(-1.0f, [&](void) -> float {
        const char* first = utils::exception::c_api::not_null(a, "a");
        const char* second = utils::exception::c_api::not_null(b, "b");

        return utils::algorithms::c2dmp::c2dmp(first, second);
    });
}

//----------------------------------------------------------------//
/* S.O.S */

C_API_SOS_DEF(u8, uint8_t)
C_API_SOS_DEF(u16, uint16_t)
C_API_SOS_DEF(u32, uint32_t)
C_API_SOS_DEF(u64, uint64_t)
