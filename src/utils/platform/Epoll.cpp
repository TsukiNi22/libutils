/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 12/10/2026 by @author Tsukini

File Name:
##  @file Epoll.cpp

File Description:
##  Definition of the epoll functions (Unix: system, Windows: emulation over WSAPoll)
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/platform/Platform.hpp"
#include "utils/platform/Epoll.hpp"
#ifdef UTILS_WINDOWS
    #include "Windows.hpp"
#else
    #include <unistd.h>
#endif
#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <chrono>
#include <thread>
#include <vector>
#include <cerrno>
#include <mutex>

//----------------------------------------------------------------//
/* EMULATION */

#ifdef UTILS_WINDOWS
    namespace {
        // One epoll instance: the linked sockets and their requested events
        struct Instance {
            std::mutex lock;
            std::unordered_map<int, struct epoll_event> fds;
        };

        // Every instance, by handle (function statics: built on the first use)
        struct Registry {
            std::mutex lock;
            std::unordered_map<int, std::shared_ptr<Instance>> instances;
            int next = 1 << 20; // far from the CRT fds (debug readability only)
        };

        Registry& registry(void)
        {
            static Registry r;
            return r;
        }

        std::shared_ptr<Instance> find(int epfd)
        {
            Registry& r = registry();
            std::lock_guard lock(r.lock);
            std::unordered_map<int, std::shared_ptr<Instance>>::iterator it = r.instances.find(epfd);
            return (it == r.instances.end()) ? nullptr : it->second;
        }

        // epoll events -> WSAPoll events (errors & hang up are always reported)
        SHORT to_poll(std::uint32_t events)
        {
            SHORT res = 0;
            if (events & (EPOLLIN | EPOLLRDNORM | EPOLLRDHUP)) res |= POLLRDNORM;
            if (events & EPOLLPRI) res |= POLLRDBAND;
            if (events & (EPOLLOUT | EPOLLWRNORM)) res |= POLLWRNORM;
            return res;
        }

        // WSAPoll events -> epoll events (a hang up is readable: recv returns 0 like on Linux)
        std::uint32_t to_epoll(SHORT revents, std::uint32_t requested)
        {
            std::uint32_t res = 0;
            if (revents & POLLRDNORM) res |= EPOLLIN;
            if (revents & POLLRDBAND) res |= EPOLLPRI;
            if (revents & POLLWRNORM) res |= EPOLLOUT;
            if (revents & POLLERR) res |= EPOLLERR;
            if (revents & POLLHUP) {
                res |= EPOLLHUP;
                if (requested & EPOLLIN) res |= EPOLLIN;
                if (requested & EPOLLRDHUP) res |= EPOLLRDHUP;
            }
            return res & (requested | EPOLLERR | EPOLLHUP);
        }
    }
#endif

//----------------------------------------------------------------//
/* FUNCTION */

_cold _nodiscard int utils::platform::epoll_create1(_unused int flags)
{
    #ifdef UTILS_WINDOWS
        utils::platform::windows::startup();
        Registry& r = registry();
        std::lock_guard lock(r.lock);
        const int epfd = r.next++;
        r.instances.emplace(epfd, std::make_shared<Instance>());
        return epfd;
    #else
        return ::epoll_create1(flags);
    #endif
}

#ifdef UTILS_WINDOWS
    _cold bool utils::platform::windows::epoll_release(int epfd)
    {
        Registry& r = registry();
        std::lock_guard lock(r.lock);
        return r.instances.erase(epfd) > 0;
    }
#endif

