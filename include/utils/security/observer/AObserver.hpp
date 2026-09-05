/**************************************************************\
Edition:
##  @date 29/08/2026 by @author Tsukini

File Name:
##  @file AObserver.hpp

File Description:
##  Abstract version of the different observers
\**************************************************************/

#ifndef AOBSERVER_H
    #define AOBSERVER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"        // _nodiscard, _unused, _hot
    #include "../../manip/smanip/fixed_string.hpp"  // utils::smanip::fixed_string
    #include "IObserver.hpp"                        // utils::security::observer::IObserver
    #include "INotifier.hpp"                        // utils::security::observer::INotifier
    #include "Instances.hpp"                        // utils::security::observer::instances::*
    #include <string_view>                          // std::string_view
    #include <cstdint>                              // std::uint64_t
    #include <memory>                               // std::unique_ptr
    #include <string>                               // std::string

namespace utils::security::observer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<utils::smanip::fixed_string __instance__ = "[unknown]", bool __safe_mode__ = true>
class AObserver: public utils::security::observer::IObserver {
    private:
        // id = 0 is reserved for unattribued one
        std::uint64_t _id = 0; // up to 2^64 - 1 item at the same time
        std::string_view _instance = __instance__.view();

        // ------------ Function ---------- //
        _hot void link(void) override
        {
            utils::security::observer::instances::IdHandler.allocate(this->_id, __safe_mode__);
            for (std::unique_ptr<utils::security::observer::INotifier>& notifier: utils::security::observer::instances::Notifiers)
                notifier->link(this->_id, this->_instance, __safe_mode__);
        };
        _hot void unlink(void) override
        {
            if (this->_id == 0) return; // Ignore thoese who where already realese/transfered
            for (std::unique_ptr<utils::security::observer::INotifier>& notifier: utils::security::observer::instances::Notifiers)
                notifier->unlink(this->_id, __safe_mode__);
            utils::security::observer::instances::IdHandler.free(this->_id, __safe_mode__);
        };

    public:
        // ------------ Operator ---------- //
        AObserver& operator=(_unused const AObserver& other) {return *this;};
        AObserver& operator=(AObserver&& other)
        {
            this->unlink();
            this->_id = other._id;
            this->_instance = other._instance;
            other._id = 0;
            return *this;
        };

        // ---------- Constructor --------- //
        AObserver() {this->link();};
        AObserver(_unused const AObserver& other) {this->link();};
        AObserver(AObserver&& other): _id{other._id}, _instance{other._instance} {other._id = 0;};

        // ----------- Destructor --------- //
        ~AObserver() {this->unlink();};
};

} // namespace end
#endif /* AOBSERVER_H */
