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
##  @file Server.cpp

File Description:
##  Different method of the server class
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/IException.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/basic/WarningException.hpp"
#include "utils/exception/basic/NoneException.hpp"
#include "utils/network/NetworkDefine.hpp"
#include "utils/network/Server.hpp"
#include "utils/verbose/Verbose.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <thread>
#include <format>
#include <vector>
#include <string>
#include <mutex>

// Close the epoll, deferred to the last join leaving when one is waiting on it
_cold void utils::network::Server::retire_(const int epfd)
{
    if (epfd == -1) return;

    // a join may still be in epoll_wait on it: closing it now would let the fd number be reused under it
    if (this->_waiting > 0) this->_retired.push_back(epfd);
    else ::close(epfd);
}

// Close every connection & the epoll
_cold void utils::network::Server::release_(void)
{
    for (const auto &[fd, _]: this->_payloads) {
        ::close(fd);
        this->_socket->discard(fd);
    }
    this->_payloads.clear();
    this->_toClean = -1;
    this->_socket->close();
    this->retire_(this->_epfd.exchange(-1));
}

_cold void utils::network::Server::start(void)
{
    std::lock_guard lock(this->_lock);

    // Check status
    if (this->_status == utils::network::Status::Up) {
        throw utils::exception::WarningException(utils::exception::InternalCode::AlreadyRunning);
    } else if (this->_status == utils::network::Status::Terminated) {
        throw utils::exception::WarningException(utils::exception::InternalCode::Killed);
    }
    onBasicVerbose("Starting server...");

    // Reset buffers
    this->_payloads.clear();
    this->_toClean = -1;

    // Start the socket
    try {
        // Open the socket (doesn't restart open already open)
        if (this->_socket->getFd() == -1) _likely {
            this->_socket->listen(this->_address);
        }

        // Setup the epoll
        this->_fd = this->_socket->getFd();
        int epfd = epoll_create1(0);
        if (epfd < 0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, std::strerror(errno));
        }
        this->_epfd = epfd;

        // Init the socket fd (server)
        struct epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = this->_fd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, this->_fd, &ev) < 0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, std::strerror(errno));
        }

        this->_status = utils::network::Status::Up;
    } catch (const utils::exception::IException& e) {
        this->retire_(this->_epfd.exchange(-1));
        this->_status = utils::network::Status::Crashed;
        throw;
    }
}

_cold void utils::network::Server::stop(void)
{
    std::lock_guard lock(this->_lock);

    // Only one transition Up -> Down (no double close)
    utils::network::Status expected = utils::network::Status::Up;
    if (!this->_status.compare_exchange_strong(expected, utils::network::Status::Down)) return;

    onBasicVerbose("Stopping server...");
    this->release_();
}

_cold void utils::network::Server::kill(void)
{
    std::lock_guard lock(this->_lock);

    // Only one transition to Terminated (no double close)
    if (this->_status.exchange(utils::network::Status::Terminated) == utils::network::Status::Terminated) return;

    onBasicVerbose("Killing server...");
    this->release_();
}

// Error on the listening socket: the server can't continue
_cold _noreturn void utils::network::Server::crash_(const std::string& info)
{
    onBasicVerbose("Detected invalid epoll events on the server socket, exiting...");
    this->stop();
    this->_status = utils::network::Status::Crashed;
    throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, info);
}

// Read the available data of a client & extract its complete payloads (never wait), remove it on close/error
_hot void utils::network::Server::receive_(const int fd, const bool read)
{
    utils::network::Payloads& payloads = this->_payloads[fd];
    try {
        if (read) (void)this->_socket->receive(fd); // only once: the fd is readable
        std::vector<std::string> ret = this->_socket->recvAll(fd); // complete payloads of the buffer
        payloads.insert(payloads.end(), ret.begin(), ret.end());
    } catch (const utils::exception::IException& e) {
        if (e.isNone() && e.getCode() == utils::exception::InternalCode::SocketClosed) {
            onBasicVerbose("Socket closed, remove client...");
        } else {
            onBasicVerbose("Error during the client request handling, remove client...");
            onDebugVerboseC(std::cerr, e.formated());
        }
        this->remove_(fd);
    }
}

