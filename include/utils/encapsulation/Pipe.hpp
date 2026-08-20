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
##  @file Pipe.hpp

File Description:
##  Basic encapsulation for pipe
\**************************************************************/

#ifndef PIPE_H
    #define PIPE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../security/observer/Observer.hpp"    // utils::security::observer::Observer
    #include "../attribute/Attribute.hpp"           // _cold, _nodiscard
    #include <unistd.h>                             // ::close
    #include <array>                                // std::array

namespace utils::encapsulation { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Pipe: private utils::security::observer::Observer<"Pipe"> {
    private:
        std::array<int, 2> _fds = {-1, -1}; // {read, write}

        // ------------ Function ---------- //
        _cold inline void close_(int& fd) noexcept {if (fd != -1) ::close(fd); fd = -1;};

    public:
        // ------------ Function ---------- //
        void trigger(void);

        /* close */
        _cold inline void closeRead(void) noexcept {this->close_(this->_fds[0]);};
        _cold inline void closeWrite(void) noexcept {this->close_(this->_fds[1]);};
        _cold inline void close(void) noexcept {this->closeRead(); this->closeWrite();};
        _cold inline void clear(void) {this->_fds[0] = -1; this->_fds[1] = -1;};

        /* getter */
        _cold _nodiscard inline const std::array<int, 2>& getFds(void) const {return this->_fds;};
        _cold _nodiscard inline int getRead(void) const {return this->_fds[0];};
        _cold _nodiscard inline int getWrite(void) const {return this->_fds[1];};

        /* setter */
        _cold inline void setFds(const std::array<int, 2>& fds) {this->_fds = fds;};
        _cold inline void setRead(const int fd = -1) {this->_fds[0] = fd;};
        _cold inline void setWrite(const int fd = -1) {this->_fds[1] = fd;};

        // ------------ Operator ---------- //
        Pipe& operator=(const Pipe& other) = delete;
        Pipe& operator=(Pipe&& other) {this->_fds = other._fds; other.clear(); return *this;};

        // ---------- Constructor --------- //
        Pipe(const int fds[2]): _fds{fds[0], fds[1]} {};
        Pipe(const std::array<int, 2>& fds): _fds{fds} {};
        Pipe(const int read = -1, const int write = -1): _fds{read, write} {};
        Pipe(const Pipe& other) = delete;
        Pipe(Pipe&& other): _fds{other._fds} {other.clear();};

        // ----------- Destructor --------- //
        ~Pipe() {this->close();};
};

} // namespace end
#endif /* PIPE_H */
