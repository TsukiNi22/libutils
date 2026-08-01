/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 01/08/2026 by @author Tsukini

File Name:
##  @file Instances.hpp

File Description:
##  Different static instance used by the observer
\**************************************************************/

#ifndef INSTANCES_H
    #define INSTANCES_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../IdHandler.hpp" // utils::security::IdHandler
    #include "INotifier.hpp"    // utils::security::observer::INotifier
    #include <cstdint>          // std::uint64_t
    #include <memory>           // std::unique_ptr, std::make_unique
    #include <array>            // std::array

namespace utils::security::observer::instances { // namespace start
//----------------------------------------------------------------//
/* CLASS */

/* id distributor */
extern utils::security::IdHandler<std::uint64_t> IdHandler;

/* different notifiers to link/unlink */
extern std::array<std::unique_ptr<utils::security::observer::INotifier>, 1> Notifiers;

} // namespace end
#endif /* INSTANCES_H */
