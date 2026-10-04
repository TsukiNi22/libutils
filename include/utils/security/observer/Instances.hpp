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
##  @file Instances.hpp

File Description:
##  Different static instance used by the observer
\**************************************************************/

#ifndef INSTANCES_H
    #define INSTANCES_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../system/IdHandler.hpp"   // utils::system::IdHandler
    #include "INotifier.hpp"                // utils::security::observer::INotifier
    #include <cstdint>                      // std::uint64_t
    #include <memory>                       // std::unique_ptr, std::make_unique
    #include <array>                        // std::array

namespace utils::security::observer::instances { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

// Function local statics (constructed on first use)

/* id distributor */
utils::system::IdHandler<std::uint64_t>& id_handler(void);

/* different notifiers to link/unlink */
std::array<std::unique_ptr<utils::security::observer::INotifier>, 1>& notifiers(void);

} // namespace end
#endif /* INSTANCES_H */
