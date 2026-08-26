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
##  @file BidirectionalLookupTable.hpp

File Description:
##  Class used for a bidirectional lookup table specialized for one type
\**************************************************************/

#ifndef BIDIRECTIONALLOOKUPTABLE_T_H
    #define BIDIRECTIONALLOOKUPTABLE_T_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"                // _nodiscard, _unused
    #include "../../security/observer/Observer.hpp"         // utils::security::observer::Observer
    #include "../../exception/basic/WarningException.hpp"   // utils::exception::WarningException
    #include "../../exception/basic/ErrorException.hpp"     // utils::exception::ErrorException
    #include "../../exception/ExceptionDefine.hpp"          // utils::exception::* (Type)
    #include "../../type/Freezable.hpp"                     // utils::type::Freezable
    #include <unordered_map>                                // std::unordered_map
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
        void clear(void)
        {
            this->requireFrozen();
            this->_table.clear();
        };
        void removeElement(const T& element) noexcept
        {
            this->requireFrozen();
            if (!this->_table.contains(element)) {
                utils::exception::WarningException e(utils::exception::InternalCode::UnknowKey);
                std::cerr << e.formated() << std::endl;
                return;
            }
            this->_table.erase(element);
        };
        void removeElements(const std::vector<T>& elements) noexcept {for (const T& element: elements) this->removeElement(element);};
        void addElement(const T& left, const T& right) {this->setElement(left, right);};
        template<bool force = false> // Can't override an exiting one by default, throw of error
        void setElement(const T& left, const T& right)
        {
            this->requireFrozen();
            if constexpr (!force) {
                if (this->_table.contains(left) || this->_table.contains(right))
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Override, "The override is disabled for the BidirectionalLookupTable");
            }
            this->_table[left] = right;
            this->_table[right] = left;
        };

        // ------------ Operator ---------- //
        BidirectionalLookupTable& operator=(const BidirectionalLookupTable& other) = delete;
        BidirectionalLookupTable& operator=(BidirectionalLookupTable&& other) = default;
        const T& operator[](const T& element) const
        {
            if (!this->_table.contains(element))
                throw utils::exception::ErrorException(utils::exception::InternalCode::UnknowKey);
            return this->_table.at(element);
        }

        // ---------- Constructor --------- //
        BidirectionalLookupTable() = default;
        BidirectionalLookupTable(const BidirectionalLookupTable& other) = delete;
        BidirectionalLookupTable(BidirectionalLookupTable&& other) = default;

        // ----------- Destructor --------- //
        ~BidirectionalLookupTable() = default;
};

} // namespace end
#endif /* BIDIRECTIONALLOOKUPTABLE_T_H */
