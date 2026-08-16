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
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/basic/WarningException.hpp"
#include "utils/network/NetworkDefine.hpp"
#include "utils/network/Server.hpp"
#include "utils/verbose/Verbose.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <format>

_cold void utils::network::Server::start(void)
{
    if (this->_status == utils::network::Status::Up) {
        throw utils::exception::WarningException(utils::exception::InternalCode::AlreadyRunning);
    } else if (this->_status == utils::network::Status::Terminated) {
        throw utils::exception::WarningException(utils::exception::InternalCode::Killed);
    }
    onBasicVerbose("Starting server...");

    // Start the socket
    try {
        // Open the socket (doesn't restart open already open)
        if (this->_socket->getFd() == -1) _likely {
            this->_socket->listen(this->_address);
        }

        // Setup the epoll
        this->_fd = this->_socket->getFd();
        this->_epfd = epoll_create1(0);
        if (this->_epfd < 0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, strerror(errno));
        }

        // Init the socket fd (server)
        struct epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = this->_fd;
        if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, this->_fd, &ev) < 0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, strerror(errno));
        }

        this->_status = utils::network::Status::Up;
    } catch (...) {
        this->_status = utils::network::Status::Crashed;
        throw;
    }
}

_cold void utils::network::Server::stop(void)
{
    if (this->_status != utils::network::Status::Up) return;
    onBasicVerbose("Stopping server...");
    for (const auto& [fd, _]: this->_payloads) ::close(fd);
    this->_payloads.clear();
    this->_socket->close();
    ::close(this->_epfd);
    this->_epfd = -1;
    this->_status = utils::network::Status::Down;
}

_cold void utils::network::Server::kill(void)
{
    if (this->_status == utils::network::Status::Terminated) return;
    onBasicVerbose("Killing server...");
    for (const auto& [fd, _]: this->_payloads) ::close(fd);
    this->_payloads.clear();
    this->_socket->close();
    if (this->_epfd != -1) ::close(this->_epfd);
    this->_epfd = -1;
    this->_status = utils::network::Status::Terminated;
}

_hot _nodiscard const std::unordered_map<int, utils::network::Payloads>& utils::network::Server::listen_(const int fd)
{
    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return this->_payloads;}
    else if (fd != -1 && !this->_payloads.contains(fd)) _unlikely {
        throw utils::exception::WarningException(utils::exception::InternalCode::UnknowId, std::to_string(fd));
    }

    // Clear the last events getted
    if (this->_to_clean == -1) for (auto& [_, payloads]: this->_payloads) payloads.clear();
    else if (this->_to_clean >= 0) this->_payloads[this->_to_clean].clear();
    this->_to_clean = fd;

    // Read the events
    std::vector<struct epoll_event> events(this->_payloads.size() + 1);
    while (this->_status == utils::network::Status::Up) {
        std::this_thread::yield(); // To not fully take the cpu computing
        int res = epoll_wait(this->_epfd, events.data(), events.size(), 0);

        if (res < 0) _unlikely {
            if (errno == EINTR) return this->_payloads; // handle the ctrl-c before the first connection
            this->kill();
            this->_status = utils::network::Status::Crashed;
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, strerror(errno));
        }

        // No event
        if (res == 0) break; // Nothing more to read on the socket
        onDebugVerbose("Event(s) detected on poll: " << std::to_string(res));

        for (int i = 0; i < res; ++i) {
            int actualFd = events[i].data.fd;

            // Error or Invalid poll handling
            /*
             * EPOLLHUP  -> connection closed
             * EPOLLERR  -> error on the socket
            */
            if (events[i].events & (EPOLLERR | EPOLLHUP)) _unlikely {
                onDebugVerbose("Error detected on epoll events");
                if (actualFd == this->_fd) _unlikely {
                    onBasicVerbose("Detected invalid epoll events, exiting...");
                    this->kill();
                    this->_status = utils::network::Status::Crashed;
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Poll);
                } else {
                    onBasicVerbose("Detected invalid epoll events, remove client...");
                    this->remove(actualFd);
                    continue;
                }
            }

            // Nothing to read
            if (!(events[i].events & EPOLLIN)) continue;

            // Handle new connection
            if (actualFd == this->_fd) _unlikely {
                struct epoll_event ev{};
                ev.events = EPOLLIN;
                ev.data.fd = this->_socket->accept();
                if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, ev.data.fd, &ev) < 0) _unlikely {
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, strerror(errno));
                }
                this->_payloads[ev.data.fd]; // Ensure the existance of the new connection in the hashtable
            }

            // Handle event
            else if (fd == -1 || _unlikely_c(actualFd == fd)) { // Ignore those not concerned
                utils::network::Payloads& payloads = this->_payloads[actualFd];
                try {
                    payloads.emplace_back(this->_socket->recv(actualFd)); // Read the event
                    std::vector<std::string> ret = this->_socket->recvAll(actualFd); // Get all the remaning payload on the buffer
                    payloads.insert(payloads.end(), ret.begin(), ret.end());
                } catch (const utils::exception::IException& e) {
                    if (e.isNone() && e.getCode() == utils::exception::InternalCode::SocketClosed) {
                        onBasicVerbose("Socket closed, remove client...");
                    } else {
                        onBasicVerbose("Error during the client request handling, remove client...");
                        onBasicVerbose(e.formated());
                    }
                    this->remove(actualFd);
                    continue;
                }
            }
        }
    }

    // Ensure the correct reading when there is no event but data on the buffer
    if (fd == -1) {
        for (auto& [actualFd, payloads]: this->_payloads) {
            try {
                std::vector<std::string> ret = this->_socket->recvAll(actualFd); // Get all the remaning payload on the buffer
                payloads.insert(payloads.end(), ret.begin(), ret.end());
            } catch (const utils::exception::IException& e) {
                if (e.isNone() && e.getCode() == utils::exception::InternalCode::SocketClosed) {
                    onBasicVerbose("Socket closed, remove client...");
                } else {
                    onBasicVerbose("Error during the client request handling, remove client...");
                    onBasicVerbose(e.formated());
                }
                this->remove(actualFd);
            }
        }
    } else {
        utils::network::Payloads& payloads = this->_payloads[fd];
        try {
            std::vector<std::string> ret = this->_socket->recvAll(fd); // Get all the remaning payload on the buffer
            payloads.insert(payloads.end(), ret.begin(), ret.end());
        } catch (const utils::exception::IException& e) {
            if (e.isNone() && e.getCode() == utils::exception::InternalCode::SocketClosed) {
                onBasicVerbose("Socket closed, remove client...");
            } else {
                onBasicVerbose("Error during the client request handling, remove client...");
                onBasicVerbose(e.formated());
            }
            this->remove(fd);
        }
    }

    return this->_payloads;
}

