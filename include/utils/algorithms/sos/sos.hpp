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
##  @file sos.hpp

File Description:
##  Header for include all the different algorithm
\**************************************************************/

#ifndef SOS_H
    #define SOS_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* algorithm */
    #include "utils/attribute/Attribute.hpp"
    #include "algorithm/extract_optimized.hpp"  // utils::algorithms::sos::algorithm::sos_extract_optimized
    #include "algorithm/embed_optimized.hpp"    // utils::algorithms::sos::algorithm::sos_embed_optimized

    /* tools */
    #include "tools/convert.hpp"                // utils::algorithms::sos::tools::to_bytes, utils::algorithms::sos::tools::bytes_to

    /* type */
    #include "sosDefine.hpp"                    // utils::algorithms::sos::Option, MAGIC
    #include "sosType.hpp"                      // utils::algorithms::sos::Byte, utils::algorithms::sos::Bytes, utils::algorithms::sos::Key
    #include <concepts>                         // std::unsigned_integral
    #include <optional>                         // std::make_optional
    #include <cstdint>                          // std::uint8_t
    #include <vector>                           // std::vector

namespace utils::algorithms::sos { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* embed (redirection, auto handle of the optional build of the key) */
template<utils::algorithms::sos::Option options = utils::algorithms::sos::Option::None, std::uint8_t magic = MAGIC, typename ByteT>
_hot inline void sos_embed(std::vector<ByteT>& carrier, const std::vector<ByteT>& payload)
{
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    utils::algorithms::sos::algorithm::sos_embed_optimized<options, magic>(carrier, payload);
}

template<utils::algorithms::sos::Option options = utils::algorithms::sos::Option::None, std::uint8_t magic = MAGIC, typename ByteT>
_hot inline void sos_embed(std::vector<ByteT>& carrier, const std::vector<ByteT>& payload, const std::vector<ByteT>& key)
{
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    utils::algorithms::sos::algorithm::sos_embed_optimized<options, magic>(carrier, payload, std::make_optional(key));
}

/* extract (redirection, auto handle of the optional build of the key) */
template<std::uint8_t magic = MAGIC, typename ByteT>
_hot _nodiscard inline std::vector<ByteT> sos_extract(const std::vector<ByteT>& carrier)
{
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    return utils::algorithms::sos::algorithm::sos_extract_optimized<magic>(carrier);
}

template<std::uint8_t magic = MAGIC, typename ByteT>
_hot _nodiscard inline std::vector<ByteT> sos_extract(const std::vector<ByteT>& carrier, const std::vector<ByteT>& key)
{
    static_assert(std::unsigned_integral<ByteT>, "ByteT must be an unsigned integer type");
    return utils::algorithms::sos::algorithm::sos_extract_optimized<magic>(carrier, std::make_optional(key));
}

} // namespace end
#endif /* SOS_H */
