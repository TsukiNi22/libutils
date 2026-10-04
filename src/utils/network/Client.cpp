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
#include "utils/exception/IException.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/basic/WarningException.hpp"
#include "utils/exception/basic/NoneException.hpp"
#include "utils/network/NetworkDefine.hpp"
#include "utils/network/Client.hpp"
#include "utils/verbose/Verbose.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>

// Close the epoll, deferred to the last join leaving when one is waiting on it
_cold void utils::network::Client::retire_(const int epfd)
{
    if (epfd == -1) return;

    // a join may still be in epoll_wait on it: closing it now would let the fd number be reused under it
    if (this->_waiting > 0) this->_retired.push_back(epfd);
    else ::close(epfd);
}

// Close the connection & the epoll
_cold void utils::network::Client::release_(void)
{
    this->_socket->close(); // also forget the buffers
    this->retire_(this->_epfd.exchange(-1));
}

_cold void utils::network::Client::start(void)
{
    std::lock_guard lock(this->_lock);

    // Check status
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
        int epfd = epoll_create1(0);
        if (epfd < 0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, std::strerror(errno));
        }
        this->_epfd = epfd;

        // Init the socket fd
        struct epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = fd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, std::strerror(errno));
        }

        this->_status = utils::network::Status::Up;
    } catch (const utils::exception::IException& e) {
        this->retire_(this->_epfd.exchange(-1));
        this->_status = utils::network::Status::Crashed;
        throw;
    }
}

_cold void utils::network::Client::stop(void)
{
    std::lock_guard lock(this->_lock);

    // Only one transition Up -> Down (no double close)
    utils::network::Status expected = utils::network::Status::Up;
    if (!this->_status.compare_exchange_strong(expected, utils::network::Status::Down)) return;

    onBasicVerbose("Stopping client...");
    this->release_();
}

_cold void utils::network::Client::kill(void)
{
    std::lock_guard lock(this->_lock);

    // Only one transition to Terminated (no double close)
    if (this->_status.exchange(utils::network::Status::Terminated) == utils::network::Status::Terminated) return;

    onBasicVerbose("Killing client...");
    this->release_();
}

// Handle an error on the socket: closed (Down) or error (Crashed)
_cold void utils::network::Client::fail_(const utils::exception::IException& e)
{
    if (e.isNone() && e.getCode() == utils::exception::InternalCode::SocketClosed) _likely {
        onBasicVerbose("Socket closed...");
        this->stop();
    } else _unlikely {
        onBasicVerbose("Error detected...");
        onDebugVerboseC(std::cerr, e.formated());
        this->stop();
        this->_status = utils::network::Status::Crashed;
    }
}

_hot _nodiscard const utils::network::Payloads& utils::network::Client::listen(void)
{
    std::lock_guard lock(this->_lock);

    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return this->_payloads;}

    // Clear the last events
    this->_payloads.clear();

    // Read the events
    struct epoll_event events[1];
    while (this->_status == utils::network::Status::Up) {
        int res = epoll_wait(this->_epfd, events, 1, 0);

        if (res < 0) _unlikely {
            if (errno == EINTR) return this->_payloads; // handle the ctrl-c before the first connection
            if (this->_status != utils::network::Status::Up) return this->_payloads; // stopped meanwhile
            this->stop();
            this->_status = utils::network::Status::Crashed;
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, std::strerror(errno));
        }

        // No event
        if (res == 0) break; // Nothing more to read on the socket

        // Data (EPOLLIN, can come with EPOLLHUP: read what is left)
        if (events[0].events & EPOLLIN) _likely {
            try {
                (void)this->_socket->receive(); // only once: the socket is readable (never wait for a full payload)
                std::vector<std::string> ret = this->_socket->recvAll(); // complete payloads of the buffer
                this->_payloads.insert(this->_payloads.end(), ret.begin(), ret.end());
            } catch (const utils::exception::IException& e) {
                this->fail_(e);
                return this->_payloads;
            }
            continue;
        }

        // Error or closed connection without data
        /*
         * EPOLLHUP  -> connection closed
         * EPOLLERR  -> error on the socket
        */
        if (events[0].events & (EPOLLERR | EPOLLHUP)) _unlikely {
            onBasicVerbose("Detected invalid epoll event, exiting...");
            if (events[0].events & EPOLLERR) _unlikely {
                this->stop();
                this->_status = utils::network::Status::Crashed;
                throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, "Error on the socket");
            }
            onBasicVerbose("Socket closed...");
            this->stop();
            return this->_payloads;
        }
        break;
    }

    // Ensure the correct reading when there is no event but data on the buffer
    if (this->_status == utils::network::Status::Up) {
        try {
            std::vector<std::string> ret = this->_socket->recvAll(); // complete payloads of the buffer
            this->_payloads.insert(this->_payloads.end(), ret.begin(), ret.end());
        } catch (const utils::exception::IException& e) {this->fail_(e);}
    }

    return this->_payloads;
}

_hot void utils::network::Client::join(void)
{
    // Read the events (without the lock during the wait: stop/kill can be called by another thread)
    struct epoll_event events[1];
    while (this->_status == utils::network::Status::Up) {
        int epfd = -1;
        {
            std::lock_guard lock(this->_lock);
            if (this->_status != utils::network::Status::Up) _unlikely {return;}
            epfd = this->_epfd;
            ++this->_waiting; // the epoll stays open until the wait is done
        }
        int res = epoll_wait(epfd, events, 1, 10); // wake up at least evry 10ms
        int error = errno;

        std::lock_guard lock(this->_lock);
        if (--this->_waiting == 0) {
            for (const int retired: this->_retired) ::close(retired);
            this->_retired.clear();
        }
        errno = error;
        if (res < 0) _unlikely {
            if (errno == EINTR) return; // handle the ctrl-c before the first connection
            if (this->_status != utils::network::Status::Up) return; // stopped meanwhile
            this->stop();
            this->_status = utils::network::Status::Crashed;
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, std::strerror(errno));
        }

        // Break the waiting when there is valid event (data, close or error: handled by listen)
        if (res > 0 && (events[0].events & (EPOLLIN | EPOLLERR | EPOLLHUP))) return;
    }
}

_hot void utils::network::Client::flush(void)
{
    std::lock_guard lock(this->_lock);

    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}

    try {this->_socket->flush();}
    catch (const utils::exception::IException& e) {
        this->fail_(e);
        throw;
    }
}

template<>
_hot void utils::network::Client::send<false>(const utils::network::Payload& payload)
{
    std::lock_guard lock(this->_lock);

    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}

    try {this->_socket->send(payload);}
    catch (const utils::exception::IException& e) {
        this->fail_(e);
        throw;
    }
}

template<>
_hot void utils::network::Client::send<true>(const utils::network::Payload& payload)
{
    std::lock_guard lock(this->_lock);

    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}

    try {this->_socket->sendBuffered(payload);}
    catch (const utils::exception::IException& e) {
        this->fail_(e);
        throw;
    }
}
