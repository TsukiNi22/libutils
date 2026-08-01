/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 01/08/2026 by @author Tsukini

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
    #include "../attribute/Attribute.hpp"               // _hot, _unlikely, _likely
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::InternalCode
    #include "../exception/custom/FatalException.hpp"   // utils::exception::FatalException
    #include <limits>                                   // std::numeric_limits<T>
    #include <vector>                                   // std::vector
    #include <mutex>                                    // std::mutex

namespace utils::security { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class IdHandler {
    static_assert(std::is_integral_v<T>, "T must be an integral type");

    private:
        std::mutex _lock; // Handling of multithreading
        // 0 is reserved for unallocated ones / default value
        T _id = std::numeric_limits<T>::min();
        std::vector<T> _freeIds;

    public:
        // ------------ Function ---------- //
        _hot void allocate(T& id, const bool safe_mode = true)
        {
            std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
            if (safe_mode) lock.lock();
            else (void)lock.try_lock();

            if (this->_freeIds.size() > 0) {
                id = this->_freeIds.back();
                this->_freeIds.pop_back();
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

            this->_freeIds.push_back(id);
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
