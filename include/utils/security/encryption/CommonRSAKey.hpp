/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/08/2026 by @author Tsukini

File Name:
##  @file CommonRSAKey.hpp

File Description:
##  Declaration of the key used for the common RSA
\**************************************************************/

#ifndef COMMOMRSAKEY_H
    #define COMMOMRSAKEY_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _cold, _nodiscard
    #include "RSAKey.hpp"                       // utils::security::encryption::RSAKey
    #include <string>                           // std::string

    //----------------------------------------------------------------//
    /* DEFINE */

    /* default */
    #define DEFAULT_COMMON_RSA_PATH "~/.ssh/common" // common (priv) | common.pub (pub)

namespace utils::security::encryption { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class CommonRSAKey: public utils::security::encryption::RSAKey {
    public:
        // ---------- Pre-Function -------- //
        void loadCommon(std::string path = DEFAULT_COMMON_RSA_PATH);

        // ------------ Operator ---------- //
        CommonRSAKey& operator=(const CommonRSAKey& other) = default;
        CommonRSAKey& operator=(CommonRSAKey&& other) = default;

        // ---------- Constructor --------- //
        CommonRSAKey() = default;
        CommonRSAKey(std::string path) {this->loadCommon(path);};
        CommonRSAKey(const CommonRSAKey& other) = default;
        CommonRSAKey(CommonRSAKey&& other) = default;

        // ----------- Destructor --------- //
        virtual ~CommonRSAKey() = default;
};

} // namespace end
#endif /* COMMOMRSAKEY_H */
