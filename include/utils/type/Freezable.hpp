/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 27/08/2026 by @author Tsukini

File Name:
##  @file Freezable.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef FREEZABLE_H
    #define FREEZABLE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"               // _nodiscard, _hot, _cold, _unlikely
    #include "../security/observer/Observer.hpp"        // utils::security::observer::Observer
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::* (Type)
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include <atomic>                                   // std::atomic

namespace utils::type { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Freezable: private utils::security::observer::Observer<"Freezable"> {
    private:
        std::atomic<bool> _frozen = false;

        // ------------ Function ---------- //
        _hot inline void requireFrozenState(const bool frozen) const // throw if frozen status doesn't match the demanded one
        {if (this->_frozen != frozen) _unlikely {throw utils::exception::ErrorException(frozen ? utils::exception::InternalCode::ShouldBeFrozen : utils::exception::InternalCode::Frozen);}}

    public:
        // ------------ Function ---------- //
        _cold inline void freeze(void) {this->_frozen = true;};
        _hot _nodiscard inline bool isFrozen(void) const {return this->_frozen;};

        /* auto throw */
        _hot inline void requireFrozen(void) const {this->requireFrozenState(true);};
        _hot inline void requireUnfrozen(void) const {this->requireFrozenState(false);};

        // ------------ Operator ---------- //
        Freezable& operator=(const Freezable& other) {this->_frozen = other._frozen.load(); return *this;};
        Freezable& operator=(Freezable&& other) {this->_frozen = other._frozen.load(); return *this;};

        // ---------- Constructor --------- //
        Freezable(const bool frozen): _frozen{frozen} {};
        Freezable(const Freezable& other): _frozen{other._frozen.load()} {};
        Freezable(Freezable&& other): _frozen{other._frozen.load()} {};

        // ----------- Destructor --------- //
        ~Freezable() = default;
};

} // namespace end
#endif /* FREEZABLE_H */
