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
##  @file Server.hpp

File Description:
##  Definition of the server class for custom network
\**************************************************************/

#ifndef SERVER_H
    #define SERVER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _cold, _hot 
    #include "NetworkDefine.hpp"            // utils::network::Status
    #include "NetworkType.hpp"              // utils::network::Address, utils::network::Payload, utils::network::Payloads
    #include "socket/Socket.hpp"            // utils::network::socket::ISocket, utils::network::socket::TCPSocket
    #include <unordered_map>                // std::unordered_map
    #include <memory>                       // std::shared_ptr, std::make_shared
    #include <vector>                       // std::vector

namespace utils::network { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Server {
    private:
        utils::network::Status _status = utils::network::Status::Down;

        /* connection */
        std::shared_ptr<utils::network::socket::ISocket> _socket = std::make_shared<utils::network::socket::TCPSocket>();
        utils::network::Address _address;
        int _epfd = -1;
        int _fd = -1; // Server fd

        /* buffer */
        std::unordered_map<int, utils::network::Payloads> _payloads;
        int _to_clean = -1; // Store id of the payloads to clean, -1 == all, < -1 == none

        // ---------- Pre-Function -------- //
        void remove(const int fd);

    public:
        // ---------- Pre-Function -------- //
        template<bool initsafe = false> // doesn't init when the socket is already open
        void start(void); // start/restart the server (buffer not reset)
        void stop(void); // stop the server (can be restarted, same has error)
        void kill(void); // terminate the server (can't be restarted)

        // Allways return the same reference and clean between each call
        const std::unordered_map<int, utils::network::Payloads>& listen(void);
        const utils::network::Payloads& listen(const int fd);
        void join(const int fd = -1); // Await until the next listen event on this precise fd or every one (-1)

        void flush(void); // send all the stack
        void flush(const int fd); // send all the stack of the specified client
        template<bool buffered = false>
        void send(const int fd, const utils::network::Payload& payload);
        // <false> -> by default send directly
        // <true>  -> store the payload in a stack and send them when a send<false> is call or flush

        /* getter */
        std::vector<int> getFds(void) const;

        // ------------ Function ---------- //
        utils::network::Status getStatus(void) const {return this->_status;};

        // ------------ Operator ---------- //
        Server& operator=(const Server& other) = delete;
        Server& operator=(Server&& other) = delete;

        // ---------- Constructor --------- //
        Server() = default;
        Server(const std::shared_ptr<utils::network::socket::ISocket>& socket, const utils::network::Address& address = {}): _socket{socket}, _address{address} {};
        Server(const Server& other) = delete;
        Server(Server&& other) = delete;

        // ----------- Destructor --------- //
        ~Server() {this->kill();};
};

} // namespace end
#endif /* SERVER_H */
