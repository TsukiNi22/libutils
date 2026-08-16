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
##  @file Client.cpp

File Description:
##  Different method of the client class
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/basic/WarningException.hpp"
#include "utils/network/NetworkDefine.hpp"
#include "utils/network/Client.hpp"
#include "utils/verbose/Verbose.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <thread>

_cold void utils::network::Client::start(void)
{
    if (this->_status == utils::network::Status::Up) {
        throw utils::exception::WarningException(utils::exception::InternalCode::AlreadyRunning);
    } else if (this->_status == utils::network::Status::Terminated) {
        throw utils::exception::WarningException(utils::exception::InternalCode::Killed);
    }
    onBasicVerbose("Starting client...");

    // Reset buffers
    this->_payloads.clear();

    // Start the socket
    try {
        // Open the socket (doesn't restart open already open)
        if (this->_socket->getFd() == -1) _likely {
            this->_socket->connect(this->_address);
        }

        // Setup the epoll
        const int fd = this->_socket->getFd();
        this->_epfd = epoll_create1(0);
        if (this->_epfd < 0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, strerror(errno));
        }

        // Init the socket fd
        struct epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = fd;
        if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, fd, &ev) < 0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, strerror(errno));
        }

        this->_status = utils::network::Status::Up;
    } catch (...) {
        this->_status = utils::network::Status::Crashed;
        throw;
    }
}

_cold void utils::network::Client::stop(void)
{
    if (this->_status != utils::network::Status::Up) return;
    onBasicVerbose("Stopping client...");
    this->_socket->close();
    ::close(this->_epfd);
    this->_epfd = -1;
    this->_status = utils::network::Status::Down;
}

_cold void utils::network::Client::kill(void)
{
    if (this->_status == utils::network::Status::Terminated) return;
    onBasicVerbose("Killing client...");
    this->_socket->close();
    if (this->_epfd != -1) ::close(this->_epfd);
    this->_epfd = -1;
    this->_status = utils::network::Status::Terminated;
}

_hot _nodiscard const utils::network::Payloads& utils::network::Client::listen(void)
{
    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return this->_payloads;}

    // Clear the last events
    this->_payloads.clear();

    // Read the events
    struct epoll_event events[1];
    while (this->_status == utils::network::Status::Up) {
        std::this_thread::yield(); // To not fully take the cpu computing
        int res = epoll_wait(this->_epfd, events, 1, 0);

        if (res < 0) _unlikely {
            if (errno == EINTR) return this->_payloads; // handle the ctrl-c before the first connection
            this->kill();
            this->_status = utils::network::Status::Crashed;
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, strerror(errno));
        }

        // No event
        if (res == 0) break; // Nothing more to read on the socket

        // Error or Invalid poll handling
        /*
         * EPOLLHUP  -> connection closed
         * EPOLLERR  -> error on the socket
        */
        if (events[0].events & (EPOLLERR | EPOLLHUP)) _unlikely {
            onBasicVerbose("Detected invalid epoll event, exiting...");
            this->kill();
            this->_status = utils::network::Status::Crashed;
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll);
        }

        // Nothing to read
        if (!(events[0].events & EPOLLIN)) break;

        // Handle event
        try {
            this->_payloads.emplace_back(this->_socket->recv()); // Read the event
            std::vector<std::string> ret = this->_socket->recvAll(); // Get all the remaning payload on the buffer
            this->_payloads.insert(this->_payloads.end(), ret.begin(), ret.end());
        } catch (const utils::exception::IException& e) {
            if (e.isNone() && e.getCode() == utils::exception::InternalCode::SocketClosed) {
                onBasicVerbose("Socket closed...");
                this->_status = utils::network::Status::Down;
            } else {
                onBasicVerbose("Error during the client request handling...");
                onBasicVerbose(e.formated());
                this->kill();
                this->_status = utils::network::Status::Crashed;
            }
            ::close(this->_epfd);
            this->_epfd = -1;
            return this->_payloads;
        }
    }

    // Ensure the correct reading when there is no event bu data on the buffer
    std::vector<std::string> ret = this->_socket->recvAll(); // Get all the remaning payload on the buffer
    this->_payloads.insert(this->_payloads.end(), ret.begin(), ret.end());

    return this->_payloads;
}

_hot void utils::network::Client::join(void)
{
    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}

    // Read the events
    struct epoll_event events[1];
    while (this->_status == utils::network::Status::Up) {
        std::this_thread::yield(); // To not fully take the cpu computing
        int res = epoll_wait(this->_epfd, events, 1, 10); // wake up at least evry 10ms

        if (res < 0) _unlikely {
            if (errno == EINTR) return; // handle the ctrl-c before the first connection
            this->kill();
            this->_status = utils::network::Status::Crashed;
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, strerror(errno));
        }

        // No event
        if (res == 0) continue;

        // Error or Invalid poll handling
        /*
         * EPOLLHUP  -> connection closed
         * EPOLLERR  -> error on the socket
        */
        if (events[0].events & (EPOLLERR | EPOLLHUP)) _unlikely {
            onBasicVerbose("Detected invalid epoll event, exiting...");
            this->kill();
            this->_status = utils::network::Status::Crashed;
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll);
        }

        // Nothing to read
        if (!(events[0].events & EPOLLIN)) continue;

        // Break the waiting when there is valid event
        return;
    }
}

_hot inline void utils::network::Client::flush(void)
{
    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}

    try {this->_socket->flush();}
    catch (...) {
        this->kill();
        this->_status = utils::network::Status::Crashed;
        throw;
    }
}

template<>
_hot inline void utils::network::Client::send<false>(const utils::network::Payload& payload)
{ 
    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}

    try {this->_socket->send(payload);}
    catch (...) {
        this->kill();
        this->_status = utils::network::Status::Crashed;
        throw;
    }
}

template<>
_hot inline void utils::network::Client::send<true>(const utils::network::Payload& payload)
{
    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}

    try {this->_socket->sendBuffered(payload);}
    catch (...) {
        this->kill();
        this->_status = utils::network::Status::Crashed;
        throw;
    }
}
