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
##  @file RSAKey.cpp

File Description:
##  Definition of the RSA key methods (OpenSSL 3.0 EVP_PKEY API)
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/manip/smanip/key/RSAKey.hpp"
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <memory>
#include <vector>
#include <string>

namespace {

using BioPtr = std::unique_ptr<BIO, decltype(&BIO_free)>;
using PkeyPtr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using PkeyCtxPtr = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;

inline BioPtr makeBio(BIO* bio)
{
    return BioPtr(bio, BIO_free);
}

inline PkeyPtr makePkey(EVP_PKEY* pkey)
{
    return PkeyPtr(pkey, EVP_PKEY_free);
}

inline PkeyCtxPtr makePkeyCtx(EVP_PKEY_CTX* ctx)
{
    return PkeyCtxPtr(ctx, EVP_PKEY_CTX_free);
}

} // namespace

_cold void utils::smanip::key::RSAKey::generate(void)
{
    char* privData = nullptr;
    char* pubData = nullptr;
    long privLen = 0, pubLen = 0;

    // Init the keygen context
    PkeyCtxPtr genCtx = makePkeyCtx(EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr));
    if (!genCtx)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the creation of the RSA keygen context");

    if (EVP_PKEY_keygen_init(genCtx.get()) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the initialization of the RSA keygen context");

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(genCtx.get(), 2048) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error while setting the RSA key size");

    EVP_PKEY* rawPkey = nullptr;
    if (EVP_PKEY_keygen(genCtx.get(), &rawPkey) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Failed to generate RSA key");
    PkeyPtr pkey = makePkey(rawPkey);

    // Store the key in memory (PEM)
    BioPtr privBio = makeBio(BIO_new(BIO_s_mem()));
    BioPtr pubBio  = makeBio(BIO_new(BIO_s_mem()));
    if (!privBio || !pubBio)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the allocation of the BIO to store the key (PEM)");

    if (!PEM_write_bio_PrivateKey(privBio.get(), pkey.get(), nullptr, nullptr, 0, nullptr, nullptr)
        || !PEM_write_bio_PUBKEY(pubBio.get(), pkey.get()))
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Failed to store keys (PEM)");

    // Convert the key PEM in string
    if ((privLen = BIO_get_mem_data(privBio.get(), &privData)) <= 0 || !privData)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Failed to convert private keys (PEM) into string");
    this->_keys.priv.assign(privData, privLen);

    if ((pubLen = BIO_get_mem_data(pubBio.get(), &pubData)) <= 0 || !pubData)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Failed to convert public keys (PEM) into string");
    this->_keys.pub.assign(pubData, pubLen);
}

_hot _nodiscard std::string utils::smanip::key::RSAKey::encrypt(const std::string& s) const
{
    std::vector<std::uint8_t> data = utils::smanip::key::stringToKey(s);

    // Convert the key string in PEM
    BioPtr pubBio = makeBio(BIO_new_mem_buf(this->_keys.pub.data(), static_cast<int>(this->_keys.pub.size())));
    if (!pubBio)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the allocation of the BIO to store the key (PEM)");

    PkeyPtr pkey = makePkey(PEM_read_bio_PUBKEY(pubBio.get(), nullptr, nullptr, nullptr));
    if (!pkey)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the read of the RSA key");

    PkeyCtxPtr ctx = makePkeyCtx(EVP_PKEY_CTX_new(pkey.get(), nullptr));
    if (!ctx || EVP_PKEY_encrypt_init(ctx.get()) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the initialization of the RSA encryption context");

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error while setting the RSA padding");

    // Determine the output size, then encrypt
    std::size_t outLen = 0;
    if (EVP_PKEY_encrypt(ctx.get(), nullptr, &outLen, data.data(), data.size()) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Data too big to be encrypted");

    std::vector<uint8_t> encryptedData(outLen);
    if (EVP_PKEY_encrypt(ctx.get(), encryptedData.data(), &outLen, data.data(), data.size()) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Error during the RSA encryption of the data");

    encryptedData.resize(outLen);
    return utils::smanip::key::keyToString(encryptedData);
}

_hot _nodiscard std::string utils::smanip::key::RSAKey::decrypt(const std::string& s) const
{
    std::vector<std::uint8_t> encryptedData = utils::smanip::key::stringToKey(s);

    // Convert the key string in PEM
    BioPtr privBio = makeBio(BIO_new_mem_buf(this->_keys.priv.data(), static_cast<int>(this->_keys.priv.size())));
    if (!privBio)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the allocation of the BIO to store the key (PEM)");

    PkeyPtr pkey = makePkey(PEM_read_bio_PrivateKey(privBio.get(), nullptr, nullptr, nullptr));
    if (!pkey)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the read of the RSA key");

    PkeyCtxPtr ctx = makePkeyCtx(EVP_PKEY_CTX_new(pkey.get(), nullptr));
    if (!ctx || EVP_PKEY_decrypt_init(ctx.get()) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the initialization of the RSA decryption context");

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error while setting the RSA padding");

    // Determine the output size, then decrypt
    std::size_t outLen = 0;
    if (EVP_PKEY_decrypt(ctx.get(), nullptr, &outLen, encryptedData.data(), encryptedData.size()) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the RSA decryption of the data");

    std::vector<uint8_t> data(outLen);
    if (EVP_PKEY_decrypt(ctx.get(), data.data(), &outLen, encryptedData.data(), encryptedData.size()) <= 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Decryption, "Error during the RSA decryption of the data");

    data.resize(outLen);
    return utils::smanip::key::keyToString(data);
}
