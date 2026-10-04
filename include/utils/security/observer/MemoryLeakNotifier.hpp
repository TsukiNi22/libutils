/**************************************************************\
Edition:
##  @date 16/08/2026 by @author Tsukini

File Name:
##  @file MemoryLeakNotifier.hpp

File Description:
##  Notifier for meamory leak
\**************************************************************/

#ifndef MEMORYLEAKNOTIFIER_H
    #define MEMORYLEAKNOTIFIER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "ANotifier.hpp"    // utils::security::observer::ANotifier

namespace utils::security::observer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class MemoryLeakNotifier: public utils::security::observer::ANotifier {
    public:
        // ---------- Pre-Function -------- //
        void trigger(void) final;

        // ------------ Operator ---------- //
        MemoryLeakNotifier& operator=(const MemoryLeakNotifier& other) = delete;
        MemoryLeakNotifier& operator=(MemoryLeakNotifier&& other) = delete;

        // ---------- Constructor --------- //
        MemoryLeakNotifier() = default;
        MemoryLeakNotifier(const MemoryLeakNotifier& other) = delete;
        MemoryLeakNotifier(MemoryLeakNotifier&& other) = delete;

        // ----------- Destructor --------- //
        ~MemoryLeakNotifier() noexcept {this->trigger();};
};

} // namespace end
#endif /* MEMORYLEAKNOTIFIER_H */
