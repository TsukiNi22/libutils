/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 25/07/2026 by @author Tsukini

File Name:
##  @file BidirectionalLookupTable.hpp

File Description:
##  Class used for a bidirectional lookup table
\**************************************************************/

#ifndef BIDIRECTIONALLOOKUPTABLE_T_T_H
    #define BIDIRECTIONALLOOKUPTABLE_T_T_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"               // _nodiscard, _unused
    #include "../warning/Observer.hpp"                  // utils::warning::Observer
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "../exception/basic/WarningException.hpp"  // utils::exception::WarningException
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::* (Type)
    #include <unordered_map>                            // std::unordered_map
    #include <iostream>                                 // std::cerr, std::endl
    #include <vector>                                   // std::vector

namespace utils::blt { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<
    typename L,
    typename R,
    typename HashL = std::hash<L>,
    typename HashR = std::hash<R>,
    typename EqualL = std::equal_to<L>,
    typename EqualR = std::equal_to<R>
>
class BidirectionalLookupTable: private utils::warning::Observer {
    private:
        bool _freezed = false; // Freeze the data edition (can't be disabled after being enable)
        std::unordered_map<L, R, HashL, EqualL> _left;
        std::unordered_map<R, L, HashR, EqualR> _right;

    public:
        // ------------ Function ---------- //
        void freeze(void) {this->_freezed = true;};
        _nodiscard bool isFreezed(void) {return this->_freezed;};
        void clear(void)
        {
            if (this->_freezed) throw utils::exception::ErrorException(utils::exception::Code::Freezed);
            this->_left.clear();
            this->_right.clear();
        };
        void removeElement(const L& left)
        {
            if (this->_freezed) throw utils::exception::ErrorException(utils::exception::Code::Freezed);
            if (!this->_left.contains(left)) {
                utils::exception::WarningException e(utils::exception::Code::UnknowKey);
                std::cerr << e.formated() << std::endl;
                return;
            }
            this->_right.erase(this->_left[left]);
            this->_left.erase(left);
        };
        void removeElements(const std::vector<L>& lefts) {for (const L& left: lefts) this->removeElement(left);};
        void removeElement(const R& right)
        {
            if (this->_freezed) throw utils::exception::ErrorException(utils::exception::Code::Freezed);
            if (!this->_right.contains(right)) {
                utils::exception::WarningException e(utils::exception::Code::UnknowKey);
                std::cerr << e.formated() << std::endl;
                return;
            }
            this->_left.erase(this->_right[right]);
            this->_right.erase(right);
        };
        void removeElements(const std::vector<R>& rights) {for (const R& right: rights) this->removeElement(right);};
        void addElement(const L& left, const R& right) {this->setElement(left, right);};
        void addElement(const R& right, const L& left) {this->setElement(right, left);};
        template<bool force = false> // Can't override an exiting one by default, throw of error
        void setElement(const L& left, const R& right)
        {
            if (this->_freezed) throw utils::exception::ErrorException(utils::exception::Code::Freezed);
            if constexpr (!force) {
                if (this->_left.contains(left) || this->_right.contains(right))
                    throw utils::exception::ErrorException(utils::exception::Code::Override, "The override is disabled for the BidirectionalLookupTable");
            }
            this->_left[left] = right;
            this->_right[right] = left;
        };
        template<bool force = false> // Can't override an exiting one by default, throw of error
        void setElement(const R& right, const L& left)
        {
            if (this->_freezed) throw utils::exception::ErrorException(utils::exception::Code::Freezed);
            if constexpr (!force) {
                if (this->_right.contains(right) || this->_left.contains(left))
                    throw utils::exception::ErrorException(utils::exception::Code::Override, "The override is disabled for the BidirectionalLookupTable");
            }
            this->_right[right] = left;
            this->_left[left] = right;
        };

        // ------------ Operator ---------- //
        BidirectionalLookupTable& operator=(const BidirectionalLookupTable& other) = delete;
        BidirectionalLookupTable& operator=(BidirectionalLookupTable&& other) = delete;
        const R& operator[](const L& left) const
        {
            if (!this->_left.contains(left))
                throw utils::exception::ErrorException(utils::exception::Code::UnknowKey);
            return this->_left.at(left);
        }
        const L& operator[](const R& right) const
        {
            if (!this->_right.contains(right))
                throw utils::exception::ErrorException(utils::exception::Code::UnknowKey);
            return this->_right.at(right);
        }

        // ---------- Constructor --------- //
        BidirectionalLookupTable() = default;
        BidirectionalLookupTable(const BidirectionalLookupTable& other) = delete;
        BidirectionalLookupTable(BidirectionalLookupTable&& other) = delete;

        // ----------- Destructor --------- //
        ~BidirectionalLookupTable() = default;
};

} // namespace end
#endif /* BIDIRECTIONALLOOKUPTABLE_T_T_H */
