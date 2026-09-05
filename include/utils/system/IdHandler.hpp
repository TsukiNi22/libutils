/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 05/09/2026 by @author Tsukini

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

namespace utils::system { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class IdHandler {
    static_assert(std::is_integral_v<T>, "T must be an integral type");

    private:
        mutable std::mutex _lock; // Handling of multithreading
        // 0 is reserved for unallocated ones / default value
        T _id = std::numeric_limits<T>::min();
        std::set<T> _usedIds; // only for forced allocated ids
        std::set<T> _freeIds;

        // ------------ Function ---------- //
        _hot T allocate_(const bool safe_mode = true)
        {
            std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
            if (safe_mode) lock.lock();
            else (void)lock.try_lock();

            T id;
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
                while (this->_usedIds.contains(id)) _unlikely {id = ++this->_id;}
            }
            return id;
        };
        _hot void free_(const T id, const bool safe_mode = true)
        {
            std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
            if (safe_mode) lock.lock();
            else (void)lock.try_lock();

            // Only if the id wasn't already free
            if (!this->_freeIds.insert(id).second) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::DoubleFree);
            }

            // Check if it's need to be removed from forced
            if (!this->_usedIds.contains(id)) {this->_usedIds.erase(id);}
        };

    public:
        // ------------ Function ---------- //
        _cold inline void use(T id, const bool safe_mode = true)
        {
            std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
            if (safe_mode) lock.lock();
            else (void)lock.try_lock();

            // Check if the id is free
            if (this->_freeIds.contains(id)) _unlikely {
                this->_freeIds.erase(id);
            } else if (this->_id <= id) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::DoubleUse);
            }

            // Store the forced id
            if (!this->_usedIds.insert(id).second) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::DoubleUse);
            }
        };
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
        _hot T allocate(T& id, const bool safe_mode = true)     {return (id = this->allocate_(safe_mode));};
        _hot _nodiscard T allocate(const bool safe_mode = true) {return this->allocate_(safe_mode);};
        _hot void free(T& id, const bool safe_mode = true)       {this->free_(id, safe_mode); id = 0;}
        _hot void free(const T& id, const bool safe_mode = true) {this->free_(id, safe_mode);};
        _cold void free(const bool safe_mode = true)
        {
            std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
            if (safe_mode) lock.lock();
            else (void)lock.try_lock();

            // Reset value (no id in circulation)
            this->_id = std::numeric_limits<T>::min();
            this->_freeIds.clear();
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
