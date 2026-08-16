/**************************************************************\
Edition:
##  @date 16/08/2026 by @author Tsukini

File Name:
##  @file INotifier.hpp

File Description:
##  Interface of the different notifiers
\**************************************************************/

#ifndef INOTIFIER_H
    #define INOTIFIER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <string_view>  // std::string_view
    #include <cstdint>      // std::uint64_t

namespace utils::security::observer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class INotifier {
    protected:
        // ---------- Pre-Function -------- //
        virtual void link_(const std::uint64_t id, std::string_view instance, const bool safe_mode) = 0;
        virtual void unlink_(const std::uint64_t id, const bool safe_mode) = 0;
        virtual void clear_(const bool safe_mode) = 0;
        virtual bool hasLinkOverload(void) const = 0;
        virtual bool hasUnlinkOverload(void) const = 0;
        virtual bool hasClearOverload(void) const = 0;

    public:
        // ---------- Pre-Function -------- //
        virtual void link(const std::uint64_t id, std::string_view instance, const bool safe_mode) = 0;
        virtual void unlink(const std::uint64_t id, const bool safe_mode) = 0;

        // !!! Should only be used if all the linked instances where vanished from existance !!!
        virtual void clear(const bool safe_mode) = 0; // Free the stored if from the handler

        // Auto call by the notifiers in normal case but can be forced for test or other case
        virtual void trigger(void) = 0; // Force notifiers to check there internal status and trigger there 'notification'

        // ------------ Operator ---------- //
        INotifier& operator=(const INotifier& other) = delete;
        INotifier& operator=(INotifier&& other) = delete;

        // ---------- Constructor --------- //
        INotifier() = default;
        INotifier(const INotifier& other) = delete;
        INotifier(INotifier&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~INotifier() = default;
};

} // namespace end
#endif /* INOTIFIER_H */
