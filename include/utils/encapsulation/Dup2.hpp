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
##  @file Dup2.hpp

File Description:
##  Basic encapsulation for pipe
\**************************************************************/

#ifndef DUP2_H
    #define DUP2_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _cold, _nodiscard
    #include <unistd.h>                     // ::close

namespace utils::encapsulation { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Dup2 {
    private:
        int _origin = -1;
        int _clone = -1;

        // ------------ Function ---------- //
        _cold inline void close_(int& fd) noexcept {if (fd != -1) ::close(fd); fd = -1;};

    public:
        // ------------ Function ---------- //
        void trigger(void);

        /* close */
        _cold inline void closeOrigin(void) noexcept {this->close_(this->_origin);};
        _cold inline void closeClone(void) noexcept {this->close_(this->_clone);};
        _cold inline void close(void) noexcept {this->closeOrigin(); this->closeClone();};

        /* getter */
        _cold _nodiscard inline int getOrigin(void) const {return this->_origin;};
        _cold _nodiscard inline int getClone(void) const {return this->_clone;};

        /* setter */
        _cold inline void setOrigin(int fd = -1) {this->_origin = fd;};
        _cold inline void setClone(int fd = -1) {this->_clone = fd;};

        // ------------ Operator ---------- //
        Dup2& operator=(const Dup2& other) = delete;
        Dup2& operator=(Dup2&& other) = delete;

        // ---------- Constructor --------- //
        Dup2(int origin = -1, int clone = -1): _origin{origin}, _clone{clone} {};
        Dup2(const Dup2& other) = delete;
        Dup2(Dup2&& other) = delete;

        // ----------- Destructor --------- //
        ~Dup2() = default;
};

} // namespace end
#endif /* DUP2_H */
