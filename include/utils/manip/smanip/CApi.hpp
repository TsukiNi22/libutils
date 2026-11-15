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
##  String tools of the C API (copy in the fixed buffers of the status & in the buffers of the user)
\**************************************************************/

#ifndef SMANIP_CAPI_H
    #define SMANIP_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _cold
    #include "../../exception/CApi.hpp"         // utils::exception::c_api::not_null
    #include <algorithm>                        // std::min
    #include <cstddef>                          // std::size_t
    #include <cstring>                          // std::memcpy
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

} // namespace end
#endif /* SMANIP_CAPI_H */
