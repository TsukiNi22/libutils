/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 16/08/2026 by @author Tsukini

File Name:
##  @file IdHandler.hpp

File Description:
##  Handle id allocation
\**************************************************************/

#ifndef IDHANDLER_H
    #define IDHANDLER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"               // _hot, _cold, _unlikely, _likely, _nodiscard
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::InternalCode
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "../exception/custom/FatalException.hpp"   // utils::exception::FatalException
    #include <limits>                                   // std::numeric_limits<T>
    #include <mutex>                                    // std::mutex
    #include <set>                                      // std::set

namespace utils::security { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class IdHandler {
    static_assert(std::is_integral_v<T>, "T must be an integral type");

    private:
        mutable std::mutex _lock; // Handling of multithreading
        // 0 is reserved for unallocated ones / default value
        T _id = std::numeric_limits<T>::min();
        std::set<T> _freeIds;

    public:
        // ------------ Function ---------- //
        _cold _nodiscard inline T id(void) const {return this->_id;};
        _cold _nodiscard inline T actual(const bool safe_mode = true) const
        {
            std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
            if (safe_mode) lock.lock();
            else (void)lock.try_lock();

            if (this->_freeIds.size() > 0) _likely {return *this->_freeIds.begin();}
            else _unlikely {return this->_id;}
        };
        _cold _nodiscard T preview(const bool safe_mode = true) const
        {
            std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
            if (safe_mode) lock.lock();
            else (void)lock.try_lock();

            if (this->_freeIds.size() > 0) {
                return *this->_freeIds.begin();
            } else _likely {
                if (this->_id == std::numeric_limits<T>::max()) _unlikely {
                    throw utils::exception::FatalException(utils::exception::InternalCode::IdOverflow);
                }
                if (this->_id + 1 == 0) _unlikely {return this->_id + 2;}
                else _likely {return this->_id + 1;}
            }
            return 0;
        };
        _hot void allocate(T& id, const bool safe_mode = true)
        {
            std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
            if (safe_mode) lock.lock();
            else (void)lock.try_lock();

            if (this->_freeIds.size() > 0) {
                auto it = this->_freeIds.begin();
                id = *it;
                this->_freeIds.erase(id);
            } else _likely {
                if (this->_id == std::numeric_limits<T>::max()) _unlikely {
                    throw utils::exception::FatalException(utils::exception::InternalCode::IdOverflow);
                }
                id = ++this->_id;
                if (id == 0) _unlikely {id = ++this->_id;}
            }
        };
        _hot void free(T& id, const bool safe_mode = true)
        {
            std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
            if (safe_mode) lock.lock();
            else (void)lock.try_lock();

            // Only if the id wasn't already free
            if (!this->_freeIds.insert(id).second) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::DoubleFree);
            }
            id = 0;
        };

        // ------------ Operator ---------- //
        IdHandler& operator=(const IdHandler& other) = delete;
        IdHandler& operator=(IdHandler&& other) = delete;

        // ---------- Constructor --------- //
        IdHandler() = default;
        IdHandler(const IdHandler& other) = delete;
        IdHandler(IdHandler&& other) = delete;

        // ----------- Destructor --------- //
        ~IdHandler() = default;
};

} // namespace end
#endif /* IDHANDLER_H */
