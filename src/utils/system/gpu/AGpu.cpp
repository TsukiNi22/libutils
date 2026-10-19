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
##  @file AGpu.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils/encapsulation/SharedObject.hpp"
#include "utils/exception/IException.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/system/gpu/AGpu.hpp"
#include <memory>
#include <vector>
#include <string>

_cold bool utils::system::AGpu::load_(const std::vector<std::string>& libraries)
{
    for (const std::string& library: libraries) {
        try {
            this->_library = std::make_unique<utils::encapsulation::SharedObject>(library);
            return true;
        } catch (const utils::exception::IException&) {
            // Not an error: the next name of the library is tried
            continue;
        }
    }
    return false;
}
