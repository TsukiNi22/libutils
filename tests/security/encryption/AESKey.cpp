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
##  @file AESKey.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <string>

class AESKeyTest: public ::testing::Test, public ::testing::WithParamInterface<std::string>
{
    protected:
        void SetUp(void) final {
            this->_AES.AES = this->_key.generateRandomBytes(32);
            this->_AES.iv = this->_key.generateRandomBytes(16);
        };

    public:
        utils::security::encryption::AESKey _key;
        utils::security::encryption::KeyAES _AES;
};

TEST_P(AESKeyTest, Encryption) {
    std::string en, s = GetParam();
    ASSERT_NO_THROW(en = this->_key.encrypt(s, this->_AES));
    if (!s.empty()) ASSERT_NE(en, s);
}

TEST_P(AESKeyTest, Decryption) {
    std::string s = GetParam();
    try {
        this->_key.decrypt(s, this->_AES);
        FAIL() << "Expected decryption to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Decryption);
    }
}

TEST_P(AESKeyTest, EncryptionDecryption) {
    std::string de, s = GetParam();
    ASSERT_NO_THROW(de = this->_key.decrypt(this->_key.encrypt(s, this->_AES), this->_AES));
    ASSERT_EQ(de, s);
}

TEST_P(AESKeyTest, SwapKeys) {
    std::string s = GetParam();

    // Setup local key
    utils::security::encryption::AESKey key;
    utils::security::encryption::KeyAES AES;
    AES.AES = key.generateRandomBytes(32);
    AES.iv = key.generateRandomBytes(16);

    // Encrypt using both version
    std::string en1, en2;
    ASSERT_NO_THROW(en1 = this->_key.encrypt(s, this->_AES));
    ASSERT_NO_THROW(en2 = key.encrypt(s, AES));

    // Swap the keys
    utils::security::encryption::KeyAES tmp = AES;
    AES = this->_AES;
    this->_AES = tmp;

    // Try to decrypt
    std::string de1, de2;
    ASSERT_NO_THROW(de1 = this->_key.decrypt(en1, AES));
    ASSERT_NO_THROW(de2 = key.decrypt(en2, this->_AES));

    // Check decryption
    ASSERT_EQ(de1, s);
    ASSERT_EQ(de2, s);
}

INSTANTIATE_TEST_SUITE_P(InputCases, AESKeyTest,
    ::testing::Values(
        "Testing",
        "S.O.S",
        "Please need help, fuck the unit_tests...",
        ""
    )
);
