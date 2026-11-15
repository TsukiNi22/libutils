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
##  @file CApi.hpp

File Description:
##  String tools of the C API (copy in the fixed buffers of the status & in the buffers of the user) & handles of the C API of smanip
\**************************************************************/

// xstyle: ignore-file CPP-CLASS-NAME (C handle: C types are snake_case)

#ifndef SMANIP_CAPI_H
    #define SMANIP_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _cold, _hot, _nodiscard
    #include "../../exception/CApi.hpp"         // utils::exception::c_api::not_null, utils_handle
    #include "parser/EETPParser.hpp"            // utils::smanip::parser::EETPParser, utils::smanip::parser::EETPContent
    #include "codec/ICodec.hpp"                 // utils::smanip::codec::ICodec
    #include "CApi.h"                           // utils_string_t, utils_codec_t, utils_eetp_content_t, utils_eetp_parser_t
    #include <algorithm>                        // std::min
    #include <cstddef>                          // std::size_t
    #include <cstring>                          // std::memcpy
    #include <utility>                          // std::move
    #include <memory>                           // std::unique_ptr
    #include <cstdio>                           // std::snprintf
    #include <string>                           // std::string

namespace utils::smanip::c_api { // namespace start
//----------------------------------------------------------------//
/* TOOLS */

// Copy in a fixed size char array (strings of utils_exception, ...), truncated with '...' on the last 3 char, always '\0' terminated
template<std::size_t N>
_cold void copy(char (&dst)[N], const char* src) noexcept
{
    static_assert(N > 3, "The buffer must be able to hold at least the '...' of a truncation");

    // Truncate to the buffer size
    const int len = std::snprintf(dst, N, "%s", (src ? src : ""));

    // Mark the truncation with '...' on the last 3 char
    if (len >= static_cast<int>(N)) {
        dst[N - 4] = '.';
        dst[N - 3] = '.';
        dst[N - 2] = '.';
    }
}

// Copy in a buffer of the user (C API _r functions), like snprintf (truncated, '\0' terminated), returns the full length
_cold inline std::size_t copy_r(const std::string& str, char* buf, const std::size_t size)
{
    if (size > 0) {
        const std::size_t len = std::min(str.size(), size - 1);
        std::memcpy(utils::exception::c_api::not_null(buf, "buffer"), str.data(), len);
        buf[len] = '\0';
    }
    return str.size();
}

// Bytes given by the C side (NULL only with a size of 0)
_hot _nodiscard inline std::string bytes(const void* data, const std::size_t size, const char* name = "data")
{
    if (size == 0) return "";
    return std::string(static_cast<const char*>(utils::exception::c_api::not_null(data, name)), size);
}

} // namespace end
//----------------------------------------------------------------//
/* STRUCT */

/* handles of the C API, usable in c++ to get the object */
struct utils_string: public utils_handle {
    std::string str;

    // ---------- Constructor --------- //
    utils_string(std::string s = ""): str{std::move(s)} { /* Nothing */ };
};

struct utils_codec: public utils_handle {
    std::unique_ptr<utils::smanip::codec::ICodec> codec;

    // ---------- Constructor --------- //
    utils_codec(std::unique_ptr<utils::smanip::codec::ICodec> c): codec{std::move(c)} { /* Nothing */ };
};

struct utils_eetp_content: public utils_handle {
    utils::smanip::parser::EETPContent content;

    // ---------- Constructor --------- //
    utils_eetp_content(utils::smanip::parser::EETPContent c): content{std::move(c)} { /* Nothing */ };
};

struct utils_eetp_parser: public utils_handle {
    utils::smanip::parser::EETPParser parser;
};

#endif /* SMANIP_CAPI_H */
