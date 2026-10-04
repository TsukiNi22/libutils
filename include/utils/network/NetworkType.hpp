/**************************************************************\
Edition:
##  @date 14/08/2026 by @author Tsukini

File Name:
##  @file NetworkType.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef NETWORKTYPE_H
    #define NETWORKTYPE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "NetworkDefine.hpp"    // DEFAULT_IP, DEFAULT_PORT
    #include <cstdint>              // std::uint16_t
    #include <utility>              // std::pair
    #include <vector>               // std::vector
    #include <string>               // std::string

namespace utils::network { // namespace start
//----------------------------------------------------------------//
/* TYPE */

using Ip = std::pair<std::string, std::string>; // <ipv4 (hostname by default), hostname>
using Payload = std::string; // Not parsed (raw from the socket)
using Payloads = std::vector<utils::network::Payload>;

//----------------------------------------------------------------//
/* STRUCT */

struct Address {
    utils::network::Ip ip = {DEFAULT_IP, ""}; // Ignored on server side
    std::uint16_t port = DEFAULT_PORT;
};

} // namespace end
#endif /* NETWORKTYPE_H */
