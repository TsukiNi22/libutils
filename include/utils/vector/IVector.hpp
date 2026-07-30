/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 18/05/2026 by @author Tsukini

File Name:
##  @file IVector.hpp

File Description:
##  Interface for the cutomized vector
\**************************************************************/

#ifndef IVECTOR_H
    #define IVECTOR_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../security/observer/Observer.hpp"  // utils::security::observer::Observer
    #include <cstddef>                  // std::size_t

namespace utils::vector { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class IVector: private utils::security::observer::Observer {
    public:
        // ---------- Pre-Function -------- //
        virtual T get(std::size_t index) const = 0;

        // ------------ Operator ---------- //
        IVector& operator=(const IVector& other) = default;
        IVector& operator=(IVector&& other) = default;

        // ---------- Constructor --------- //
        IVector() = default;
        IVector(const IVector& other) = default;
        IVector(IVector&& other) = default;

        // ----------- Destructor --------- //
        virtual ~IVector() = default;
};

} // namespace end
#endif /* IVECTOR_H */