_hot int utils::platform::epoll_ctl(int epfd, int op, int fd, struct epoll_event* event)
{
    #ifdef UTILS_WINDOWS
        std::shared_ptr<Instance> instance = find(epfd);
        if (!instance) _unlikely {errno = EBADF; return -1;}
        if (op != EPOLL_CTL_DEL && !event) _unlikely {errno = EFAULT; return -1;}
        if (op != EPOLL_CTL_DEL && (event->events & EPOLLET)) _unlikely {errno = EINVAL; return -1;} // edge triggered: no equivalent
        if (op != EPOLL_CTL_DEL && !utils::platform::windows::is_socket(fd)) _unlikely {errno = ENOTSOCK; return -1;} // WSAPoll only handles sockets

        std::lock_guard lock(instance->lock);
        const bool exist = instance->fds.contains(fd);
        switch (op) {
            case EPOLL_CTL_ADD:
                if (exist) _unlikely {errno = EEXIST; return -1;}
                instance->fds[fd] = *event;
                return 0;
            case EPOLL_CTL_MOD:
                if (!exist) _unlikely {errno = ENOENT; return -1;}
                instance->fds[fd] = *event;
                return 0;
            case EPOLL_CTL_DEL:
                if (!exist) _unlikely {errno = ENOENT; return -1;}
                instance->fds.erase(fd);
                return 0;
            default:
                errno = EINVAL;
                return -1;
        }
    #else
        return ::epoll_ctl(epfd, op, fd, event);
    #endif
}

_hot int utils::platform::epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)
{
    #ifdef UTILS_WINDOWS
        std::shared_ptr<Instance> instance = find(epfd);
        if (!instance) _unlikely {errno = EBADF; return -1;}
        if (!events || maxevents <= 0) _unlikely {errno = EINVAL; return -1;}

        // Wait by slices of 10ms: a socket linked by another thread is seen at the next slice (like epoll)
        using clock = std::chrono::steady_clock;
        const clock::time_point deadline = clock::now() + std::chrono::milliseconds(std::max(timeout, 0));
        while (true) {
            int slice = 10;
            if (timeout >= 0) {
                const long long left = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - clock::now()).count();
                slice = static_cast<int>(std::clamp<long long>(left, 0, 10));
            }

            // Snapshot of the linked sockets (a disabled EPOLLONESHOT has no event left)
            std::vector<WSAPOLLFD> pfds;
            std::vector<std::uint32_t> requested;
            {
                std::lock_guard lock(instance->lock);
                for (const auto &[fd, ev]: instance->fds) {
                    if ((ev.events & ~EPOLLONESHOT) == 0) continue;
                    pfds.push_back(WSAPOLLFD{static_cast<SOCKET>(fd), to_poll(ev.events), 0});
                    requested.push_back(ev.events);
                }
            }

            int res = 0;
            if (pfds.empty()) {
                if (slice > 0) std::this_thread::sleep_for(std::chrono::milliseconds(slice));
            } else {
                res = ::WSAPoll(pfds.data(), static_cast<ULONG>(pfds.size()), slice);
                if (res == SOCKET_ERROR) _unlikely {
                    // A socket closed without EPOLL_CTL_DEL: forget it (a closed fd leaves the epoll on Linux)
                    std::lock_guard lock(instance->lock);
                    bool removed = false;
                    for (const WSAPOLLFD& pfd: pfds) {
                        if (!utils::platform::windows::is_socket(static_cast<int>(pfd.fd))) {
                            instance->fds.erase(static_cast<int>(pfd.fd));
                            removed = true;
                        }
                    }
                    if (!removed) {errno = EINVAL; return -1;}
                    continue;
                }
            }

            // Report the events
            int count = 0;
            if (res > 0) {
                std::lock_guard lock(instance->lock);
                for (std::size_t i = 0; i < pfds.size() && count < maxevents; ++i) {
                    const int fd = static_cast<int>(pfds[i].fd);
                    if (pfds[i].revents & POLLNVAL) {instance->fds.erase(fd); continue;} // closed meanwhile
                    const std::uint32_t ready = to_epoll(pfds[i].revents, requested[i]);
                    std::unordered_map<int, struct epoll_event>::iterator it = instance->fds.find(fd);
                    if (ready == 0 || it == instance->fds.end()) continue; // unlinked meanwhile
                    events[count].events = ready;
                    events[count].data = it->second.data;
                    ++count;
                    if (it->second.events & EPOLLONESHOT) it->second.events = EPOLLONESHOT; // disabled until EPOLL_CTL_MOD
                }
            }
            if (count > 0) return count;
            if (timeout >= 0 && clock::now() >= deadline) return 0;
        }
    #else
        return ::epoll_wait(epfd, events, maxevents, timeout);
    #endif
}
