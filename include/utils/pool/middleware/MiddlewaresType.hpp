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
##  @file MiddlewaresType.hpp

File Description:
##  Declaration of the Middleware type for void & non void function
\**************************************************************/

#ifndef MIDDLEWARESTYPE_H
    #define MIDDLEWARESTYPE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <functional>   // std::function

namespace utils::pool { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

template<typename T>
struct MiddlewareType {
    using type = std::function<void(T)>;
};

template<>
struct MiddlewareType<void> {
    using type = std::function<void(void)>;
};

//----------------------------------------------------------------//
/* TYPE */

template<typename T>
using Middleware = typename utils::pool::MiddlewareType<T>::type;

} // namespace end
#endif /* MIDDLEWARESTYPE_H */
