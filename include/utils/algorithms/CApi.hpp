/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 16/11/2026 by @author Tsukini

File Name:
##  @file CApi.hpp

File Description:
##  Macros of the C API of the algorithms (definitions of the fixed types, expanded in the .cpp)
\**************************************************************/

#ifndef ALGORITHMS_CAPI_H
    #define ALGORITHMS_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "CApi.h" // utils_sos_embed_*, utils_sos_extract_*

    //----------------------------------------------------------------//
    /* MACRO */

    /* s.o.s of an unsigned integer (sos_embed / sos_extract of the .cpp) */
    #define C_API_SOS_DEF(name, ctype) \
        _nothrow int utils_sos_embed_##name(ctype* carrier, size_t count, const ctype* payload, size_t payload_count, const ctype* key, size_t key_count, int options) \
        {return sos_embed(carrier, count, payload, payload_count, key, key_count, options);} \
        _nothrow size_t utils_sos_extract_##name(const ctype* carrier, size_t count, const ctype* key, size_t key_count, ctype* out, size_t max) \
        {return sos_extract(carrier, count, key, key_count, out, max);}

#endif /* ALGORITHMS_CAPI_H */
