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
    #include "../../manip/smanip/FixedString.hpp"   // utils::smanip::FixedString
    #include "AObserver.hpp"                        // utils::security::observer::AObserver

namespace utils::security::observer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<utils::smanip::FixedString instance>
class UnsafeObserver: public utils::security::observer::AObserver<instance, false> {
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
