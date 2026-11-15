/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file CApi.cpp

File Description:
##  Unit tests of the C API of utils::security (AES & RSA keys, observers of the C side)
\**************************************************************/

#include "utils/security/CApi.h"
#include "tools/TempDir.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <string>

class SecurityCApiTest: public ::testing::Test {
    protected:
        void SetUp(void) override
        {
            utils_exception_clear();
        };
};

//----------------------------------------------------------------//
/* AES */

TEST_F(SecurityCApiTest, AesRoundTrip)
{
    utils_aes_key_t* key = utils_aes_key_create();
    utils_aes_data_t* data = utils_aes_data_create();
    utils_string_t* bytes = utils_string_create(nullptr, 0);
    utils_string_t* encrypted = utils_string_create(nullptr, 0);
    utils_string_t* decrypted = utils_string_create(nullptr, 0);
    size_t size = 0;

    ASSERT_NE(key, nullptr);
    ASSERT_EQ(utils_aes_key_random_bytes(key, 32, bytes), UTILS_OK);
    EXPECT_EQ(utils_string_size(bytes), 32u);
    ASSERT_EQ(utils_aes_data_set_key(data, utils_string_data(bytes), 32), UTILS_OK);
    ASSERT_EQ(utils_aes_key_random_bytes(key, 16, bytes), UTILS_OK);
    ASSERT_EQ(utils_aes_data_set_iv(data, utils_string_data(bytes), 16), UTILS_OK);
    ASSERT_EQ(utils_aes_key_encrypt(key, "secret message", 14, data, encrypted), UTILS_OK);
    EXPECT_NE(std::string(utils_string_data(encrypted), utils_string_size(encrypted)), "secret message");
    EXPECT_NE(utils_aes_data_get_tag(data, &size), nullptr);
    EXPECT_GT(size, 0u);
    ASSERT_EQ(utils_aes_key_decrypt(key, utils_string_data(encrypted), utils_string_size(encrypted), data, decrypted), UTILS_OK);
    EXPECT_EQ(std::string(utils_string_data(decrypted), utils_string_size(decrypted)), "secret message");
    EXPECT_FALSE(utils_aes_key_has_generate_overload(key));
    EXPECT_EQ(utils_aes_data_set_tag(data, "xxxx", 4), UTILS_OK); // tampered tag
    EXPECT_EQ(utils_aes_key_decrypt(key, utils_string_data(encrypted), utils_string_size(encrypted), data, decrypted), UTILS_KO);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_INTERNAL);
    utils_string_destroy(decrypted);
    utils_string_destroy(encrypted);
    utils_string_destroy(bytes);
    utils_aes_data_destroy(data);
    utils_aes_key_destroy(key);
}

//----------------------------------------------------------------//
/* RSA */

TEST_F(SecurityCApiTest, RsaRoundTripAndCommonKey)
{
    tests::tools::TempDir dir;
    utils_rsa_key_t* key = utils_rsa_key_create();
    utils_string_t* encrypted = utils_string_create(nullptr, 0);
    utils_string_t* decrypted = utils_string_create(nullptr, 0);

    ASSERT_NE(key, nullptr);
    EXPECT_TRUE(utils_rsa_key_has_generate_overload(key));
    ASSERT_EQ(utils_rsa_key_generate(key), UTILS_OK);
    EXPECT_NE(std::string(utils_rsa_key_get_public(key)).find("BEGIN PUBLIC KEY"), std::string::npos);
    ASSERT_EQ(utils_rsa_key_encrypt(key, "hello", 5, encrypted), UTILS_OK);
    ASSERT_EQ(utils_rsa_key_decrypt(key, utils_string_data(encrypted), utils_string_size(encrypted), decrypted), UTILS_OK);
    EXPECT_STREQ(utils_string_data(decrypted), "hello");

    std::ofstream(dir / "common", std::ios::binary) << utils_rsa_key_get_private(key);
    std::ofstream(dir / "common.pub", std::ios::binary) << utils_rsa_key_get_public(key);
    utils_rsa_key_t* common = utils_rsa_key_create_common((dir / "common").string().c_str());
    ASSERT_NE(common, nullptr) << utils_exception.info;
    EXPECT_STREQ(utils_rsa_key_get_public(common), utils_rsa_key_get_public(key));
    EXPECT_EQ(utils_rsa_key_decrypt(common, utils_string_data(encrypted), utils_string_size(encrypted), decrypted), UTILS_OK);
    EXPECT_STREQ(utils_string_data(decrypted), "hello");
    EXPECT_EQ(utils_rsa_key_create_common((dir / "missing").string().c_str()), nullptr);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::Encryption));
    EXPECT_EQ(utils_rsa_key_set(key, nullptr, nullptr), UTILS_OK);
    EXPECT_STREQ(utils_rsa_key_get_private(key), "");

    utils_rsa_key_destroy(common);
    utils_string_destroy(decrypted);
    utils_string_destroy(encrypted);
    utils_rsa_key_destroy(key);
}

//----------------------------------------------------------------//
/* OBSERVER */

TEST_F(SecurityCApiTest, ObserverOfTheCSideIsReported)
{
    utils_observer_t* observer = utils_observer_create("CSideInstance", true);
    std::string report;

    ASSERT_NE(observer, nullptr);
    testing::internal::CaptureStderr();
    EXPECT_EQ(utils_observer_trigger(), UTILS_OK);
    report = testing::internal::GetCapturedStderr();
    EXPECT_NE(report.find("CSideInstance"), std::string::npos);
    utils_observer_destroy(observer);
    testing::internal::CaptureStderr();
    EXPECT_EQ(utils_observer_trigger(), UTILS_OK);
    report = testing::internal::GetCapturedStderr();
    EXPECT_EQ(report.find("CSideInstance"), std::string::npos);
    EXPECT_EQ(utils_observer_create(nullptr, true), nullptr);
    utils_observer_destroy(nullptr);
}
