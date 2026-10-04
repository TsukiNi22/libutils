/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 16/08/2026 by @author Tsukini

File Name:
##  @file RSAKey.cpp

File Description:
##  Definition of the RSA key methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/security/encryption/RSAKey.hpp"
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

/* encapsulation using unique_ptr */
using BioPtr = std::unique_ptr<BIO, decltype(&BIO_free)>;
using PkeyPtr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using PkeyCtxPtr = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;

_hot _nodiscard static inline BioPtr make_bio(BIO* bio)                   {return BioPtr(bio, BIO_free);};
_hot _nodiscard static inline PkeyPtr make_pkey(EVP_PKEY* pkey)           {return PkeyPtr(pkey, EVP_PKEY_free);};
_hot _nodiscard static inline PkeyCtxPtr make_pkey_ctx(EVP_PKEY_CTX* ctx) {return PkeyCtxPtr(ctx, EVP_PKEY_CTX_free);};

_cold void utils::security::encryption::RSAKey::generate(void)
{
    char* privData = nullptr;
    char* pubData = nullptr;
    long privLen = 0, pubLen = 0;

    // Init the keygen context
    PkeyCtxPtr genCtx = make_pkey_ctx(EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr));
    if (!genCtx)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the creation of the RSA keygen context");

    if (EVP_PKEY_keygen_init(genCtx.get()) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the initialization of the RSA keygen context");

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(genCtx.get(), 2048) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error while setting the RSA key size");

    EVP_PKEY* rawPkey = nullptr;
    if (EVP_PKEY_keygen(genCtx.get(), &rawPkey) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Failed to generate RSA key");
    PkeyPtr pkey = make_pkey(rawPkey);

    // Store the key in memory (PEM)
    BioPtr privBio = make_bio(BIO_new(BIO_s_mem()));
    BioPtr pubBio  = make_bio(BIO_new(BIO_s_mem()));
    if (!privBio || !pubBio)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the allocation of the BIO to store the key (PEM)");

    if (!PEM_write_bio_PrivateKey(privBio.get(), pkey.get(), nullptr, nullptr, 0, nullptr, nullptr)
        || !PEM_write_bio_PUBKEY(pubBio.get(), pkey.get()))
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Failed to store keys (PEM)");

    // Convert the key PEM in string
    if ((privLen = BIO_get_mem_data(privBio.get(), &privData)) <= 0 || !privData)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Failed to convert private keys (PEM) into string");
    this->_keys.priv.assign(privData, static_cast<std::size_t>(privLen));

    if ((pubLen = BIO_get_mem_data(pubBio.get(), &pubData)) <= 0 || !pubData)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Failed to convert public keys (PEM) into string");
    this->_keys.pub.assign(pubData, static_cast<std::size_t>(pubLen));
}

_hot _nodiscard std::string utils::security::encryption::RSAKey::encrypt(const std::string& s) const
{
    std::vector<std::uint8_t> data = utils::security::encryption::string_to_key(s);

    // Convert the key string in PEM
    BioPtr pubBio = make_bio(BIO_new_mem_buf(this->_keys.pub.data(), static_cast<int>(this->_keys.pub.size())));
    if (!pubBio)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the allocation of the BIO to store the key (PEM)");

    PkeyPtr pkey = make_pkey(PEM_read_bio_PUBKEY(pubBio.get(), nullptr, nullptr, nullptr));
    if (!pkey)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the read of the RSA key");

    PkeyCtxPtr ctx = make_pkey_ctx(EVP_PKEY_CTX_new(pkey.get(), nullptr));
    if (!ctx || EVP_PKEY_encrypt_init(ctx.get()) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the initialization of the RSA encryption context");

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error while setting the RSA padding");

    // RSA can only encrypt (key size - padding) bytes at once: encrypt block by block
    const int keySize = EVP_PKEY_get_size(pkey.get());
    if (keySize <= RSA_OAEP_PADDING_SIZE) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Invalid RSA key size");
    }
    const std::size_t blockSize = static_cast<std::size_t>(keySize - RSA_OAEP_PADDING_SIZE);

    std::vector<std::uint8_t> encryptedData;
    std::size_t offset = 0;
    do { // at least one block (empty data)
        const std::size_t len = std::min(blockSize, data.size() - offset);

        // Determine the output size, then encrypt
        std::size_t outLen = 0;
        if (EVP_PKEY_encrypt(ctx.get(), nullptr, &outLen, data.data() + offset, len) <= 0)
            throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the RSA encryption setup of the data");

        std::vector<std::uint8_t> block(outLen);
        if (EVP_PKEY_encrypt(ctx.get(), block.data(), &outLen, data.data() + offset, len) <= 0)
            throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the RSA encryption of the data");

        encryptedData.insert(encryptedData.end(), block.begin(), block.begin() + static_cast<std::ptrdiff_t>(outLen));
        offset += len;
    } while (offset < data.size());

    return utils::security::encryption::key_to_string(encryptedData);
}

_hot _nodiscard std::string utils::security::encryption::RSAKey::decrypt(const std::string& s) const
{
    std::vector<std::uint8_t> encryptedData = utils::security::encryption::string_to_key(s);

    // Convert the key string in PEM
    BioPtr privBio = make_bio(BIO_new_mem_buf(this->_keys.priv.data(), static_cast<int>(this->_keys.priv.size())));
    if (!privBio)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the allocation of the BIO to store the key (PEM)");

    PkeyPtr pkey = make_pkey(PEM_read_bio_PrivateKey(privBio.get(), nullptr, nullptr, nullptr));
    if (!pkey)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the read of the RSA key");

    PkeyCtxPtr ctx = make_pkey_ctx(EVP_PKEY_CTX_new(pkey.get(), nullptr));
    if (!ctx || EVP_PKEY_decrypt_init(ctx.get()) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the initialization of the RSA decryption context");

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error while setting the RSA padding");

    // The data is a list of encrypted blocks of 'key size' bytes
    const std::size_t keySize = static_cast<std::size_t>(EVP_PKEY_get_size(pkey.get()));
    if (keySize == 0 || encryptedData.empty() || encryptedData.size() % keySize != 0) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Invalid RSA encrypted data size");
    }

    std::vector<std::uint8_t> data;
    for (std::size_t offset = 0; offset < encryptedData.size(); offset += keySize) {
        // Determine the output size, then decrypt
        std::size_t outLen = 0;
        if (EVP_PKEY_decrypt(ctx.get(), nullptr, &outLen, encryptedData.data() + offset, keySize) <= 0)
            throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the RSA decryption of the data");

        std::vector<std::uint8_t> block(outLen);
        if (EVP_PKEY_decrypt(ctx.get(), block.data(), &outLen, encryptedData.data() + offset, keySize) <= 0)
            throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the RSA decryption of the data");

        data.insert(data.end(), block.begin(), block.begin() + static_cast<std::ptrdiff_t>(outLen));
    }

    return utils::security::encryption::key_to_string(data);
}
