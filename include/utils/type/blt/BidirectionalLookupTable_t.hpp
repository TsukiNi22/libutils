/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 27/08/2026 by @author Tsukini

File Name:
##  @file BidirectionalLookupTable_t.hpp

File Description:
##  Class used for a bidirectional lookup table specialized for one type
\**************************************************************/

#ifndef BIDIRECTIONALLOOKUPTABLET_H
    #define BIDIRECTIONALLOOKUPTABLET_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"                // _cold, _hot, _nodiscard, _unlikely
    #include "../../security/observer/Observer.hpp"         // utils::security::observer::Observer
    #include "../../exception/basic/WarningException.hpp"   // utils::exception::WarningException
    #include "../../exception/basic/ErrorException.hpp"     // utils::exception::ErrorException
    #include "../../exception/ExceptionDefine.hpp"          // utils::exception::* (Type)
    #include "../../type/Freezable.hpp"                     // utils::type::Freezable
    #include "BidirectionalLookupTable_t-t.hpp"             // utils::type::BidirectionalLookupTable<L, R, ...> (primary template)
    #include <unordered_map>                                // std::unordered_map
    #include <functional>                                   // std::hash, std::equal_to
    #include <iostream>                                     // std::cerr, std::endl
    #include <vector>                                       // std::vector

    //----------------------------------------------------------------//
    /* MACRO */

    /* hash & equal handling */
    #define BLT_TYPE(T) T, T, std::hash<T>, std::hash<T>, std::equal_to<T>, std::equal_to<T>

namespace utils::type { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<
    typename T,
    typename Hash,
    typename Equal
>
class BidirectionalLookupTable<T, T, Hash, Hash, Equal, Equal>: public utils::type::Freezable, private utils::security::observer::Observer<"BidirectionalLookupTable"> {
    private:
        std::unordered_map<T, T, Hash, Equal> _table;

    public:
        // ------------ Function ---------- //
        _cold void clear(void)
        {
            this->requireUnfrozen();
            this->_table.clear();
        };
        _cold void removeElement(const T& element) // throw if frozen
        {
            this->requireUnfrozen();
            auto it = this->_table.find(element);
            if (it == this->_table.end()) {
                utils::exception::WarningException e(utils::exception::InternalCode::UnknownKey);
                std::cerr << e.formated() << std::endl;
                return;
            }
            const T other = it->second;
            this->_table.erase(it);
            this->_table.erase(other); // remove both side of the link
        };
        _cold inline void removeElements(const std::vector<T>& elements) {for (const T& element: elements) this->removeElement(element);};
        _cold inline void addElement(const T& left, const T& right)      {this->setElement(left, right);};
        template<bool force = false> // Can't override an exiting one by default, throw of error
        _cold void setElement(const T& left, const T& right)
        {
            this->requireUnfrozen();
            if constexpr (!force) {
                if (this->_table.contains(left) || this->_table.contains(right))
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Override, "The override is disabled for the BidirectionalLookupTable");
            } else {
                // Remove the old pairs to not keep a dangling reverse link
                if (auto it = this->_table.find(left); it != this->_table.end()) {const T other = it->second; this->_table.erase(it); this->_table.erase(other);}
                if (auto it = this->_table.find(right); it != this->_table.end()) {const T other = it->second; this->_table.erase(it); this->_table.erase(other);}
            }
            this->_table[left] = right;
            this->_table[right] = left;
        };

        // ------------ Operator ---------- //
        BidirectionalLookupTable& operator=(const BidirectionalLookupTable& other) = delete;
        BidirectionalLookupTable& operator=(BidirectionalLookupTable&& other) = default;
        _hot _nodiscard const T& operator[](const T& element) const
        {
            if (!this->_table.contains(element)) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::UnknownKey);
            }
            return this->_table.at(element);
        };

        // ---------- Constructor --------- //
        BidirectionalLookupTable() = default;
        BidirectionalLookupTable(const BidirectionalLookupTable& other) = delete;
        BidirectionalLookupTable(BidirectionalLookupTable&& other) = default;

        // ----------- Destructor --------- //
        ~BidirectionalLookupTable() = default;
};

} // namespace end
#endif /* BIDIRECTIONALLOOKUPTABLET_H */