_hot void utils::network::Server::join(const int fd)
{
    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}
    else if (fd != -1 && !this->_payloads.contains(fd)) _unlikely {
        throw utils::exception::WarningException(utils::exception::InternalCode::UnknowId, std::to_string(fd));
    }

    // Read the events
    std::vector<struct epoll_event> events(this->_payloads.size() + 1);
    while (this->_status == utils::network::Status::Up) {
        std::this_thread::yield(); // To not fully take the cpu computing
        int res = epoll_wait(this->_epfd, events.data(), events.size(), 10); // wake up at least evry 10ms

        if (res < 0) _unlikely {
            if (errno == EINTR) return; // handle the ctrl-c before the first connection
            this->kill();
            this->_status = utils::network::Status::Crashed;
            throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, strerror(errno));
        }

        // No event
        if (res == 0) continue;

        for (int i = 0; i < res; ++i) {
            int actualFd = events[i].data.fd;

            // Error or Invalid poll handling
            /*
             * EPOLLHUP  -> connection closed
             * EPOLLERR  -> error on the socket
            */
            if (events[i].events & (EPOLLERR | EPOLLHUP)) _unlikely {
                onDebugVerbose("Error detected on epoll events");
                if (actualFd == this->_fd) _unlikely {
                    onBasicVerbose("Detected invalid epoll events, exiting...");
                    this->kill();
                    this->_status = utils::network::Status::Crashed;
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Poll);
                } else {
                    onBasicVerbose("Detected invalid epoll events, remove client...");
                    this->remove(actualFd);
                    continue;
                }
            }

            // Ignore those not concerned
            if (fd != -1 && _likely_c(fd != actualFd)) continue;

            // Nothing to read
            if (!(events[i].events & EPOLLIN)) continue;

            // Break the waiting when there is valid event
            return;
        }
    }
}

_hot void utils::network::Server::flush(void)
{
    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}

    for (const auto& [fd, _]: this->_payloads) this->flush(fd);
}

_hot void utils::network::Server::flush(const int fd)
{
    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}
    else if (!this->_payloads.contains(fd)) _unlikely {
        throw utils::exception::WarningException(utils::exception::InternalCode::UnknowId, std::to_string(fd));
    }

    try {this->_socket->flush(fd);}
    catch (...) {
        this->remove(fd);
    }
}

template<>
_hot void utils::network::Server::send<false>(const int fd, const utils::network::Payload& payload)
{ 
    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}
    else if (!this->_payloads.contains(fd)) _unlikely {
        throw utils::exception::WarningException(utils::exception::InternalCode::UnknowId, std::to_string(fd));
    }

    try {this->_socket->send(payload, fd);}
    catch (...) {
        this->remove(fd);
    }
}

template<>
_hot void utils::network::Server::send<true>(const int fd, const utils::network::Payload& payload)
{
    // Check status
    if (this->_status != utils::network::Status::Up) _unlikely {return;}
    else if (!this->_payloads.contains(fd)) _unlikely {
        throw utils::exception::WarningException(utils::exception::InternalCode::UnknowId, std::to_string(fd));
    }

    try {this->_socket->sendBuffered(payload, fd);}
    catch (...) {
        this->remove(fd);
    }
}

_hot void utils::network::Server::remove(const int fd)
{
    if (!this->_payloads.contains(fd)) _unlikely {
        throw utils::exception::WarningException(utils::exception::InternalCode::UnknowId, std::to_string(fd));
    }

    // Verbose
    struct sockaddr_in addr{};
    socklen_t addrLen = sizeof(addr);
    if (getpeername(fd, reinterpret_cast<struct sockaddr*>(&addr), &addrLen) < 0) _unlikely {
        onBasicVerbose(std::format("Remove client 'fd={}' (unknown address: {})", fd, strerror(errno)));
    } else {
        onBasicVerbose(std::format("Remove client '{}:{}'", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port)));
    }

    // Remove the fd from epoll
    if (epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, nullptr) < 0) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Poll, strerror(errno));
    }

    // Close it and remove related ressources
    ::close(fd);
    this->_payloads.erase(fd);
    if (this->_to_clean == fd) _unlikely {this->_to_clean = -2;}
}

_hot _nodiscard std::vector<int> utils::network::Server::getFds(void) const
{
    std::vector<int> ids;
    ids.reserve(this->_payloads.size());
    for (const auto& [fd, _]: this->_payloads) ids.push_back(fd);
    return ids;
}
