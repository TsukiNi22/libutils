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
##  @file Dlfcn.cpp

File Description:
##  Definition of the dynamic loading calls of Dlfcn.hpp (Unix: system, Windows: Win32)
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/platform/Platform.hpp"
#include "utils/platform/Dlfcn.hpp"
#ifdef UTILS_WINDOWS
    #include "Windows.hpp"
#else
    #include <dlfcn.h>
#endif
#include <string>

#ifdef UTILS_WINDOWS
    // Last error of the thread (dlerror), empty = none
    static thread_local std::string last;
    static thread_local std::string returned;

    static void set_error(const std::string& prefix)
    {
        last = prefix + ": " + utils::platform::windows::strerror(::GetLastError());
    }
#endif

_cold _nodiscard void* utils::platform::dlopen(const char* path, _unused int flags)
{
    #ifdef UTILS_WINDOWS
        HMODULE module = ::LoadLibraryA(path);
        if (!module) _unlikely set_error(path ? path : "[null]");
        return reinterpret_cast<void*>(module);
    #else
        return ::dlopen(path, flags);
    #endif
}

_hot _nodiscard void* utils::platform::dlsym(void* handle, const char* name)
{
    #ifdef UTILS_WINDOWS
        void* fn = reinterpret_cast<void*>(::GetProcAddress(reinterpret_cast<HMODULE>(handle), name));
        if (!fn) _unlikely set_error(name);
        return fn;
    #else
        return ::dlsym(handle, name);
    #endif
}

_cold int utils::platform::dlclose(void* handle)
{
    #ifdef UTILS_WINDOWS
        if (!::FreeLibrary(reinterpret_cast<HMODULE>(handle))) _unlikely {
            set_error("FreeLibrary");
            return -1;
        }
        return 0;
    #else
        return ::dlclose(handle);
    #endif
}

_cold _nodiscard char* utils::platform::dlerror(void)
{
    #ifdef UTILS_WINDOWS
        if (last.empty()) return nullptr;
        returned = last; // valid until the next dlerror of the thread
        last.clear();
        return returned.data();
    #else
        return ::dlerror();
    #endif
}

_cold _nodiscard int utils::platform::dladdr(const void* addr, Dl_info* info)
{
    #ifdef UTILS_WINDOWS
        static thread_local char path[MAX_PATH];
        HMODULE module = nullptr;

        // Module containing the address (without changing its reference count)
        if (!addr || !info || !::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            static_cast<LPCSTR>(addr), &module)) _unlikely return 0;
        const DWORD len = ::GetModuleFileNameA(module, path, MAX_PATH);
        if (len == 0 || len >= MAX_PATH) _unlikely return 0;
        info->dli_fname = path;
        info->dli_fbase = reinterpret_cast<void*>(module);
        info->dli_sname = nullptr;
        info->dli_saddr = nullptr;
        return 1;
    #else
        return ::dladdr(addr, info);
    #endif
}