_hot _nodiscard const std::unordered_map<int, utils::network::Payloads>& utils::network::Server::listen_(const int fd)
{
    std::lock_guard lock(this->_lock);

    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return this->_payloads;}
    else if (fd != -1 && !this->_payloads.contains(fd)) _unlikely {
        throw utils::exception::WarningException(utils::exception::InternalCode::UnknownFd, std::to_string(fd));
    }

    // Clear the last events getted
    if (this->_toClean == -1) for (auto &[_, payloads]: this->_payloads) payloads.clear();
    else if (this->_toClean >= 0 && this->_payloads.contains(this->_toClean)) this->_payloads[this->_toClean].clear();
    this->_toClean = fd;

    // Read the events (level triggered: an fd stay returned while it has unread data)
    std::vector<struct epoll_event> events;
    while (this->_status == utils::network::Status::Up) {
        events.resize(this->_payloads.size() + 1);
        int res = epoll_wait(this->_epfd, events.data(), static_cast<int>(events.size()), 0);

        if (res < 0) _unlikely {
            if (errno == EINTR) break; // handle the ctrl-c before the first connection
            if (this->_status != utils::network::Status::Up) return this->_payloads; // stopped meanwhile
            this->crash_(std::strerror(errno));
        }

        // No event
        if (res == 0) break; // Nothing more to read on the socket
        onDebugVerbose("Event(s) detected on poll: " << std::to_string(res));

        bool consumed = false;
        for (std::size_t i = 0; i < static_cast<std::size_t>(res); ++i) {
            int actualFd = events[i].data.fd;

            // Server socket: error or new connection
            if (actualFd == this->_fd) _unlikely {
                if (events[i].events & (EPOLLERR | EPOLLHUP)) _unlikely {this->crash_("Error on the server socket");}
                if (!(events[i].events & EPOLLIN)) continue;
                struct epoll_event ev{};
                ev.events = EPOLLIN;
                ev.data.fd = this->_socket->accept();
                if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, ev.data.fd, &ev) < 0) _unlikely {
                    ::close(ev.data.fd);
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, std::strerror(errno));
                }
                this->_payloads[ev.data.fd]; // Ensure the existance of the new connection in the hashtable
                consumed = true;
                continue;
            }

            // Ignore those not concerned & the removed ones
            if (!this->_payloads.contains(actualFd)) continue;
            if (fd != -1 && actualFd != fd) continue;
            consumed = true;

            // Data (EPOLLIN, can come with EPOLLHUP: read what is left), or only an error/close
            if (events[i].events & EPOLLIN) _likely {
                this->receive_(actualFd, true);
            } else if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                onBasicVerbose("Detected invalid epoll events, remove client...");
                this->remove_(actualFd);
            }
        }

        // Only events of other fds (listen(fd)): they will be read by their own call
        if (!consumed) break;
    }

    // Ensure the correct reading when there is no event but data on the buffer
    if (fd == -1) {
        for (int actualFd: this->getFds()) this->receive_(actualFd, false); // copy: receive can remove
    } else if (this->_payloads.contains(fd)) { // can have been removed (closed)
        this->receive_(fd, false);
    }

    return this->_payloads;
}

_hot _nodiscard const utils::network::Payloads& utils::network::Server::listen(const int fd)
{
    const std::unordered_map<int, utils::network::Payloads>& all = this->listen_(fd);
    std::lock_guard lock(this->_lock);
    auto it = all.find(fd);
    if (it == all.end()) _unlikely { // the client was removed during the listen (closed)
        static const utils::network::Payloads empty;
        return empty;
    }
    return it->second;
}

_hot void utils::network::Server::join(const int fd)
{
    {
        std::lock_guard lock(this->_lock);

        // Check status
        if (this->_status != utils::network::Status::Up) _unlikely {return;}
        else if (fd != -1 && !this->_payloads.contains(fd)) _unlikely {
            throw utils::exception::WarningException(utils::exception::InternalCode::UnknownFd, std::to_string(fd));
        }
    }

    // Read the events (without the lock during the wait: stop/kill can be called by another thread)
    std::vector<struct epoll_event> events;
    while (this->_status == utils::network::Status::Up) {
        int epfd = -1;
        {
            std::lock_guard lock(this->_lock);
            if (this->_status != utils::network::Status::Up) _unlikely {return;}
            epfd = this->_epfd;
            ++this->_waiting; // the epoll stays open until the wait is done
            events.resize(this->_payloads.size() + 1);
        }
        int res = epoll_wait(epfd, events.data(), static_cast<int>(events.size()), 10); // wake up at least evry 10ms
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
            this->crash_(std::strerror(errno));
        }

        // No event
        if (res == 0) continue;

        for (std::size_t i = 0; i < static_cast<std::size_t>(res); ++i) {
            int actualFd = events[i].data.fd;

            // Server socket: error, or a new connection is an event when waiting for any fd
            if (actualFd == this->_fd) _unlikely {
                if (events[i].events & (EPOLLERR | EPOLLHUP)) _unlikely {this->crash_("Error on the server socket");}
                if (fd == -1 && (events[i].events & EPOLLIN)) return;
                continue;
            }

            // Ignore those not concerned
            if (fd != -1 && _likely_c(fd != actualFd)) continue;

            // Break the waiting when there is valid event (data, close or error: handled by listen)
            if (events[i].events & (EPOLLIN | EPOLLERR | EPOLLHUP)) return;
        }
    }
}

