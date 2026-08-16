/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 16/08/2026 by @author Tsukini

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
    #include "../security/observer/Observer.hpp"    // utils::security::observer::Observer
    #include "../attribute/Attribute.hpp"           // _cold, _hot , _nodiscard
    #include "NetworkDefine.hpp"                    // utils::network::Status
    #include "NetworkType.hpp"                      // utils::network::Address, utils::network::Payload, utils::network::Payloads
    #include "socket/Socket.hpp"                    // utils::network::socket::ISocket, utils::network::socket::TCPSocket
    #include <unordered_map>                        // std::unordered_map
    #include <memory>                               // std::shared_ptr, std::make_shared
    #include <atomic>                               // std::atomic
    #include <vector>                               // std::vector

namespace utils::network { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Server: private utils::security::observer::Observer<"Server"> {
    private:
        std::atomic<utils::network::Status> _status = utils::network::Status::Down;

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
        const std::unordered_map<int, utils::network::Payloads>& listen_(const int fd = -1);

    public:
        // ---------- Pre-Function -------- //
        void start(void); // start/restart the server

        /* thread safe */
        void stop(void); // stop the server (can be restarted, same has error)
        void kill(void); // terminate the server (can't be restarted)

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
        // Allways return the same reference and clean between each call
        _hot _nodiscard inline const std::unordered_map<int, utils::network::Payloads>& listen(void) {return this->listen_();};
        _hot _nodiscard inline const utils::network::Payloads& listen(const int fd) {return this->listen_(fd).at(fd);};

        /* getter */
        _nodiscard inline utils::network::Status getStatus(void) const {return this->_status;};

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
