/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 06/07/2026 by @author Tsukini

File Name:
##  @file Middlewares.hpp

File Description:
##  Declaration of the Middlewares<void, T>
\**************************************************************/

#ifndef MIDDLEWARES_VOID_T_H
    #define MIDDLEWARES_VOID_T_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../warning/Observer.hpp"                  // utils::warning::Observer
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::Type, utils::exception::Code
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "MiddlewaresType.hpp"                      // utils::middleware::Middleware<...>
    #include <shared_mutex>                             // std::shared_mutex, std::unique_lock, std::shared_lock
    #include <functional>                               // std::function
    #include <exception>                                // std::exception

namespace utils::middleware { // namespace
//----------------------------------------------------------------//
/* CLASS */

template<typename U>
class Middlewares<void, U>: private utils::warning::Observer {
    public:
        mutable std::shared_mutex _lock;
        std::vector<utils::middleware::Middleware<void>> before;
        std::vector<utils::middleware::Middleware<U>> after;

        // ------------ Function ---------- //
        void clear() {this->before.clear(); this->after.clear();}

        /* adder */
        void addBefore(utils::middleware::Middleware<void>& toAdd)                     {std::unique_lock lock(this->_lock); this->before.push_back(toAdd);}
        void addBefore(const std::vector<utils::middleware::Middleware<void>>& toAdds) {std::unique_lock lock(this->_lock); this->before.insert(before.end(), toAdds.begin(), toAdds.end());}
        void addAfter(utils::middleware::Middleware<U>& toAdd)                         {std::unique_lock lock(this->_lock); this->after.push_back(toAdd);}
        void addAfter(const std::vector<utils::middleware::Middleware<U>>& toAdds)     {std::unique_lock lock(this->_lock); this->after.insert(after.end(), toAdds.begin(), toAdds.end());}

        /* caller */
        void callBefore() const
        {
            std::shared_lock lock(this->_lock);
            for (const utils::middleware::Middleware<void>& middleware: this->before) {
                try {middleware();}
                catch (const std::exception& e) {throw utils::exception::ErrorException(utils::exception::Code::MiddlewareCall, e.what());}
            }
        }
        void callAfter(U arg) const
        {
            std::shared_lock lock(this->_lock);
            for (const utils::middleware::Middleware<U>& middleware: this->after) {
                try {middleware(arg);}
                catch (const std::exception& e) {throw utils::exception::ErrorException(utils::exception::Code::MiddlewareCall, e.what());}
            }
        }

        // ------------ Operator ---------- //
        Middlewares& operator=(const Middlewares& other)
        {
            if (this == &other) return *this;
            std::unique_lock localLock(_lock);
            std::shared_lock lock(other._lock);
            this->before = other.before;
            this->after = other.after;
            return *this;
        }
        Middlewares& operator=(Middlewares&& other)
        {
            if (this == &other) return *this;
            std::unique_lock localLock(_lock);
            std::shared_lock lock(other._lock);
            this->before = std::move(other.before);
            this->after = std::move(other.after);
            return *this;
        }

        // ---------- Constructor --------- //
        Middlewares() = default;
        Middlewares(const Middlewares& other)
        {
            std::shared_lock lock(other._lock);
            this->before = other.before;
            this->after = other.after;
        }
        Middlewares(Middlewares&& other)
        {
            std::unique_lock lock(other._lock);
            this->before = std::move(other.before);
            this->after = std::move(other.after);
        }

        // ----------- Destructor --------- //
        ~Middlewares() = default;
};

} // namespace end
#endif /* MIDDLEWARES_VOID_T_H */
