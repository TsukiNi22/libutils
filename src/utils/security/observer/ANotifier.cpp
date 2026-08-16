/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 16/08/2026 by @author Tsukini

File Name:
##  @file ANotifier.cpp

File Description:
##  Declaration of the notifier init
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/basic/WarningException.hpp"
#include "utils/security/observer/Instances.hpp"
#include "utils/security/observer/ANotifier.hpp"
#include <dlfcn.h>
#include <string_view>
#include <cstdint>

// Used to locate the code
_hidden static void fn(void) {/* Nothing */};

_hot utils::security::observer::ANotifier::ANotifier() noexcept
{
    Dl_info info{};

    // Get data on the actual module
    if (dladdr((void*)fn, &info) == 0) _unlikely {return;}

    // Check if the name was succefully getted
    if (info.dli_fname) _likely {
        this->_origin = info.dli_fname;
    }
}

_hot void utils::security::observer::ANotifier::link(const std::uint64_t id, std::string_view instance, const bool safe_mode)
{
    // Lock the internal edition
    std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
    if (safe_mode) lock.lock();
    else (void)lock.try_lock();

    // Minimal check on the id
    if (id == 0) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidId, "Can't link the id: 0");   
    } else if (this->_links.contains(id)) _unlikely {
        utils::exception::WarningException e(utils::exception::InternalCode::InvalidId, "Can't relink the id: " + std::to_string(id));
        std::cerr << e.formated() << std::endl;
        return;
    }

    // Store the new instance
    this->_links[id] = instance;

    // Internal sub-call
    if (this->hasLinkOverload()) {this->link_(id, instance, safe_mode);}
}

_hot void utils::security::observer::ANotifier::unlink(const std::uint64_t id, const bool safe_mode)
{
    // Lock the internal edition
    std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
    if (safe_mode) lock.lock();
    else (void)lock.try_lock();

    // Minimal check on the id
    if (id == 0) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidId, "Can't link the id: 0");   
    } else if (!this->_links.contains(id)) _unlikely {
        utils::exception::WarningException e(utils::exception::InternalCode::UnknowId, std::to_string(id));
        std::cerr << e.formated() << std::endl;
        return;
    }

    // Remove the id from storage
    this->_links.erase(id);

    // Internal sub-call
    if (this->hasUnlinkOverload()) {this->unlink_(id, safe_mode);}
}

_cold void utils::security::observer::ANotifier::clear(const bool safe_mode)
{
    // Lock the internal edition
    std::unique_lock<std::mutex> lock(this->_lock, std::defer_lock);
    if (safe_mode) lock.lock();
    else (void)lock.try_lock();

    // Free all the link
    std::uint64_t subId = 0;
    for (const auto& [id, _]: this->_links) {
        utils::security::observer::instances::IdHandler.free((subId = id), safe_mode);
    }
    this->_links.clear();

    // Internal sub-call
    if (this->hasClearOverload()) {this->clear_(safe_mode);}
}
