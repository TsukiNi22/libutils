/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 17/10/2026 by @author Tsukini

File Name:
##  @file AGpu.hpp

File Description:
##  Shared part of the gpu handlers (driver loading, device info)
\**************************************************************/

#ifndef AGPU_H
    #define AGPU_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../encapsulation/SharedObject.hpp" // utils::encapsulation::SharedObject
    #include "../../attribute/Attribute.hpp"        // _cold, _nodiscard
    #include "../../platform/Platform.hpp"          // UTILS_WINDOWS
    #include "IGpu.hpp"                             // utils::system::IGpu
    #include <cstddef>                              // std::size_t
    #include <memory>                               // std::unique_ptr
    #include <vector>                               // std::vector
    #include <string>                               // std::string

    //----------------------------------------------------------------//
    /* DEFINE */

    // Calling convention of the gpu drivers (CUDAAPI, CL_API_CALL, VKAPI_CALL)
    #ifdef UTILS_WINDOWS
        #define GPU_API_CALL __stdcall
    #else
        #define GPU_API_CALL
    #endif

namespace utils::system { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class AGpu: public utils::system::IGpu {
    protected:
        std::unique_ptr<utils::encapsulation::SharedObject> _library = nullptr; // driver / loader of the backend (loaded at runtime)
        bool _available = false; // backend loaded and a device ready
        std::string _name = "[None]"; // name of the device
        std::size_t _memory = 0; // memory of the device in bytes (0 = unknown)

        // ---------- Pre-Function -------- //
        bool load_(const std::vector<std::string>& libraries); // load the first library found (false = none, never throw)

    public:
        // ------------ Function ---------- //
        /* getter */
        _cold _nodiscard bool isAvailable(void) const final      {return this->_available;};
        _cold _nodiscard std::string getName(void) const final   {return this->_name;};
        _cold _nodiscard std::size_t getMemory(void) const final {return this->_memory;};

        // ------------ Operator ---------- //
        AGpu& operator=(const AGpu& other) = delete;
        AGpu& operator=(AGpu&& other) = delete;

        // ---------- Constructor --------- //
        AGpu() = default;
        AGpu(const AGpu& other) = delete;
        AGpu(AGpu&& other) = delete;

        // ----------- Destructor --------- //
        ~AGpu() = default;
};

} // namespace end
#endif /* AGPU_H */
