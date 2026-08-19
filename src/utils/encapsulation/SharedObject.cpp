/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/08/2026 by @author Tsukini

File Name:
##  @file SharedObject.cpp

File Description:
##  Definition of the SharedObject's methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/encapsulation/SharedObject.hpp"
#include <dlfcn.h>
#include <iostream>
#include <string>

_cold utils::encapsulation::SharedObject::SharedObject(const std::string& path)
: _path{path}
{
    this->_lib = ::dlopen(path.c_str(), RTLD_NOW);
    if (!this->_lib) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Dlopen, ::dlerror());
    }
}

_cold utils::encapsulation::SharedObject::~SharedObject() noexcept
{
    if (this->_lib && dlclose(this->_lib) != 0) _unlikely {
        utils::exception::ErrorException e(utils::exception::InternalCode::Dlclose, ::dlerror());
        std::cerr << e.formated() << std::endl;
    }
}
