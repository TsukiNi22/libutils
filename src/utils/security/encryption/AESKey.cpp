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
##  @file AESKey.hpp

File Description:
##  Definition of the AES key methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/security/encryption/AESKey.hpp"
#include <openssl/evp.h>
#include <vector>
#include <string>

_hot _nodiscard std::string utils::security::encryption::AESKey::encrypt(const std::string& s, utils::security::encryption::KeyAES& key) const
{
    std::vector<std::uint8_t> data = utils::security::encryption::stringToKey(s);
    std::vector<std::uint8_t> AES = utils::security::encryption::stringToKey(key.AES);
    std::vector<std::uint8_t> iv = utils::security::encryption::stringToKey(key.iv);
    std::vector<std::uint8_t> tag(16);
    std::vector<std::uint8_t> encryptedData(data.size() + 16);
    int total = 0, len = 0;

    // Init the context
    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
    if (!context)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the creation of the AES context");
    if (!EVP_EncryptInit_ex(context, EVP_aes_256_gcm(), nullptr, AES.data(), iv.data())) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Failed to init AES encryption");
    }

    // Encrypt the data
    if (!EVP_EncryptUpdate(context, encryptedData.data(), &len, data.data(), data.size())) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the AES encryption of the data");
    }
    total = len;

    // Potential padding
    if (!EVP_EncryptFinal_ex(context, encryptedData.data() + total, &len)) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during AES finalization");
    }
    total += len;

    // Get the tag
    if (EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Tag error on AES encryption");
    }
    key.tag = utils::security::encryption::keyToString(tag);

    // Reduce the size of the vector to the right size
    encryptedData.resize(total);

    // Clear
    EVP_CIPHER_CTX_free(context);

    return utils::security::encryption::keyToString(encryptedData);
}

_hot _nodiscard std::string utils::security::encryption::AESKey::decrypt(const std::string& s, utils::security::encryption::KeyAES& key) const
{
    std::vector<std::uint8_t> encryptedData = utils::security::encryption::stringToKey(s);
    std::vector<std::uint8_t> AES = utils::security::encryption::stringToKey(key.AES);
    std::vector<std::uint8_t> iv = utils::security::encryption::stringToKey(key.iv);
    std::vector<std::uint8_t> tag = utils::security::encryption::stringToKey(key.tag);
    std::vector<std::uint8_t> data(encryptedData.size());
    int total = 0, len = 0;
    
    // Init the context
    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
    if (!context)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the creation of the AES context");
    if (!EVP_DecryptInit_ex(context, EVP_aes_256_gcm(), nullptr, AES.data(), iv.data())) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Failed to init AES decryption");
    }

    // Set the tag
    if (EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_TAG, 16, tag.data()) != 1) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Tag error on AES decryption");
    }

    // Decrypt the data
    if (!EVP_DecryptUpdate(context, data.data(), &len, encryptedData.data(), encryptedData.size())) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the AES decryption of the data");
    }
    total = len;

    // Potential padding
    if (!EVP_DecryptFinal_ex(context, data.data() + total, &len)) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during AES finalization");
    }
    total += len;

    // Reduce the size of the vector to the right size
    data.resize(total);

    // Clear
    EVP_CIPHER_CTX_free(context);
    
    return utils::security::encryption::keyToString(data);
}
