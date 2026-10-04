/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/08/2026 by @author Tsukini

File Name:
##  @file Middlewares_t-void.hpp

File Description:
##  Declaration of the Middlewares<T, void>
\**************************************************************/

#ifndef MIDDLEWARESTVOID_H
    #define MIDDLEWARESTVOID_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../security/observer/Observer.hpp"     // utils::security::observer::Observer
    #include "../../exception/ExceptionDefine.hpp"      // utils::exception::Type, utils::exception::InternalCode
    #include "../../exception/basic/ErrorException.hpp" // utils::exception::ErrorException
    #include "../../attribute/Attribute.hpp"            // _cold, _hot
    #include "MiddlewaresType.hpp"                      // utils::pool::Middleware<...>
    #include <shared_mutex>                             // std::shared_mutex, std::unique_lock, std::shared_lock
    #include <functional>                               // std::function
    #include <exception>                                // std::exception
    #include <utility>                                  // std::move
    #include <vector>                                   // std::vector
    #include <mutex>                                    // std::lock, std::scoped_lock, std::defer_lock

namespace utils::pool { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class Middlewares<T, void>: private utils::security::observer::Observer<"Middlewares"> {
    public:
        mutable std::shared_mutex _lock;
        std::vector<utils::pool::Middleware<T>> before;
        std::vector<utils::pool::Middleware<void>> after;

        // ------------ Function ---------- //
        _cold inline void clear(void) {std::unique_lock lock(this->_lock); this->before.clear(); this->after.clear();};

        /* adder */
        _cold inline void addBefore(const utils::pool::Middleware<T>& toAdd)                 {std::unique_lock lock(this->_lock); this->before.push_back(toAdd);};
        _cold inline void addBefore(const std::vector<utils::pool::Middleware<T>>& toAdds)   {std::unique_lock lock(this->_lock); this->before.insert(this->before.end(), toAdds.begin(), toAdds.end());};
        _cold inline void addAfter(const utils::pool::Middleware<void>& toAdd)               {std::unique_lock lock(this->_lock); this->after.push_back(toAdd);};
        _cold inline void addAfter(const std::vector<utils::pool::Middleware<void>>& toAdds) {std::unique_lock lock(this->_lock); this->after.insert(this->after.end(), toAdds.begin(), toAdds.end());};

        /* caller */
        _hot void callBefore(T arg) const
        {
            std::vector<utils::pool::Middleware<T>> snapshot;
            {std::shared_lock lock(this->_lock); snapshot = this->before;} // called without the lock: a middleware can edit the middlewares
            for (const utils::pool::Middleware<T>& middleware: snapshot) {
                try {middleware(arg);}
                catch (const std::exception& e) {throw utils::exception::ErrorException(utils::exception::InternalCode::MiddlewareCall, e.what());}
            }
        };
        _hot void callAfter(void) const
        {
            std::vector<utils::pool::Middleware<void>> snapshot;
            {std::shared_lock lock(this->_lock); snapshot = this->after;} // called without the lock: a middleware can edit the middlewares
            for (const utils::pool::Middleware<void>& middleware: snapshot) {
                try {middleware();}
                catch (const std::exception& e) {throw utils::exception::ErrorException(utils::exception::InternalCode::MiddlewareCall, e.what());}
            }
        };

        // ------------ Operator ---------- //
        Middlewares& operator=(const Middlewares& other)
        {
            if (this == &other) return *this;
            std::unique_lock localLock(this->_lock, std::defer_lock);
            std::shared_lock lock(other._lock, std::defer_lock);
            std::lock(localLock, lock);
            this->before = other.before;
            this->after = other.after;
            return *this;
        };
        Middlewares& operator=(Middlewares&& other)
        {
            if (this == &other) return *this;
            std::scoped_lock lock(this->_lock, other._lock);
            this->before = std::move(other.before);
            this->after = std::move(other.after);
            return *this;
        };

        // ---------- Constructor --------- //
        Middlewares() = default;
        Middlewares(const Middlewares& other)
        {
            std::shared_lock lock(other._lock);
            this->before = other.before;
            this->after = other.after;
        };
        Middlewares(Middlewares&& other)
        {
            std::unique_lock lock(other._lock);
            this->before = std::move(other.before);
            this->after = std::move(other.after);
        };

        // ----------- Destructor --------- //
        ~Middlewares() = default;
};

} // namespace end
#endif /* MIDDLEWARESTVOID_H */
