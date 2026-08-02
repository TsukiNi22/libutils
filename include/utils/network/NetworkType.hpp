/**************************************************************\
Edition:
##  @date 01/08/2026 by @author Tsukini

File Name:
##  @file Type.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef TYPE_H
    #define TYPE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "NetworkDefine.hpp"    // DEFAULT_IP, DEFAULT_PORT, zappy::Action, zappy::Direction
    #include <cstdint>              // std::uint16_t
    #include <string>               // std::string

namespace utils::network { // namespace start
//----------------------------------------------------------------//
/* TYPE */

using Ip = std::pair<std::string, std::string>; // <ipv4 (hostname by default), hostname>

struct Address {
    utils::network::Ip ip = {DEFAULT_IP, ""}; // Ignored on server side
    std::uint16_t port = DEFAULT_PORT;
};

} // namespace end
#endif /* TYPE_H */
