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
##  @file Instances.cpp

File Description:
##  Declaration fo the physic version for the observer statique instances
\**************************************************************/

#include "utils/security/observer/Instances.hpp"
#include "utils/security/observer/INotifier.hpp"
#include "utils/security/observer/MemoryLeakNotifier.hpp"
#include "utils/system/IdHandler.hpp"
#include <cstdint>
#include <memory>
#include <array>

/* id distributor */
utils::system::IdHandler<std::uint64_t> utils::security::observer::instances::IdHandler;

/* different notifiers to link/unlink */
std::array<std::unique_ptr<utils::security::observer::INotifier>, 1> utils::security::observer::instances::Notifiers = {
    std::make_unique<utils::security::observer::MemoryLeakNotifier>(),
};
