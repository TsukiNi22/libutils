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
##  @file Middlewares.hpp

File Description:
##  Declaration of the Middleware type for void & non void function
\**************************************************************/

#ifndef MIDDLEWARESTYPE_H
    #define MIDDLEWARESTYPE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <functional>   // std::function

namespace utils::middleware { // namespace
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
struct MiddlewareType {
    using type = std::function<void(T)>;
};

template<>
struct MiddlewareType<void> {
    using type = std::function<void()>;
};

template<typename T>
using Middleware = typename MiddlewareType<T>::type;

} // namespace end
#endif /* MIDDLEWARESTYPE_H */
