/**************************************************************\
Edition:
##  @date 20/07/2026 by @author Tsukini

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
    #include "../attribute/Attribute.hpp"   // _nodiscard, _deprecated
    #include "SharedObject.hpp"             // utils::warning::WarningInstance::SharedObject
    #include <cstdint>                      // std::uint32_t
    #include <string>                       // std::string

namespace utils::warning { // namespace start
//----------------------------------------------------------------//
/* CLASS */

//class _deprecated("UnsafeObserver dosen't have any memory safty (Be careful!!!)") UnsafeObserver {
class UnsafeObserver {
    private:
        std::uint32_t _id = 0;

        // ---------- Pre-Function -------- //
        void link(void);
        void unlink(void);

        // ------------ Function ---------- //
        _nodiscard inline const std::string getInstanceName(void) const noexcept {return "[Unknow]";};

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
