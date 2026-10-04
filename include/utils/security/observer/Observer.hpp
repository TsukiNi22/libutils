/**************************************************************\
Edition:
##  @date 06/09/2026 by @author Tsukini

File Name:
##  @file Observer.hpp

File Description:
##  Observer used for the different warning
\**************************************************************/

#ifndef OBSERVER_H
    #define OBSERVER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../manip/smanip/FixedString.hpp"   // utils::smanip::FixedString
    #include "AObserver.hpp"                        // utils::security::observer::AObserver
    #include <string>                               // std::string

namespace utils::security::observer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<utils::smanip::FixedString instance>
class Observer: public utils::security::observer::AObserver<instance, true> {
    public:
        // ------------ Operator ---------- //
        Observer& operator=(const Observer& other) = default;
        Observer& operator=(Observer&& other) = default;

        // ---------- Constructor --------- //
        Observer() = default;
        Observer(const Observer& other) = default;
        Observer(Observer&& other) = default;

        // ----------- Destructor --------- //
        ~Observer() = default;
};

} // namespace end
#endif /* OBSERVER_H */
