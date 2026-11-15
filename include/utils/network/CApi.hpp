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
##  Handles of the C API of utils::network (usable in c++ to get the object)
\**************************************************************/

// xstyle: ignore-file CPP-CLASS-NAME (C handle: C types are snake_case)

#ifndef NETWORK_CAPI_H
    #define NETWORK_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../exception/CApi.hpp"    // utils_handle
    #include "socket/ISocket.hpp"       // utils::network::ISocket
    #include "NetworkType.hpp"          // utils::network::Payloads, utils::network::Address
    #include "Client.hpp"               // utils::network::Client
    #include "Server.hpp"               // utils::network::Server
    #include "CApi.h"                   // utils_socket_t, utils_client_t, utils_server_t, utils_payloads_t, utils_network_messages_t
    #include <utility>                  // std::pair
    #include <memory>                   // std::shared_ptr
    #include <vector>                   // std::vector

//----------------------------------------------------------------//
/* STRUCT */

/* handles of the C API */
struct utils_socket: public utils_handle {
    std::shared_ptr<utils::network::ISocket> socket;

    // ---------- Constructor --------- //
    utils_socket(std::shared_ptr<utils::network::ISocket> s): socket{std::move(s)} { /* Nothing */ };
};

struct utils_client: public utils_handle, public utils::network::Client {
    // ---------- Constructor --------- //
    utils_client(const std::shared_ptr<utils::network::ISocket>& socket, const utils::network::Address& address): utils::network::Client(socket, address) { /* Nothing */ };
};

struct utils_server: public utils_handle, public utils::network::Server {
    // ---------- Constructor --------- //
    utils_server(const std::shared_ptr<utils::network::ISocket>& socket, const utils::network::Address& address): utils::network::Server(socket, address) { /* Nothing */ };
};

struct utils_payloads: public utils_handle {
    utils::network::Payloads payloads;
};

struct utils_network_messages: public utils_handle {
    std::vector<std::pair<int, utils::network::Payloads>> messages;
};

#endif /* NETWORK_CAPI_H */
