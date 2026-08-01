/**************************************************************\
Edition:
##  @date 01/08/2026 by @author Tsukini

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
    #include "../../manip/smanip/fixed_string.hpp"  // utils::smanip::fixed_string
    #include "AObserver.hpp"                        // utils::security::observer::AObserver

namespace utils::security::observer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<utils::smanip::fixed_string __instance__>
class UnsafeObserver: public utils::security::observer::AObserver<__instance__, false> {
    public:
        // ------------ Operator ---------- //
        UnsafeObserver& operator=(const UnsafeObserver& other) = default;
        UnsafeObserver& operator=(UnsafeObserver&& other) = default;

        // ---------- Constructor --------- //
        UnsafeObserver() = default;
        UnsafeObserver(const UnsafeObserver& other) = default;
        UnsafeObserver(UnsafeObserver&& other) = default;

        // ----------- Destructor --------- //
        ~UnsafeObserver() = default;
};

} // namespace end
#endif /* UNSAFEOBSERVER_H */
