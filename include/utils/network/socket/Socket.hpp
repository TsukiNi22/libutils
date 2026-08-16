/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 15/08/2026 by @author Tsukini

File Name:
##  @file Socket.hpp

File Description:
##  Include for all the different sockets
\**************************************************************/

#ifndef SOCKET_H
    #define SOCKET_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* interface */
    #include "ISocket.hpp"  // utils::network::socket::ISocket

    /* tools */
    #include "ASocket.hpp"  // utils::network::socket::isIp, utils::network::socket::resolveHostname, utils::network::socket::resolveAddress

    /* socket */
    #include "TCPSocket.hpp"    // utils::network::socket::TCPSocket

#endif /* SOCKET_H */
