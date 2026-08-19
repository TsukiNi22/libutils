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
##  @file Cluster.hpp

File Description:
##  Cluster class definition
\**************************************************************/

#ifndef CLUSTER_H
    #define CLUSTER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::Type, utils::exception::InternalCode
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "../attribute/Attribute.hpp"               // _cold, _hot, _nodiscard
    #include <functional>                               // std::function
    #include <cstddef>                                  // std::size_t
    #include <vector>                                   // std::vector

namespace utils::pool { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class Cluster {
    private:
        std::vector<T> _class;

    public:
        // ------------ Function ---------- //
        _cold _nodiscard inline std::size_t size(void) const {return this->_class.size();};
        _hot void apply(std::function<void(T&)> fn) {for (auto& c: this->_class) fn(c);}

        /* spawn */
        template <typename... Args>
        _hot inline void spawn(Args&&... args) // spawn one
        {this->_class.emplace_back(std::forward<Args>(args)...);}
        template <typename... Args>
        _hot void spawn(std::size_t n, Args&&... args) // spawn n
        {
            this->_class.reserve(this->_class.size() + n);
            for (std::size_t i = 0; i < n; ++i)
                this->_class.emplace_back(std::forward<Args>(args)...);
        }

        /* kill */
        _hot inline void kill() // kill all
        {this->_class.clear();}
        _hot void kill(std::size_t n) // kill n last
        {
            if (n > this->_class.size()) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds);
            }
            this->_class.erase(this->_class.end() - static_cast<std::ptrdiff_t>(n), this->_class.end());
        }

        // ------------ Operator ---------- //
        Cluster& operator=(const Cluster& other) = delete;
        Cluster& operator=(Cluster&& other) = delete;

        // ---------- Constructor --------- //
        Cluster() = default;
        Cluster(const Cluster& other) = delete;
        Cluster(Cluster&& other) = delete;

        // ----------- Destructor --------- //
        ~Cluster() = default;
};

} // namespace end
#endif /* CLUSTER_H */
