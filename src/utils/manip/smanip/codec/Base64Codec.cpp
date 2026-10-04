/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 03/08/2026 by @author Tsukini

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
#include <cstddef>
#include <cstdint>
#include <climits>
#include <string>

_hot _nodiscard std::string utils::smanip::codec::Base64Codec::encode(std::string s) const
{
    // EVP_EncodeBlock takes the size as int
    if (s.size() > static_cast<std::size_t>(INT_MAX / 4 * 3)) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Base 64 encoding of " + std::to_string(s.size()) + " bytes");
    }
    std::size_t len = ((s.size() + 2) / 3) * 4;
    std::string encoded(len, '\0');

    // Encode string (base 64)
    int size = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(encoded.data()), reinterpret_cast<const unsigned char*>(s.data()), static_cast<int>(s.size()));
    if (size < 0) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Codec, "Fail to encode given string in base 64");
    }

    // Remove useless char
    encoded.resize(static_cast<std::size_t>(size));

    return encoded;
}

_hot _nodiscard std::string utils::smanip::codec::Base64Codec::decode(std::string s) const
{
    // EVP_DecodeBlock takes the size as int
    if (s.size() > static_cast<std::size_t>(INT_MAX)) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Base 64 decoding of " + std::to_string(s.size()) + " bytes");
    }
    std::size_t padding = 0, len = (s.size() * 3) / 4;
    std::string decoded(len, '\0');

    // Decode string (base 64)
    int size = EVP_DecodeBlock(reinterpret_cast<unsigned char*>(decoded.data()), reinterpret_cast<const unsigned char*>(s.data()), static_cast<int>(s.size()));
    if (size < 0) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Codec, "Fail to decode given string using base 64");
    }

    // Padding at the end "==" (after the trailing whitespace, ignored by EVP_DecodeBlock)
    std::size_t end = s.find_last_not_of(" \t\n\r\f\v");
    end = (end == std::string::npos) ? 0 : end + 1;
    padding += (end > 0 && s[end - 1] == '=');
    padding += (end > 1 && s[end - 2] == '=');

    // Remove useless char
    decoded.resize(static_cast<std::size_t>(size) - padding);

    return decoded;
}
