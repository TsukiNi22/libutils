/**************************************************************\
Edition:
##  @date 27/04/2026 by @author Tsukini

File Name:
##  @file UnsafeObserver.hpp

File Description:
##  UnsafeObserver used for the different warning
\**************************************************************/

#ifndef UNSAFEOBSERVER_H
    #define UNSAFEOBSERVER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // nodiscard, outdated
    #include "SharedObject.hpp"             // utils::warning::WarningInstance::SharedObject
    #include <cstdint>                      // std::uint32_t
    #include <string>                       // std::string

namespace utils::warning { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class outdated("Thread unsafe observer used") UnsafeObserver {
    private:
        std::uint32_t _id = 0;

        // ---------- Pre-Function -------- //
        void link(void);
        void unlink(void);

        // ------------ Function ---------- //
        inline nodiscard const std::string getInstanceName() const noexcept {return "[Unknow]";};

    public:
        // ------------ Operator ---------- //
        UnsafeObserver& operator=(const UnsafeObserver& other);
        UnsafeObserver& operator=(UnsafeObserver&& other);

        // ---------- Constructor --------- //
        explicit UnsafeObserver();
        UnsafeObserver(const UnsafeObserver& other);
        UnsafeObserver(UnsafeObserver&& other);

        // ----------- Destructor --------- //
        ~UnsafeObserver();
};

} // namespace end
#endif /* UNSAFEOBSERVER_H */
