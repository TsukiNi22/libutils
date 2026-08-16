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
##  @file MemoryLeakNotifier.cpp

File Description:
##  Declaration of the memory leak notifier destructor
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/security/observer/MemoryLeakNotifier.hpp"
#include <iostream>
#include <sstream>

_cold void utils::security::observer::MemoryLeakNotifier::trigger(void)
{
    // Check for instance existance
    if (this->_links.empty()) return;

    // Build the warning message
    std::ostringstream oss;
    oss << "[WARNING] Memory leak detected (origin: " << this->_origin << ")" << std::endl;
    oss << "-- At least one instance wasn't properly closed --" << std::endl;
    for (const auto& [id, instance]: this->_links)
        oss << "  " << id << " - " << instance << std::endl;

    // Display warning
    std::cerr << oss.str();
}
