/**************************************************************\
Edition:
##  @date 06/09/2026 by @author Tsukini

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
    #include "../../attribute/Attribute.hpp"        // _hot, _unused
    #include "../../manip/smanip/FixedString.hpp"   // utils::smanip::FixedString
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

template<utils::smanip::FixedString instance = "[unknown]", bool safe_mode = true>
class AObserver: public utils::security::observer::IObserver {
    private:
        // id = 0 is reserved for unattribued one
        std::uint64_t _id = 0; // up to 2^64 - 1 item at the same time
        std::string_view _instance = instance.view();

        // ------------ Function ---------- //
        _hot void link_(void) override
        {
            utils::security::observer::instances::id_handler().allocate(this->_id, safe_mode);
            for (std::unique_ptr<utils::security::observer::INotifier>& notifier: utils::security::observer::instances::notifiers())
                notifier->link(this->_id, this->_instance, safe_mode);
        };
        _hot void unlink_(void) override
        {
            if (this->_id == 0) return; // Ignore thoese who where already realese/transfered
            for (std::unique_ptr<utils::security::observer::INotifier>& notifier: utils::security::observer::instances::notifiers())
                notifier->unlink(this->_id, safe_mode);
            utils::security::observer::instances::id_handler().free(this->_id, safe_mode);
        };

    public:
        // ------------ Operator ---------- //
        AObserver& operator=(_unused const AObserver& other) {return *this;};
        AObserver& operator=(AObserver&& other)
        {
            this->unlink_();
            this->_id = other._id;
            this->_instance = other._instance;
            other._id = 0;
            return *this;
        };

        // ---------- Constructor --------- //
        AObserver()                               {this->link_();};
        AObserver(_unused const AObserver& other) {this->link_();};
        AObserver(AObserver&& other): _id{other._id}, _instance{other._instance} {other._id = 0;};

        // ----------- Destructor --------- //
        ~AObserver() {this->unlink_();};
};

} // namespace end
#endif /* AOBSERVER_H */
