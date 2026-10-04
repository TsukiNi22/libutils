/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 04/10/2026 by @author Tsukini

File Name:
##  @file Key.cpp

File Description:
##  Unit tests of the key tools (key_to_string, string_to_key), the AKey defaults & the IKey interface
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/* ------------------------------ Key tools ------------------------------ */
TEST(KeyTools, RoundTrip) {
    const std::vector<std::uint8_t> bytes = {0x00, 0x01, 0x7F, 0x80, 0xFF, 'a', '\n'};
    const std::string s = utils::security::encryption::key_to_string(bytes);
    EXPECT_EQ(s.size(), bytes.size()); // the '\0' is kept
    EXPECT_EQ(utils::security::encryption::string_to_key(s), bytes);
}

TEST(KeyTools, Empty) {
    EXPECT_TRUE(utils::security::encryption::key_to_string({}).empty());
    EXPECT_TRUE(utils::security::encryption::string_to_key("").empty());
}

TEST(KeyTools, HighBytesStayUnsigned) {
    const std::vector<std::uint8_t> bytes = utils::security::encryption::string_to_key(std::string(1, static_cast<char>(-1)));
    ASSERT_EQ(bytes.size(), 1u);
    EXPECT_EQ(bytes[0], 0xFF);
}

/* -------------------------------- AKey --------------------------------- */
TEST(AKey, GenerateRandomBytesSize) {
    utils::security::encryption::AESKey key;
    for (std::uint16_t size: std::vector<std::uint16_t>{0, 1, 16, 32, 4096})
        EXPECT_EQ(key.generateRandomBytes(size).size(), size);
}

TEST(AKey, GenerateRandomBytesDiffer) {
    utils::security::encryption::AESKey key;
    EXPECT_NE(key.generateRandomBytes(32), key.generateRandomBytes(32)); // 2^-256 chance of a false failure
}

TEST(AKey, AESKeyHasNoOverload) {
    utils::security::encryption::AESKey key;
    EXPECT_FALSE(key.hasGenerateOverload());
    EXPECT_FALSE(key.hasSetOverload());
    EXPECT_FALSE(key.hasGetOverload());
}

TEST(AKey, RSAKeyHasOverload) {
    utils::security::encryption::RSAKey key;
    EXPECT_TRUE(key.hasGenerateOverload());
    EXPECT_TRUE(key.hasSetOverload());
    EXPECT_TRUE(key.hasGetOverload());
}

TEST(AKeyDeathTest, UndefinedGenerate) {
    utils::security::encryption::AESKey key;
    EXPECT_DEATH(key.generate(), "");
}

TEST(AKeyDeathTest, UndefinedGet) {
    utils::security::encryption::AESKey key;
    EXPECT_DEATH((void)key.get(), "");
}

TEST(AKeyDeathTest, UndefinedSimpleEncrypt) {
    utils::security::encryption::AESKey key;
    utils::security::encryption::IKey<utils::security::encryption::KeyAES>& base = key; // encrypt(s) hidden by AESKey::encrypt(s, key)
    EXPECT_DEATH((void)base.encrypt("data"), "");
}

/* -------------------------------- IKey --------------------------------- */
TEST(IKey, RSAThroughInterface) {
    std::unique_ptr<utils::security::encryption::IKey<utils::security::encryption::KeyPair>> key = std::make_unique<utils::security::encryption::RSAKey>();
    key->generate();
    EXPECT_FALSE(key->get().pub.empty());
    EXPECT_FALSE(key->get().priv.empty());
    utils::security::encryption::KeyPair unused;
    const std::string encrypted = key->encrypt("hello", unused); // fallback on encrypt(s)
    EXPECT_NE(encrypted, "hello");
    EXPECT_EQ(key->decrypt(encrypted, unused), "hello");
}

TEST(IKey, AESThroughInterface) {
    std::unique_ptr<utils::security::encryption::IKey<utils::security::encryption::KeyAES>> key = std::make_unique<utils::security::encryption::AESKey>();
    utils::security::encryption::KeyAES aes;
    aes.AES = key->generateRandomBytes(32);
    aes.iv = key->generateRandomBytes(12);
    const std::string encrypted = key->encrypt("hello", aes);
    EXPECT_EQ(key->decrypt(encrypted, aes), "hello");
}
