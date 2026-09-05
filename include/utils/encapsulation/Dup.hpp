/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 29/08/2026 by @author Tsukini

File Name:
##  @file Dup.hpp

File Description:
##  Basic encapsulation for pipe
\**************************************************************/

#ifndef DUP_H
    #define DUP_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../security/observer/Observer.hpp"    // utils::security::observer::Observer
    #include "../attribute/Attribute.hpp"           // _cold, _nodiscard
    #include <unistd.h>                             // ::close

namespace utils::encapsulation { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Dup: private utils::security::observer::Observer<"Dup"> {
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
        _cold inline void clear(void) {this->_origin = -1; this->_clone = -1;};

        /* getter */
        _cold _nodiscard inline int getOrigin(void) const {return this->_origin;};
        _cold _nodiscard inline int getClone(void) const {return this->_clone;};

        /* setter */
        _cold inline void setOrigin(const int fd = -1) {this->_origin = fd;};
        _cold inline void setClone(const int fd = -1) {this->_clone = fd;};

        // ------------ Operator ---------- //
        Dup& operator=(const Dup& other) = delete;
        Dup& operator=(Dup&& other) {this->_origin = other._origin; this->_clone = other._clone; other.clear(); return *this;};

        // ---------- Constructor --------- //
        Dup(const int origin = -1, const int clone = -1): _origin{origin}, _clone{clone} {};
        Dup(const Dup& other) = delete;
        Dup(Dup&& other): _origin{other._origin}, _clone{other._clone} {other.clear();};

        // ----------- Destructor --------- //
        ~Dup() = default;
};

} // namespace end
#endif /* DUP_H */
