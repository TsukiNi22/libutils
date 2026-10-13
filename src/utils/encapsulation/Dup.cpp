/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 15/09/2026 by @author Tsukini

File Name:
##  @file Dup.cpp

File Description:
##  Definition of the Dup's methods
\**************************************************************/

#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/encapsulation/Dup.hpp"
#include "utils/platform/Unistd.hpp"
#include <cstring>
#include <cerrno>

_cold void utils::encapsulation::Dup::trigger(void)
{
    if (this->_origin == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Dup, "The origin fd is close, please set a valid fd: setOrigin(int fd)");
    }

    // Dispatch
    if (this->_clone == -1 && (this->_clone = utils::platform::dup(this->_origin)) == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Dup, utils::platform::strerror(errno));
    } else if (utils::platform::dup2(this->_origin, this->_clone) == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Dup, utils::platform::strerror(errno));
    }
}
