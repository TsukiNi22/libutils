/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 27/11/2026 by @author Tsukini

File Name:
##  @file Sanitizer.hpp

File Description:
##  AddressSanitizer of the tests: detection of the Asan mode & RAII disabler of LeakSanitizer for the leaks made on purpose
\**************************************************************/

#ifndef SANITIZER_H
    #define SANITIZER_H

    //----------------------------------------------------------------//
    /* DEFINE */

    // AddressSanitizer build (Asan mode): __SANITIZE_ADDRESS__ with gcc, __has_feature with clang
    #if defined(__SANITIZE_ADDRESS__)
        #define TESTS_ASAN
    #elif defined(__has_feature)
        #if __has_feature(address_sanitizer)
            #define TESTS_ASAN
        #endif
    #endif

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* sanitizer */
    #ifdef TESTS_ASAN
        #include <sanitizer/lsan_interface.h>   // __lsan_disable, __lsan_enable
    #endif

namespace tests::tools { // namespace start

//----------------------------------------------------------------//
/* CLASS */

// The allocations made during its life are not reported by LeakSanitizer (a leak made on purpose), nothing without Asan
class ScopedLeak {
    public:
        // ---------- Constructor --------- //
        #ifdef TESTS_ASAN
            ScopedLeak(void) {__lsan_disable();};
        #else
            ScopedLeak(void) { /* Nothing */ };
        #endif

        ScopedLeak(const ScopedLeak& other) = delete;

        ScopedLeak& operator=(const ScopedLeak& other) = delete;

        // ----------- Destructor --------- //
        #ifdef TESTS_ASAN
            ~ScopedLeak() {__lsan_enable();};
        #else
            ~ScopedLeak() { /* Nothing */ };
        #endif
};

} // namespace end

#endif /* SANITIZER_H */
