/**************************************************************\
Edition:
##  @date 20/08/2026 by @author Tsukini

File Name:
##  @file ANotifier.hpp

File Description:
##  Abstract of the different notifiers
\**************************************************************/

#ifndef ANOTIFIER_H
    #define ANOTIFIER_H

    //----------------------------------------------------------------//
    /* ANCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"                // _cold, _nodiscard, _unused
    #include "../../exception/ExceptionDefine.hpp"          // utils::exception::InternalCode
    #include "../../exception/custom/FatalException.hpp"    // utils::exception::FatalException
    #include "INotifier.hpp"                                // utils::security::observer::INotifier
    #include <unordered_map>                                // std::unordered_map
    #include <string_view>                                  // std::string_view
    #include <cstdint>                                      // std::uint64_t
    #include <string>                                       // std::string
    #include <mutex>                                        // std::mutex

namespace utils::security::observer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class ANotifier: public utils::security::observer::INotifier {
    protected:
        std::mutex _lock; // Handling of multithreading
        std::string _origin = "[unknown]"; // module path/name
        std::unordered_map<std::uint64_t, std::string> _links;

        // ---------- Pre-Function -------- //
        void link_(_unused const std::uint64_t id, _unused std::string_view instance, _unused const bool safe_mode) override
        {throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall);};
        void unlink_(_unused const std::uint64_t id, _unused const bool safe_mode) override
        {throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall);};
        void clear_(_unused const bool safe_mode) override
        {throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall);};
        _hot _nodiscard bool hasLinkOverload(void) const override {return false;};
        _hot _nodiscard bool hasUnlinkOverload(void) const override {return false;};
        _hot _nodiscard bool hasClearOverload(void) const override {return false;};

    public:
        // ---------- Pre-Function -------- //
        void link(const std::uint64_t id, std::string_view instance, const bool safe_mode) final;
        void unlink(const std::uint64_t id, const bool safe_mode) final;
    
        // !!! Should only be used if all the linked instances where vanished from existance !!!
        void clear(const bool safe_mode) final;

        // ------------ Operator ---------- //
        ANotifier& operator=(const ANotifier& other) = delete;
        ANotifier& operator=(ANotifier&& other) = delete;

        // ---------- Constructor --------- //
        ANotifier() noexcept;
        ANotifier(const ANotifier& other) = delete;
        ANotifier(ANotifier&& other) = delete;

        // ----------- Destructor --------- //
        ~ANotifier() = default;
};

} // namespace end
#endif /* ANOTIFIER_H */
