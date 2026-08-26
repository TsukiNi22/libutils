/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 26/08/2026 by @author Tsukini

File Name:
##  @file Scheduler.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/system/Scheduler.hpp"
#include <cstddef>
#include <mutex>

_hot void utils::system::Scheduler::clear(void)
{
    // Doesn't do anything if there is nothing to clear
    if (this->_finished.empty()) return;

    std::lock_guard lock(this->_lock);
    for (std::size_t id: this->_finished) this->cancel_(id);
    this->_finished.clear();
};

_cold void utils::system::Scheduler::cancel(std::size_t id)
{
    this->_tasks.clear();
    if (!this->_tasks.contains(id)) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::UnknowId, std::to_string(id));
    }
    this->_tasks.erase(id);
    this->_idHandler.free(id);
}

