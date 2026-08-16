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
##  @file TCPSocket.cpp

File Description:
##  Definition of basic method for any socket
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/network/socket/TCPSocket.hpp"
#include "utils/network/NetworkDefine.hpp"
#include "utils/network/NetworkType.hpp"
#include "utils/verbose/Verbose.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <cstddef>
#include <vector>
#include <string>

_cold void utils::network::socket::TCPSocket::connect(const utils::network::Address& address)
{
    // Check if connection is allowed
    if (this->_fd != -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, "Can't connect the socket, already connected: " + std::to_string(this->_fd));
    }
    this->_mode = false;

    // Init the socket
    onAdvancedVerbose("Socket initialisation...");
    if ((this->_fd = ::socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, strerror(errno));

    // Setup the sockaddr_in
    onAdvancedVerbose("Setup of the address 'ip:port' used for the connection...");
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(address.port);
    if (::inet_pton(AF_INET, address.ip.first.c_str(), &addr.sin_addr) != 1)
        throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, "Invalid ip given, can't convert it, excepted ipv4: x.x.x.x");

    // Setup the timout
    struct timeval tv;
    tv.tv_sec = SOCKET_TIMEOUT;
    tv.tv_usec = 0;
    onAdvancedVerbose("Setup of the socket's parameters...");
    if (::setsockopt(this->_fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv)) < 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, strerror(errno));

    // Bind the socket with the parameter
    onBasicVerbose("Connection to '" << address.ip.first << ":" << address.port << "'...");
    if (::connect(this->_fd, (struct sockaddr *)&(addr), sizeof(addr)) < 0) {
        onBasicVerbose("Failed to connect client to '" << address.ip.first << ":" << address.port << "'");
        if (errno == EINPROGRESS || errno == EWOULDBLOCK)
            throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, std::string("Connection timed out (") + std::to_string(SOCKET_TIMEOUT) + "s)");
        else
            throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, strerror(errno));
    }

    onBasicVerbose("Client started!");
}

_cold void utils::network::socket::TCPSocket::listen(const utils::network::Address& address)
{
    // Check if connection is allowed
    if (this->_fd != -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, "Can't start the socket, already running: " + std::to_string(this->_fd));
    }
    this->_mode = true;

    // Init the socket
    onAdvancedVerbose("Socket initialisation...");
    if ((this->_fd = ::socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, strerror(errno));

    // Setup port to accept a reuse of the server before the tcp cooldown/TIME_WAIT
    int opt = 1;
    onAdvancedVerbose("Setup of the socket's parameters...");
    if (::setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, strerror(errno));

    // Setup the sockaddr_in
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(address.port);
    addr.sin_addr.s_addr = INADDR_ANY;
 
    // Bind the socket with the parameter
    onAdvancedVerbose("Binding of the socket...");
    if (::bind(this->_fd, (struct sockaddr *)&(addr), sizeof(addr)) < 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, strerror(errno));
    onBasicVerbose("Server started!");

    // Init the listen of the server
    if (::listen(this->_fd, SOMAXCONN) < 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, strerror(errno));
    onBasicVerbose("Listening on '0.0.0.0:" << address.port << "'...");
}
