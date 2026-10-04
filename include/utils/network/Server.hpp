/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/08/2026 by @author Tsukini

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
    #include "../attribute/Attribute.hpp"           // _cold, _hot, _nodiscard, _noreturn
    #include "NetworkDefine.hpp"                    // utils::network::Status
    #include "NetworkType.hpp"                      // utils::network::Address, utils::network::Payload, utils::network::Payloads
    #include "socket/Socket.hpp"                    // utils::network::ISocket, utils::network::TCPSocket
    #include <unordered_map>                        // std::unordered_map
    #include <memory>                               // std::shared_ptr, std::make_shared
    #include <atomic>                               // std::atomic
    #include <vector>                               // std::vector
    #include <string>                               // std::string
    #include <unistd.h>                             // ::close
    #include <mutex>                                // std::recursive_mutex

namespace utils::network { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Server: private utils::security::observer::Observer<"Server"> {
    private:
        std::atomic<utils::network::Status> _status = utils::network::Status::Down;
        mutable std::recursive_mutex _lock; // the methods can be called by different threads (stop/kill during a listen/join)

        /* connection */
        std::shared_ptr<utils::network::ISocket> _socket = std::make_shared<utils::network::TCPSocket>();
        utils::network::Address _address;
        std::atomic<int> _epfd = -1;
        int _waiting = 0; // join calls waiting on the epoll (guarded by _lock)
        std::vector<int> _retired; // epoll closed by stop/kill during a join, closed by the last join leaving
        int _fd = -1; // Server fd

        /* buffer */
        std::unordered_map<int, utils::network::Payloads> _payloads;
        int _toClean = -1; // Store id of the payloads to clean, -1 == all, < -1 == none

        // ---------- Pre-Function -------- //
        void remove_(const int fd);
        void retire_(const int epfd); // close the epoll, or defer it while a join is waiting on it
        void release_(void); // close every connection & the epoll
        _noreturn void crash_(const std::string& info); // error on the server socket
        void receive_(const int fd, const bool read); // read (once) & extract the payloads of a client
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

        /* listen */
        // Allways return the same reference and clean between each call (empty if the client was removed)
        const utils::network::Payloads& listen(const int fd);

        /* getter */
        std::vector<int> getFds(void) const;

        // ------------ Function ---------- //
        // Allways return the same reference and clean between each call
        _hot _nodiscard inline const std::unordered_map<int, utils::network::Payloads>& listen(void) {return this->listen_();};

        /* getter */
        _cold _nodiscard inline utils::network::Status getStatus(void) const {return this->_status;};

        // ------------ Operator ---------- //
        Server& operator=(const Server& other) = delete;
        Server& operator=(Server&& other) = delete;

        // ---------- Constructor --------- //
        Server() = default;
        Server(const std::shared_ptr<utils::network::ISocket>& socket, const utils::network::Address& address = {}): _socket{socket}, _address{address} {};
        Server(const Server& other) = delete;
        Server(Server&& other) = delete;

        // ----------- Destructor --------- //
        ~Server() {this->kill(); for (const int epfd: this->_retired) ::close(epfd);};
};

} // namespace end
#endif /* SERVER_H */
