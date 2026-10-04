/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 01/10/2026 by @author Tsukini

File Name:
##  @file CommonRSAKey.cpp

File Description:
##  Unit tests of the CommonRSAKey (loading of the shared key from files)
\**************************************************************/

#include "utils.hpp"
#include "tools/TempDir.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>

class CommonRSAKeyTest: public ::testing::Test {
    protected:
        tests::tools::TempDir _dir;
        utils::security::encryption::KeyPair _keys;

        void SetUp(void) override
        {
            utils::security::encryption::RSAKey key;
            key.generate();
            this->_keys = key.get();
        };
        void writePriv(const std::filesystem::path& path) {std::ofstream(path, std::ios::binary) << this->_keys.priv;};
        void writePub(const std::filesystem::path& path) {std::ofstream(path, std::ios::binary) << this->_keys.pub;};
};

TEST_F(CommonRSAKeyTest, LoadBoth) {
    this->writePriv(this->_dir / "common");
    this->writePub(this->_dir / "common.pub");

    utils::security::encryption::CommonRSAKey key;
    ASSERT_NO_THROW(key.loadCommon((this->_dir / "common").string()));
    EXPECT_EQ(key.get().priv, this->_keys.priv);
    EXPECT_EQ(key.get().pub, this->_keys.pub);
    EXPECT_EQ(key.decrypt(key.encrypt("hello")), "hello");
}

TEST_F(CommonRSAKeyTest, ConstructorLoad) {
    this->writePriv(this->_dir / "common");
    this->writePub(this->_dir / "common.pub");

    utils::security::encryption::CommonRSAKey key((this->_dir / "common").string());
    EXPECT_EQ(key.get().pub, this->_keys.pub);
}

TEST_F(CommonRSAKeyTest, LoadOnlyPublic) {
    this->writePub(this->_dir / "common.pub");

    utils::security::encryption::CommonRSAKey key;
    ASSERT_NO_THROW(key.loadCommon((this->_dir / "common").string()));
    EXPECT_TRUE(key.get().priv.empty());
    EXPECT_EQ(key.get().pub, this->_keys.pub);
    EXPECT_NO_THROW((void)key.encrypt("hello"));
}

TEST_F(CommonRSAKeyTest, LoadOnlyPrivate) {
    this->writePriv(this->_dir / "common");

    utils::security::encryption::CommonRSAKey key;
    ASSERT_NO_THROW(key.loadCommon((this->_dir / "common").string()));
    EXPECT_TRUE(key.get().pub.empty());
    EXPECT_EQ(key.get().priv, this->_keys.priv);
}

TEST_F(CommonRSAKeyTest, PublicAndPrivateSidesCommunicate) {
    this->writePriv(this->_dir / "private");
    this->writePub(this->_dir / "public.pub");

    utils::security::encryption::CommonRSAKey sender((this->_dir / "public").string());
    utils::security::encryption::CommonRSAKey receiver((this->_dir / "private").string());
    EXPECT_EQ(receiver.decrypt(sender.encrypt("secret")), "secret");
}

TEST_F(CommonRSAKeyTest, MissingFiles) {
    utils::security::encryption::CommonRSAKey key;
    try {
        key.loadCommon((this->_dir / "nothing").string());
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Encryption);
    }
}

TEST_F(CommonRSAKeyTest, TildeExpansion) {
    tests::tools::ScopedEnv env("HOME", this->_dir.path().string());
    this->writePriv(this->_dir / "common");
    this->writePub(this->_dir / "common.pub");

    utils::security::encryption::CommonRSAKey key;
    ASSERT_NO_THROW(key.loadCommon("~/common"));
    EXPECT_EQ(key.get().pub, this->_keys.pub);
}

TEST_F(CommonRSAKeyTest, DefaultPath) {
    tests::tools::ScopedEnv env("HOME", this->_dir.path().string());
    std::filesystem::create_directories(this->_dir / ".ssh");
    this->writePriv(this->_dir / ".ssh/common");
    this->writePub(this->_dir / ".ssh/common.pub");

    utils::security::encryption::CommonRSAKey key;
    ASSERT_NO_THROW(key.loadCommon());
    EXPECT_EQ(key.get().priv, this->_keys.priv);
}

TEST_F(CommonRSAKeyTest, TildeWithoutHome) {
    tests::tools::ScopedEnv env("HOME", "");
    ::unsetenv("HOME");
    utils::security::encryption::CommonRSAKey key;
    try {
        key.loadCommon("~/common");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Encryption);
    }
}
