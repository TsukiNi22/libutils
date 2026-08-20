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
    #include <unistd.h> // ::close
    #include <array>    // std::array

namespace utils::encapsulation { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Pipe {
    private:
        std::array<int, 2> _fds = {-1, -1};

        // ------------ Function ---------- //
        void close(int& fd) noexcept {if (fd != -1) ::close(fd); fd = -1;};

    public:
        // ------------ Function ---------- //
        void trigger(void);

        /* close */
        _cold inline void closeRead(void) noexcept {this->close(this->_fds[0]);};
        _cold inline void closeWrite(void) noexcept {this->close(this->_fds[1]);};
        _cold inline void close(void) noexcept {this->closeRead(); this->closeWrite();};

        /* getter */
        _cold _nodiscard const std::array<int, 2>& getFd(void) const {return this->_fds;};
        _cold _nodiscard int getRead(void) const {return this->_fds[0];};
        _cold _nodiscard int getWrite(void) const {return this->_fds[1];};

        // ------------ Operator ---------- //
        Pipe& operator=(const Pipe& other) = delete;
        Pipe& operator=(Pipe&& other) = delete;

        // ---------- Constructor --------- //
        Pipe(int fds[2]): _fds{fds[0], fds[1]} {};
        Pipe(const std::array<int, 2>& fds): _fds{fds} {};
        Pipe(int read = -1, int write = -1): _fds{read, write} {};
        Pipe(const Pipe& other) = delete;
        Pipe(Pipe&& other) = delete;

        // ----------- Destructor --------- //
        ~Pipe() {this->close();};
};

} // namespace end
#endif /* PIPE_H */
