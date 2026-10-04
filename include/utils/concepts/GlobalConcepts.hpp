/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/04/2026 by @author Tsukini

File Name:
##  @file GlobalConcepts.hpp

File Description:
##  Definition of the different global concepts
\**************************************************************/

#ifndef GLOBALCONCEPTS_H
    #define GLOBALCONCEPTS_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <type_traits>  // std::is_convertible_v
    #include <concepts>     // std::convertible_to
    #include <iostream>     // std::ostream
    #include <utility>      // std::swap, std::declval

namespace utils::concepts { // namespace start
//----------------------------------------------------------------//
/* CONCEPTS */

template<typename T, typename U>
concept Convertible = requires(T a, U b) {
    {static_cast<U>(a)} -> std::convertible_to<U>;
};

template<typename T>
concept Swappable = requires(T a, T b) {
    {std::swap(a, b)};
};

template<typename T>
concept Streamable = requires(std::ostream& os, T a) {os << a;};

template<typename T, typename U>
concept ConvertibleTo = std::is_convertible_v<T, U> && requires {
    static_cast<U>(std::declval<T>());
};

} // namespace end

//----------------------------------------------------------------//
/* MIGRATION */
namespace utils::concepts {
    // A concept can't be deprecated: kept until the next major (~v4.0.0)
    template<typename T, typename U>
    concept convertible_to = utils::concepts::ConvertibleTo<T, U>;
}

#endif /* GLOBALCONCEPTS_H */
