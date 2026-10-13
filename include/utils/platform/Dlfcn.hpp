/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 12/10/2026 by @author Tsukini

File Name:
##  @file Dlfcn.hpp

File Description:
##  Dynamic loading calls of <dlfcn.h> for Unix & Windows (same names, same contract)
##  Unix: the system call, Windows: LoadLibrary / GetProcAddress / FreeLibrary
\**************************************************************/

#ifndef PLATFORM_DLFCN_H
    #define PLATFORM_DLFCN_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "Platform.hpp"             // UTILS_WINDOWS
    #ifndef UTILS_WINDOWS
        #include <dlfcn.h>              // RTLD_*, Dl_info
    #endif

    //----------------------------------------------------------------//
    /* DEFINE */

    #ifdef UTILS_WINDOWS
        // Same values as Linux (ignored on Windows: a dll is always resolved at the load)
        #define RTLD_LAZY   0x00001
        #define RTLD_NOW    0x00002
        #define RTLD_GLOBAL 0x00100
        #define RTLD_LOCAL  0
    #endif

//----------------------------------------------------------------//
/* STRUCT */

#ifdef UTILS_WINDOWS
    // Same fields as Linux (<dlfcn.h>)
    struct Dl_info { // xstyle: ignore CPP-CLASS-NAME (POSIX name)
        const char* dli_fname; // path of the module (thread local storage, valid until the next dladdr of the thread)
        void* dli_fbase; // base address of the module
        const char* dli_sname; // always nullptr on Windows
        void* dli_saddr; // always nullptr on Windows
    };
#endif

namespace utils::platform { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

void* dlopen(const char* path, int flags); // nullptr on error (dlerror)
void* dlsym(void* handle, const char* name); // nullptr on error (dlerror)
int dlclose(void* handle); // 0 on success
char* dlerror(void); // last error of the thread, nullptr if none (cleared by the call)
int dladdr(const void* addr, Dl_info* info); // 0 on error

} // namespace end
#endif /* PLATFORM_DLFCN_H */
