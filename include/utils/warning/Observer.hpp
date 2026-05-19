/**************************************************************\
Edition:
##  @date 18/05/2026 by @author Tsukini

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
    #include "../attribute/Attribute.hpp"   // nodiscard
    #include "SharedObject.hpp"             // utils::warning::WarningInstance::SharedObject
    #include <cstdint>                      // std::uint32_t
    #include <string>                       // std::string

namespace utils::warning { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Observer {
    private:
        std::uint32_t _id = 0;

        // ---------- Pre-Function -------- //
        void link(void);
        void unlink(void);

        // ------------ Function ---------- //
        inline nodiscard const std::string getInstanceName() const noexcept {return "[Unknow]";};

    public:
        // ------------ Operator ---------- //
        Observer& operator=(const Observer& other);
        Observer& operator=(Observer&& other);

        // ---------- Constructor --------- //
        explicit Observer();
        Observer(const Observer& other);
        Observer(Observer&& other);

        // ----------- Destructor --------- //
        ~Observer();
};

} // namespace end
#endif /* OBSERVER_H */
