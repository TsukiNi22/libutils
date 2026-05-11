/**************************************************************\
Edition:
##  @date 27/04/2026 by @author Tsukini

File Name:
##  @file SharedObject.cpp

File Description:
##  SharedObject methods and static declaration to intercept dl*
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/warning/SharedObject.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/custom/CustomException.hpp"
#include <dlfcn.h>
#include <iostream>
#include <cstdint>
#include <format>
#include <string>
#include <mutex>

// Global warning instance
utils::warning::SharedObject utils::warning::WarningInstance::SharedObject;

// Used to locate the code
static void fn(void) {/* Nothing */};

nodiscard std::string utils::warning::SharedObject::getOrigin(void)
{
    Dl_info info{};

    // Get data on the isSharedObject function
    if (dladdr((void*)fn, &info) == 0) unlikely {
        return "";
    }

    // Check if the orign file name was succefully getted
    if (!info.dli_fname) unlikely {
        return "";
    } else likely {
        return info.dli_fname;
    }
}

nodiscard bool utils::warning::SharedObject::isSharedObject(void)
{
    // Try to get the orign
    std::string path = this->getOrigin();
    if (path.empty()) return false;
    return path.ends_with(".so");
}

void utils::warning::SharedObject::link(const std::string& InstanceName, std::uint32_t& id, bool safe)
{
    if (!this->_isSharedObject) return;
    std::unique_lock<std::mutex> lock(this->_mutex, std::defer_lock);
    if (safe) lock.lock();
    else (void)lock.try_lock();

    // If id where free
    if (this->_availableId.size() > 0) {
        id = this->_availableId.back();
        this->_links[id] = InstanceName;
        this->_availableId.pop_back();
    }

    // No id available
    else {
        // Check for overflow
        if (this->_nextId == UINT32_MAX) unlikely {
            throw utils::exception::ErrorException(utils::exception::Code::IdOverflow);
        }
        id = ++(this->_nextId);
        this->_links[id] = InstanceName;
    }

    // Check the attribution
    if (id == 0)
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::UnknowId, "Can't attribute the id: 0");
}

void utils::warning::SharedObject::unlink(std::uint32_t id, bool safe)
{
    if (!this->_isSharedObject) return;
    std::unique_lock<std::mutex> lock(this->_mutex, std::defer_lock);
    if (safe) lock.lock();
    else (void)lock.try_lock();

    // Basic check for the id validity
    if (id == 0)
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::UnknowId, "Can't unlink an invalide id: 0");

    // Check the id existance
    if (!this->_links.contains(id)) unlikely {
        utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::UnknowId, std::to_string(id));
        std::cerr << e.formated() << std::endl;
        return;
    }

    // Free the id
    this->_links.erase(id);
    this->_availableId.push_back(id);
}

utils::warning::SharedObject::SharedObject(void) noexcept
: _isSharedObject{utils::warning::WarningInstance::SharedObject.isSharedObject()},
    _origin{this->getOrigin()}
{}

utils::warning::SharedObject::~SharedObject() noexcept
{
    if (!this->_isSharedObject) return;

    // Check for instance existance
    if (this->_links.empty()) return;
    std::cerr << "SharedObject leak detected: " << this->_origin << std::endl;
    
    // Build the warning message
    std::string message = "At least one instance wasn't properly closed before freeing the dynamic loaded code:";
    for (const auto& [id, InstanceName]: this->_links)
        message += std::format("\n{} - {}", id, InstanceName);

    // Display warning
    //utils::exception::CustomException warning(utils::exception::Type::Warning, utils::exception::Code::SharedObject, message);
    //std::cerr << warning.formated() << std::endl;
    std::cerr << message << std::endl;
}
