/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/07/2026 by @author Tsukini

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
    #include "../attribute/Attribute.hpp"               // _nodiscard, _unused
    #include "../warning/Observer.hpp"                  // utils::warning::Observer
    #include "../exception/basic/WarningException.hpp"  // utils::exception::WarningException
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::* (Type)
    #include <unordered_map>                            // std::unordered_map
    #include <iostream>                                 // std::cout, std::endl
    #include <vector>                                   // std::vector
    
    //----------------------------------------------------------------//
    /* MACRO */

    /* hash & equal handling */
    #define BLT_TYPE(T) T, T, std::hash<T>, std::hash<T>, std::equal_to<T>, std::equal_to<T>

namespace utils::blt { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<
    typename T,
    typename Hash,
    typename Equal
>
class BidirectionalLookupTable<T, T, Hash, Hash, Equal, Equal>: private utils::warning::Observer {
    private:
        bool _freezed = false; // Freeze the data edition (can't be disabled after being enable)
        std::unordered_map<T, T, Hash, Equal> _table;

    public:
        // ------------ Function ---------- //
        void freeze(void) {this->_freezed = true;};
        _nodiscard bool isFreezed(void) {return this->_freezed;};
        void clear(void)
        {
            if (this->_freezed) throw utils::exception::ErrorException(utils::exception::Code::Freezed);
            this->_table.clear();
        };
        void removeElement(const T& element) noexcept
        {
            if (this->_freezed) throw utils::exception::ErrorException(utils::exception::Code::Freezed);
            if (!this->_table.contains(element)) {
                utils::exception::WarningException e(utils::exception::Code::UnknowKey);
                std::cout << e.formated() << std::endl;
                return;
            }
            this->_table.erase(element);
        };
        void removeElements(const std::vector<T>& elements) noexcept {for (const T& element: elements) this->removeElement(element);};
        void addElement(const T& left, const T& right) {this->setElement(left, right);};
        template<bool force = false> // Can't override an exiting one by default, throw of error
        void setElement(const T& left, const T& right)
        {
            if (this->_freezed) throw utils::exception::ErrorException(utils::exception::Code::Freezed);
            if constexpr (!force) {
                if (this->_table.contains(left) || this->_table.contains(right))
                    throw utils::exception::ErrorException(utils::exception::Code::Override, "The override is disabled for the BidirectionalLookupTable");
            }
            this->_table[left] = right;
            this->_table[right] = left;
        };

        // ------------ Operator ---------- //
        BidirectionalLookupTable& operator=(const BidirectionalLookupTable& other) = delete;
        BidirectionalLookupTable& operator=(BidirectionalLookupTable&& other) = delete;
        const T& operator[](const T& element) const
        {
            if (!this->_table.contains(element))
                throw utils::exception::ErrorException(utils::exception::Code::UnknowKey);
            return this->_table.at(element);
        }

        // ---------- Constructor --------- //
        BidirectionalLookupTable() = default;
        BidirectionalLookupTable(const BidirectionalLookupTable& other) = delete;
        BidirectionalLookupTable(BidirectionalLookupTable&& other) = delete;

        // ----------- Destructor --------- //
        ~BidirectionalLookupTable() = default;
};

} // namespace end
#endif /* BIDIRECTIONALLOOKUPTABLE_T_H */
