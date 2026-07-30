/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 30/07/2026 by @author Tsukini

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

    // Open the file
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Can't open the common RSA file: " + path);

    // Read the file
    std::ostringstream ss;
    ss << file.rdbuf();
    keys.pub = ss.str();

    this->set(keys);
}
