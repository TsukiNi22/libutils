/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/08/2026 by @author Tsukini

File Name:
##  @file Dup2.cpp

File Description:
##  Definition of the Dup2's methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/encapsulation/Dup2.hpp"
#include <unistd.h>
#include <cstring>
#include <cerrno>

_cold void utils::encapsulation::Dup2::trigger(void)
{
    if (this->_origin == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Dup2, "The origin fd is close, please set a valid fd: setClone(int fd)");
    }

    // Dispatch
    if (this->_clone == -1 && (this->_clone = ::dup(this->_origin)) == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Dup2, ::strerror(errno));
    } else if (::dup2(this->_origin, this->_clone) == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Dup2, ::strerror(errno));
    }
}
