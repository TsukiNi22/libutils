/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 03/08/2026 by @author Tsukini

File Name:
##  @file RSAKey.hpp

File Description:
##  Definition of the CommomRSA key methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/security/encryption/CommonRSAKey.hpp"
#include <fstream>
#include <sstream>

_cold void utils::security::encryption::CommonRSAKey::loadCommon(std::string path)
{
    KeyPair keys;

    // Open the file (pub)
    std::ifstream filePub(path + ".pub", std::ios::binary);
    if (filePub) {
        // Read the file
        std::ostringstream ss;
        ss << filePub.rdbuf();
        keys.pub = ss.str();
    }

    // Open the file (priv)
    std::ifstream filePriv(path, std::ios::binary);
    if (filePriv) {
        // Read the file
        std::ostringstream ss;
        ss << filePriv.rdbuf();
        keys.priv = ss.str();
    }

    // Check if any of the 2 file where found
    if (keys.pub.empty() && keys.priv.empty()) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Can't open any common RSA files: " + path);
    }

    this->set(keys);
}
