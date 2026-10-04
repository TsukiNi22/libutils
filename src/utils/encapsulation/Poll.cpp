/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 06/09/2026 by @author Tsukini

File Name:
##  @file Poll.cpp

File Description:
##  Definition of the Poll's methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/encapsulation/Poll.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstddef>
#include <cstring>
#include <utility>
#include <vector>
#include <cerrno>

_cold utils::encapsulation::Poll& utils::encapsulation::Poll::operator=(utils::encapsulation::Poll&& other)
{
    this->close();
    this->_fd = other._fd;
    this->_registered = std::move(other._registered);
    other._fd = -1;
    return *this;
}

_cold void utils::encapsulation::Poll::init(int flags)
{
    this->_fd = epoll_create1(flags);
    if (this->_fd == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::EPoll, std::strerror(errno));
    }
}

_cold void utils::encapsulation::Poll::close(void)
{
    if (this->_fd != -1) ::close(this->_fd);
    this->_fd = -1;
    this->_registered.clear();
}

_cold void utils::encapsulation::Poll::link(int fd, std::uint32_t events, void* data)
{
    struct epoll_event ev{};
    ev.events = events;
    if (data) _unlikely {
        ev.data.ptr = data;
    } else {
        ev.data.fd = fd;
    }

    if (epoll_ctl(this->_fd, EPOLL_CTL_ADD, fd, &ev) == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::EPoll, std::strerror(errno));
    }

    this->_registered.insert(fd);
}

_cold void utils::encapsulation::Poll::edit(int fd, std::uint32_t events, void* data) const
{
    struct epoll_event ev{};
    ev.events = events;
    if (data) _unlikely {
        ev.data.ptr = data;
    } else {
        ev.data.fd = fd;
    }

    if (epoll_ctl(this->_fd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::EPoll, std::strerror(errno));
    }
}

_cold void utils::encapsulation::Poll::unlink(int fd)
{
    if (epoll_ctl(this->_fd, EPOLL_CTL_DEL, fd, nullptr) == -1) _unlikely {
        // ENOENT/EBADF: fd is no longer used (already closed or removed)
        if (errno != ENOENT && errno != EBADF) _likely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::EPoll, std::strerror(errno));
        }
    }
    this->_registered.erase(fd);
}

_hot _nodiscard std::vector<struct epoll_event> utils::encapsulation::Poll::wait(int delay, std::size_t limits) const
{
    const std::size_t max = (limits != 0) ? limits : (this->_registered.empty() ? 1 : this->_registered.size());
    std::vector<struct epoll_event> events(max);

    int n = 0;
    do {n = epoll_wait(this->_fd, events.data(), static_cast<int>(max), delay);} while (n == -1 && errno == EINTR); // external interruption (not an error)

    if (n == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::EPoll, std::strerror(errno));
    }

    events.resize(static_cast<std::size_t>(n));
    return events;
}
