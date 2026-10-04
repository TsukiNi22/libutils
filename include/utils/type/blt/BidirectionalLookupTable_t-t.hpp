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
##  @file BidirectionalLookupTable_t-t.hpp

File Description:
##  Class used for a bidirectional lookup table
\**************************************************************/

#ifndef BIDIRECTIONALLOOKUPTABLETT_H
    #define BIDIRECTIONALLOOKUPTABLETT_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"                // _cold, _hot, _nodiscard, _unlikely
    #include "../../security/observer/Observer.hpp"         // utils::security::observer::Observer
    #include "../../exception/basic/WarningException.hpp"   // utils::exception::WarningException
    #include "../../exception/basic/ErrorException.hpp"     // utils::exception::ErrorException
    #include "../../exception/ExceptionDefine.hpp"          // utils::exception::* (Type)
    #include "../../type/Freezable.hpp"                     // utils::type::Freezable
    #include <unordered_map>                                // std::unordered_map
    #include <functional>                                   // std::hash, std::equal_to
    #include <iostream>                                     // std::cerr, std::endl
    #include <vector>                                       // std::vector

namespace utils::type { // namespace start
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
class BidirectionalLookupTable: public utils::type::Freezable, private utils::security::observer::Observer<"BidirectionalLookupTable"> {
    private:
        std::unordered_map<L, R, HashL, EqualL> _left;
        std::unordered_map<R, L, HashR, EqualR> _right;

    public:
        // ------------ Function ---------- //
        _cold void clear(void)
        {
            this->requireUnfrozen();
            this->_left.clear();
            this->_right.clear();
        };
        _cold void removeElement(const L& left)
        {
            this->requireUnfrozen();
            if (!this->_left.contains(left)) {
                utils::exception::WarningException e(utils::exception::InternalCode::UnknownKey);
                std::cerr << e.formated() << std::endl;
                return;
            }
            this->_right.erase(this->_left[left]);
            this->_left.erase(left);
        };
        _cold inline void removeElements(const std::vector<L>& lefts) {for (const L& left: lefts) this->removeElement(left);};
        _cold void removeElement(const R& right)
        {
            this->requireUnfrozen();
            if (!this->_right.contains(right)) {
                utils::exception::WarningException e(utils::exception::InternalCode::UnknownKey);
                std::cerr << e.formated() << std::endl;
                return;
            }
            this->_left.erase(this->_right[right]);
            this->_right.erase(right);
        };
        _cold inline void removeElements(const std::vector<R>& rights) {for (const R& right: rights) this->removeElement(right);};
        _cold inline void addElement(const L& left, const R& right)    {this->setElement(left, right);};
        _cold inline void addElement(const R& right, const L& left)    {this->setElement(right, left);};
        template<bool force = false> // Can't override an exiting one by default, throw of error
        _cold void setElement(const L& left, const R& right)
        {
            this->requireUnfrozen();
            if constexpr (!force) {
                if (this->_left.contains(left) || this->_right.contains(right))
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Override, "The override is disabled for the BidirectionalLookupTable");
            } else {
                // Remove the old pairs to not keep a dangling reverse link
                if (auto it = this->_left.find(left); it != this->_left.end()) {this->_right.erase(it->second); this->_left.erase(it);}
                if (auto it = this->_right.find(right); it != this->_right.end()) {this->_left.erase(it->second); this->_right.erase(it);}
            }
            this->_left[left] = right;
            this->_right[right] = left;
        };
        template<bool force = false> // Can't override an exiting one by default, throw of error
        _cold inline void setElement(const R& right, const L& left) {this->template setElement<force>(left, right);};

        // ------------ Operator ---------- //
        BidirectionalLookupTable& operator=(const BidirectionalLookupTable& other) = delete;
        BidirectionalLookupTable& operator=(BidirectionalLookupTable&& other) = default;
        _hot _nodiscard const R& operator[](const L& left) const
        {
            if (!this->_left.contains(left)) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::UnknownKey);
            }
            return this->_left.at(left);
        };
        _hot _nodiscard const L& operator[](const R& right) const
        {
            if (!this->_right.contains(right)) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::UnknownKey);
            }
            return this->_right.at(right);
        };

        // ---------- Constructor --------- //
        BidirectionalLookupTable() = default;
        BidirectionalLookupTable(const BidirectionalLookupTable& other) = delete;
        BidirectionalLookupTable(BidirectionalLookupTable&& other) = default;

        // ----------- Destructor --------- //
        ~BidirectionalLookupTable() = default;
};

} // namespace end
#endif /* BIDIRECTIONALLOOKUPTABLETT_H */
