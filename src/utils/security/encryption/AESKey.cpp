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
##  @file AESKey.cpp

File Description:
##  Definition of the AES key methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/security/encryption/AESKey.hpp"
#include <openssl/evp.h>
#include <cstddef>
#include <cstdint>
#include <climits>
#include <vector>
#include <string>

// OpenSSL takes the data sizes as int
_hot static void check_length(const std::size_t size)
{
    if (size > static_cast<std::size_t>(INT_MAX)) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "AES data of " + std::to_string(size) + " bytes (max " + std::to_string(INT_MAX) + ")");
    }
}

// Check the key/iv/tag sizes (OpenSSL read a fixed number of bytes)
_hot static void check_sizes(const utils::security::encryption::KeyAES& key, const bool tag, const utils::exception::InternalCode code)
{
    if (key.AES.size() != AES_KEY_SIZE) _unlikely {
        throw utils::exception::ErrorException(code, "Invalid AES key size: " + std::to_string(key.AES.size()) + " (expected " + std::to_string(AES_KEY_SIZE) + ")");
    } else if (key.iv.size() < AES_MIN_IV_SIZE) _unlikely {
        throw utils::exception::ErrorException(code, "Invalid AES iv size: " + std::to_string(key.iv.size()) + " (expected at least " + std::to_string(AES_MIN_IV_SIZE) + ")");
    } else if (tag && key.tag.size() != AES_TAG_SIZE) _unlikely {
        throw utils::exception::ErrorException(code, "Invalid AES tag size: " + std::to_string(key.tag.size()) + " (expected " + std::to_string(AES_TAG_SIZE) + ")");
    }
}

_hot _nodiscard std::string utils::security::encryption::AESKey::encrypt(const std::string& s, utils::security::encryption::KeyAES& key) const
{
    check_sizes(key, false, utils::exception::InternalCode::Encryption);
    check_length(s.size());
    std::vector<std::uint8_t> data = utils::security::encryption::string_to_key(s);
    std::vector<std::uint8_t> aesKey = utils::security::encryption::string_to_key(key.AES);
    std::vector<std::uint8_t> iv = utils::security::encryption::string_to_key(key.iv);
    std::vector<std::uint8_t> tag(AES_TAG_SIZE);
    std::vector<std::uint8_t> encryptedData(data.size() + AES_TAG_SIZE);
    int total = 0, len = 0;

    // Init the context
    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
    if (!context)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the creation of the AES context");
    if (!EVP_EncryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr)
        || EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv.size()), nullptr) != 1
        || !EVP_EncryptInit_ex(context, nullptr, nullptr, aesKey.data(), iv.data())) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Failed to init AES encryption");
    }

    // Encrypt the data
    if (!EVP_EncryptUpdate(context, encryptedData.data(), &len, data.data(), static_cast<int>(data.size()))) {
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
    if (EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_GET_TAG, AES_TAG_SIZE, tag.data()) != 1) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Tag error on AES encryption");
    }
    key.tag = utils::security::encryption::key_to_string(tag);

    // Reduce the size of the vector to the right size
    encryptedData.resize(static_cast<std::size_t>(total));

    // Clear
    EVP_CIPHER_CTX_free(context);

    return utils::security::encryption::key_to_string(encryptedData);
}

_hot _nodiscard std::string utils::security::encryption::AESKey::decrypt(const std::string& s, utils::security::encryption::KeyAES& key) const
{
    check_sizes(key, true, utils::exception::InternalCode::Decryption);
    check_length(s.size());
    std::vector<std::uint8_t> encryptedData = utils::security::encryption::string_to_key(s);
    std::vector<std::uint8_t> aesKey = utils::security::encryption::string_to_key(key.AES);
    std::vector<std::uint8_t> iv = utils::security::encryption::string_to_key(key.iv);
    std::vector<std::uint8_t> tag = utils::security::encryption::string_to_key(key.tag);
    std::vector<std::uint8_t> data(encryptedData.size());
    int total = 0, len = 0;

    // Init the context
    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
    if (!context)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the creation of the AES context");
    if (!EVP_DecryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr)
        || EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv.size()), nullptr) != 1
        || !EVP_DecryptInit_ex(context, nullptr, nullptr, aesKey.data(), iv.data())) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Failed to init AES decryption");
    }

    // Set the tag
    if (EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_TAG, AES_TAG_SIZE, tag.data()) != 1) {
        EVP_CIPHER_CTX_free(context);
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Tag error on AES decryption");
    }

    // Decrypt the data
    if (!EVP_DecryptUpdate(context, data.data(), &len, encryptedData.data(), static_cast<int>(encryptedData.size()))) {
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
    data.resize(static_cast<std::size_t>(total));

    // Clear
    EVP_CIPHER_CTX_free(context);

    return utils::security::encryption::key_to_string(data);
}
