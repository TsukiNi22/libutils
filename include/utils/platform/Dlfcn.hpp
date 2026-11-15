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
    #include "Types.h"                  // RTLD_*, Dl_info (Windows)
    #ifndef UTILS_WINDOWS
        #include <dlfcn.h>              // RTLD_*, Dl_info
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
