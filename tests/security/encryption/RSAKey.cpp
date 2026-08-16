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
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <string>

class RSAKeyTest: public ::testing::Test, public ::testing::WithParamInterface<std::string>
{
    protected:
        void SetUp(void) final {this->_key.generate();};

    public:
        utils::security::encryption::RSAKey _key;
};

TEST_P(RSAKeyTest, Encryption) {
    std::string en, s = GetParam();
    ASSERT_NO_THROW(en = this->_key.encrypt(s));
    ASSERT_NE(en, s);
}

TEST_P(RSAKeyTest, Decryption) {
    std::string s = GetParam();
    try {
        this->_key.decrypt(s);
        FAIL() << "Expected decryption to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Decryption);
    }
}

TEST_P(RSAKeyTest, EncryptionDecryption) {
    std::string de, s = GetParam();
    ASSERT_NO_THROW(de = this->_key.decrypt(this->_key.encrypt(s)));
    ASSERT_EQ(de, s);
}

TEST_P(RSAKeyTest, SwapKeys) {
    std::string s = GetParam();

    // Setup local key
    utils::security::encryption::RSAKey key;
    key.generate();

    // Encrypt using both version
    std::string en1, en2;
    ASSERT_NO_THROW(en1 = this->_key.encrypt(s));
    ASSERT_NO_THROW(en2 = key.encrypt(s));

    // Swap the keys
    utils::security::encryption::KeyPair k1 = this->_key.get();
    utils::security::encryption::KeyPair k2 = key.get();
    this->_key.set(k2);
    key.set(k1);

    // Try to decrypt
    std::string de1, de2;
    ASSERT_NO_THROW(de1 = this->_key.decrypt(en2));
    ASSERT_NO_THROW(de2 = key.decrypt(en1));

    // Check decryption
    ASSERT_EQ(de1, s);
    ASSERT_EQ(de2, s);
}

INSTANTIATE_TEST_SUITE_P(InputCases, RSAKeyTest,
    ::testing::Values(
        "Testing",
        "S.O.S",
        "Please need help, fuck the unit_tests...",
        ""
    )
);
