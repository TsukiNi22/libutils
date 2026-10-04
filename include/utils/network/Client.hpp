/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 29/08/2026 by @author Tsukini

File Name:
##  @file Client.hpp

File Description:
##  Definition of the client class for custom network
\**************************************************************/

#ifndef CLIENT_H
    #define CLIENT_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../security/observer/Observer.hpp"    // utils::security::observer::Observer
    #include "../attribute/Attribute.hpp"           // _cold, _hot, _nodiscard
    #include "NetworkDefine.hpp"                    // utils::network::Status
    #include "NetworkType.hpp"                      // utils::network::Address, utils::network::Payload, utils::network::Payloads
    #include "../exception/IException.hpp"          // utils::exception::IException
    #include "socket/Socket.hpp"                    // utils::network::ISocket, utils::network::TCPSocket, utils::network::resolve_address
    #include <cstddef>                              // std::size_t
    #include <memory>                               // std::shared_ptr, std::make_shared
    #include <atomic>                               // std::atomic
    #include <unistd.h>                             // ::close
    #include <vector>                               // std::vector
    #include <mutex>                                // std::recursive_mutex

namespace utils::network { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Client: private utils::security::observer::Observer<"Client"> {
    private:
        std::atomic<utils::network::Status> _status = utils::network::Status::Down;
        mutable std::recursive_mutex _lock; // the methods can be called by different threads (stop/kill during a listen/join)

        /* connection */
        std::shared_ptr<utils::network::ISocket> _socket = std::make_shared<utils::network::TCPSocket>();
        utils::network::Address _address;
        std::atomic<int> _epfd = -1;
        int _waiting = 0; // join calls waiting on the epoll (guarded by _lock)
        std::vector<int> _retired; // epoll closed by stop/kill during a join, closed by the last join leaving

        /* buffer */
        utils::network::Payloads _payloads;

        // ---------- Pre-Function -------- //
        void retire_(const int epfd); // close the epoll, or defer it while a join is waiting on it
        void release_(void); // close the connection & the epoll
        void fail_(const utils::exception::IException& e); // socket closed (Down) or error (Crashed)

    public:
        // ---------- Pre-Function -------- //
        void start(void); // start/restart the client

        /* thread safe */
        void stop(void); // stop the client (can be restarted, same has error)
        void kill(void); // terminate the client (can't be restarted)

        // Allways return the same reference and clean between each call
        const utils::network::Payloads& listen(void);
        void join(void); // Await until the next listen event

        void flush(void); // send all the stack
        template<bool buffered = false>
        void send(const utils::network::Payload& payload);
        // <false> -> by default send directly
        // <true>  -> store the payload in a stack and send them when a send<false> is call or flush

        // ------------ Function ---------- //
        _cold _nodiscard inline utils::network::Status getStatus(void) const {return this->_status;};

        // ------------ Operator ---------- //
        Client& operator=(const Client& other) = delete;
        Client& operator=(Client&& other) = delete;

        // ---------- Constructor --------- //
        Client() = default;
        Client(const std::shared_ptr<utils::network::ISocket>& socket, const utils::network::Address& address = {}): _socket{socket}, _address{address} {utils::network::resolve_address(this->_address);};
        Client(const Client& other) = delete;
        Client(Client&& other) = delete;

        // ----------- Destructor --------- //
        ~Client() {this->kill(); for (const int epfd: this->_retired) ::close(epfd);};
};

} // namespace end
#endif /* CLIENT_H */