_hot void utils::network::Server::flush(void)
{
    std::lock_guard lock(this->_lock);

    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}

    for (int fd: this->getFds()) this->flush(fd); // copy: flush can remove a client
}

_hot void utils::network::Server::flush(const int fd)
{
    std::lock_guard lock(this->_lock);

    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}
    else if (!this->_payloads.contains(fd)) _unlikely {
        throw utils::exception::WarningException(utils::exception::InternalCode::UnknownFd, std::to_string(fd));
    }

    try {this->_socket->flush(fd);}
    catch (const utils::exception::IException& e) {
        if (e.isNone() && e.getCode() == utils::exception::InternalCode::SocketClosed) _likely {
            onBasicVerbose("Socket closed, remove client...");
        } else _unlikely {
            onBasicVerbose("Error detected, remove client...");
            onDebugVerboseC(std::cerr, e.formated());
        }
        this->remove_(fd);
    }
}

template<>
_hot void utils::network::Server::send<false>(const int fd, const utils::network::Payload& payload)
{
    std::lock_guard lock(this->_lock);

    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}
    else if (!this->_payloads.contains(fd)) _unlikely {
        throw utils::exception::WarningException(utils::exception::InternalCode::UnknownFd, std::to_string(fd));
    }

    try {this->_socket->send(payload, fd);}
    catch (const utils::exception::IException& e) {
        if (e.isNone() && e.getCode() == utils::exception::InternalCode::SocketClosed) _likely {
            onBasicVerbose("Socket closed, remove client...");
        } else _unlikely {
            onBasicVerbose("Error detected, remove client...");
            onDebugVerboseC(std::cerr, e.formated());
        }
        this->remove_(fd);
    }
}

template<>
_hot void utils::network::Server::send<true>(const int fd, const utils::network::Payload& payload)
{
    std::lock_guard lock(this->_lock);

    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}
    else if (!this->_payloads.contains(fd)) _unlikely {
        throw utils::exception::WarningException(utils::exception::InternalCode::UnknownFd, std::to_string(fd));
    }

    try {this->_socket->sendBuffered(payload, fd);}
    catch (const utils::exception::IException& e) {
        if (e.isNone() && e.getCode() == utils::exception::InternalCode::SocketClosed) _likely {
            onBasicVerbose("Socket closed, remove client...");
        } else _unlikely {
            onBasicVerbose("Error detected, remove client...");
            onDebugVerboseC(std::cerr, e.formated());
        }
        this->remove_(fd);
    }
}

_hot void utils::network::Server::remove_(const int fd)
{
    std::lock_guard lock(this->_lock);

    if (!this->_payloads.contains(fd)) _unlikely {
        throw utils::exception::WarningException(utils::exception::InternalCode::UnknownFd, std::to_string(fd));
    }

    // Verbose
    struct sockaddr_in addr{};
    socklen_t addrLen = sizeof(addr);
    if (getpeername(fd, reinterpret_cast<struct sockaddr*>(&addr), &addrLen) < 0) _unlikely {
        onBasicVerbose(std::format("Remove client 'fd={}' (unknown address: {})", fd, std::strerror(errno)));
    } else {
        onBasicVerbose(std::format("Remove client '{}:{}'", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port)));
    }

    // Remove the fd from epoll (ENOENT/EBADF: already removed)
    if (epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, nullptr) < 0 && errno != ENOENT && errno != EBADF) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, std::strerror(errno));
    }

    // Close it and remove related ressources (the buffers too: the fd number can be reused by the next connection)
    ::close(fd);
    this->_socket->discard(fd);
    this->_payloads.erase(fd);
    if (this->_toClean == fd) _unlikely {this->_toClean = -2;}
}

_hot _nodiscard std::vector<int> utils::network::Server::getFds(void) const
{
    std::lock_guard lock(this->_lock);

    std::vector<int> ids;
    ids.reserve(this->_payloads.size());
    for (const auto &[fd, _]: this->_payloads) ids.push_back(fd);
    return ids;
}
