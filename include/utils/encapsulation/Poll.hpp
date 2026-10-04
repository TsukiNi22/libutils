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
##  @file Poll.hpp

File Description:
##  Encapsulation of the epoll
\**************************************************************/

#ifndef POLL_H
    #define POLL_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../security/observer/Observer.hpp"    // utils::security::observer::Observer
    #include "../attribute/Attribute.hpp"           // _hot, _cold, _nodiscard
    #include <sys/epoll.h>                          // struct epoll_event, EPOLL_CLOEXEC
    #include <unistd.h>                             // ::close
    #include <unordered_set>                        // std::unordered_set
    #include <cstddef>                              // std::size_t
    #include <cstdint>                              // std::uint32_t
    #include <utility>                              // std::move
    #include <vector>                               // std::vector

namespace utils::encapsulation { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Poll: private utils::security::observer::Observer<"Poll"> {
    private:
        int _fd = -1;
        std::unordered_set<int> _registered;

    public:
        // ---------- Pre-Function -------- //
        /* setup */
        void init(int flags = EPOLL_CLOEXEC);
        void close(void);

        /* edit */
        void link(int fd, std::uint32_t events, void* data = nullptr);
        void edit(int fd, std::uint32_t events, void* data = nullptr) const;
        void unlink(int fd);

        /* tool */
        std::vector<struct epoll_event> wait(int delay = -1, std::size_t limits = 0) const;

        // ------------ Function ---------- //
        _hot _nodiscard inline std::size_t size(void) const {return this->_registered.size();};
        _hot _nodiscard inline bool contains(int fd) const  {return this->_registered.contains(fd);};
        _cold _nodiscard inline int getFd(void) const       {return this->_fd;};

        // ------------ Operator ---------- //
        Poll& operator=(const Poll& other) = delete;
        Poll& operator=(Poll&& other);

        // ---------- Constructor --------- //
        Poll(int flags = EPOLL_CLOEXEC) {this->init(flags);};
        Poll(const Poll& other) = delete;
        Poll(Poll&& other): _fd{other._fd}, _registered{std::move(other._registered)} {other._fd = -1;};

        // ----------- Destructor --------- //
        ~Poll() {this->close();};
};

} // namespace end
#endif /* POLL_H */
