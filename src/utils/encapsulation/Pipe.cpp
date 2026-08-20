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
##  @file Pipe.cpp

File Description:
##  Definition of the Pipe's methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/encapsulation/Pipe.hpp"
#include <unistd.h>
#include <cstring>
#include <cerrno>

_cold void utils::encapsulation::Pipe::trigger(void)
{
    if (this->_fds[0] != -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Pipe, "The read fd is already open, close it before a new trigger: closeRead()");
    } else if (this->_fds[1] != -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Pipe, "The write fd is already open, close it before a new trigger: closeWrite()");
    } else if (::pipe(this->_fds) == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Pipe, ::strerror(errno));
    }
}
