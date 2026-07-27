/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 25/07/2026 by @author Tsukini

File Name:
##  @file Base64Codec.cpp

File Description:
##  Declaration of the base 64 codec methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/manip/smanip/codec/Base64Codec.hpp"
#include <openssl/evp.h>
#include <cstdint>
#include <string>

_nodiscard std::string utils::smanip::codec::Base64Codec::encode(std::string s) const
{
    int len = ((s.size() + 2) / 3) * 4;
    std::string encoded(len, '\0');

    // Decode string (base 64)
    int size = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(encoded.data()), reinterpret_cast<const unsigned char*>(s.data()), s.size());
    if (size < 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Codec, "Fail to encode given string in base 64");

    // Remove uless char
    encoded.resize(size);

    return encoded;
}

_nodiscard std::string utils::smanip::codec::Base64Codec::decode(std::string s) const
{
    std::size_t padding = 0, len = (s.size() * 3) / 4;
    std::string decoded(len, '\0');

    // Decode string (base 64)
    int size = EVP_DecodeBlock(reinterpret_cast<unsigned char*>(decoded.data()), reinterpret_cast<const unsigned char*>(s.data()), s.size());
    if (size < 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Codec, "Fail to decode given string using base 64");

    // Padding at the end "=="
    padding += (!s.empty() && s.back() == '=');
    padding += (s.size() > 1 && s[s.size() - 2] == '=');

    // Remove uless char
    decoded.resize(size - padding);

    return decoded;
}
