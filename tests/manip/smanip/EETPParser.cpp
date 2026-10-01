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
##  @file EETPParser.cpp

File Description:
##  Unit tests of the EETPParser (encrypted transmission protocol: handshake, framing, parsing)
\**************************************************************/

#include "utils.hpp"
#include "tools/TempDir.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#define SYN std::string(1, static_cast<char>(utils::iomanip::Char::SYN))
#define SO  std::string(1, static_cast<char>(utils::iomanip::Char::SO))
#define EM  std::string(1, static_cast<char>(utils::iomanip::Char::EM))
#define ACK std::string(1, static_cast<char>(utils::iomanip::Char::ACK))
#define NAK std::string(1, static_cast<char>(utils::iomanip::Char::NAK))
#define ETB static_cast<char>(utils::iomanip::Char::ETB)

// Setup a fake $HOME with a common RSA key (~/.ssh/common & ~/.ssh/common.pub)
class EETPParserTest : public ::testing::Test {
    protected:
        tests::tools::TempDir _home;
        std::unique_ptr<tests::tools::ScopedEnv> _env;
        std::unique_ptr<utils::smanip::parser::EETPParser> _client;
        std::unique_ptr<utils::smanip::parser::EETPParser> _server;

        void SetUp(void) override
        {
            this->_env = std::make_unique<tests::tools::ScopedEnv>("HOME", this->_home.path().string());
            std::filesystem::create_directories(this->_home / ".ssh");

            utils::security::encryption::RSAKey key;
            key.generate();
            std::ofstream(this->_home / ".ssh/common", std::ios::binary) << key.get().priv;
            std::ofstream(this->_home / ".ssh/common.pub", std::ios::binary) << key.get().pub;

            this->_client = std::make_unique<utils::smanip::parser::EETPParser>();
            this->_server = std::make_unique<utils::smanip::parser::EETPParser>();
        };
        void TearDown(void) override
        {
            this->_client.reset();
            this->_server.reset();
            this->_env.reset();
        };

        // Full handshake: client -> SYN -> server, server -> SO -> client
        void handshake(void)
        {
            std::string syn = this->_client->format("server", {SYN, {}});
            utils::smanip::parser::EETPContent synContent = this->_server->parse("client", syn);
            ASSERT_EQ(synContent.type, SYN);

            std::string so = this->_server->format("client", {SO, {}});
            utils::smanip::parser::EETPContent soContent = this->_client->parse("server", so);
            ASSERT_EQ(soContent.type, SO);
        };
};

TEST_F(EETPParserTest, NoCommonKeyThrows) {
    tests::tools::TempDir emptyHome;
    tests::tools::ScopedEnv env("HOME", emptyHome.path().string());
    try {
        utils::smanip::parser::EETPParser parser;
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Encryption);
    }
}

TEST_F(EETPParserTest, HasIdOverload) {
    EXPECT_TRUE(this->_client->hasIdOverload());
    EXPECT_FALSE(this->_client->hasNoIdOverload());
}

TEST_F(EETPParserTest, DisconnectionRoundTrip) {
    // EM is never encrypted, no handshake needed
    std::string framed = this->_client->format("server", {EM, {}});
    ASSERT_FALSE(framed.empty());
    EXPECT_EQ(framed.front(), ETB); // no tag
    utils::smanip::parser::EETPContent content = this->_server->parse("client", framed);
    EXPECT_EQ(content.type, EM);
    EXPECT_TRUE(content.data.empty());
}

TEST_F(EETPParserTest, InvalidTypeSize) {
    try {
        (void)this->_client->format("server", {"AB", {}});
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Parser);
    }
}

TEST_F(EETPParserTest, SetTypeSizeZeroThrows) {
    try {
        this->_client->setTypeSize(0);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Parser);
    }
}

TEST_F(EETPParserTest, ParseTooSmall) {
    try {
        (void)this->_server->parse("client", "");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Parser);
    }
}

TEST_F(EETPParserTest, ParseWithoutTagSeparator) {
    try {
        (void)this->_server->parse("client", "abcdef");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Parser);
    }
}

TEST_F(EETPParserTest, ParseNothingAfterTag) {
    try {
        (void)this->_server->parse("client", std::string("tag") + ETB);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Parser);
    }
}

TEST_F(EETPParserTest, Handshake) {
    this->handshake();
}

TEST_F(EETPParserTest, HandshakeTransmitsClientPublicKey) {
    std::string syn = this->_client->format("server", {SYN, {}});
    utils::smanip::parser::EETPContent content = this->_server->parse("client", syn);
    EXPECT_EQ(content.type, SYN);
    ASSERT_EQ(content.data.size(), 1u);
    EXPECT_NE(content.data[0].find("BEGIN PUBLIC KEY"), std::string::npos);
}

TEST_F(EETPParserTest, MessageAfterHandshake) {
    this->handshake();
    if (::testing::Test::HasFatalFailure()) return;

    // client -> server
    std::string framed = this->_client->format("server", {"A", {"hello", "world"}});
    utils::smanip::parser::EETPContent content = this->_server->parse("client", framed);
    EXPECT_EQ(content.type, "A");
    ASSERT_EQ(content.data.size(), 2u);
    EXPECT_EQ(content.data[0], "hello");
    EXPECT_EQ(content.data[1], "world");

    // server -> client
    framed = this->_server->format("client", {"B", {"answer"}});
    content = this->_client->parse("server", framed);
    EXPECT_EQ(content.type, "B");
    ASSERT_EQ(content.data.size(), 1u);
    EXPECT_EQ(content.data[0], "answer");
}

TEST_F(EETPParserTest, MessageIsEncrypted) {
    this->handshake();
    if (::testing::Test::HasFatalFailure()) return;

    std::string framed = this->_client->format("server", {"A", {"secret-content"}});
    utils::smanip::codec::Base64Codec codec;
    EXPECT_EQ(framed.find(codec.encode("secret-content")), std::string::npos);
    EXPECT_NE(framed.front(), ETB); // there is a tag
}

TEST_F(EETPParserTest, AckAndNak) {
    this->handshake();
    if (::testing::Test::HasFatalFailure()) return;

    utils::smanip::parser::EETPContent content = this->_server->parse("client", this->_client->format("server", {ACK, {"ok"}}));
    EXPECT_EQ(content.type, ACK);
    ASSERT_EQ(content.data.size(), 1u);
    EXPECT_EQ(content.data[0], "ok");

    content = this->_server->parse("client", this->_client->format("server", {NAK, {}}));
    EXPECT_EQ(content.type, NAK);
    EXPECT_TRUE(content.data.empty());
}

TEST_F(EETPParserTest, AckWithTooManyParts) {
    this->handshake();
    if (::testing::Test::HasFatalFailure()) return;

    try {
        (void)this->_server->parse("client", this->_client->format("server", {ACK, {"a", "b"}}));
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Parser);
    }
}

TEST_F(EETPParserTest, TamperedMessageFails) {
    this->handshake();
    if (::testing::Test::HasFatalFailure()) return;

    std::string framed = this->_client->format("server", {"A", {"hello"}});
    framed[framed.size() - 2] = (framed[framed.size() - 2] == 'A' ? 'B' : 'A');
    EXPECT_THROW((void)this->_server->parse("client", framed), utils::exception::IException);
}

TEST_F(EETPParserTest, CustomTypeSize) {
    this->_client->setTypeSize(3);
    this->_server->setTypeSize(3);
    std::string framed = this->_client->format("server", {EM + "xx", {}});
    utils::smanip::parser::EETPContent content = this->_server->parse("client", framed);
    EXPECT_EQ(content.type, EM + "xx");
}

TEST_F(EETPParserTest, NoIdOverloadAbortDeathTest) {
    // AParser default definition for the no-id overload is a FatalException
    const utils::smanip::parser::IParser<utils::smanip::parser::EETPContent>& parser = *this->_client;
    EXPECT_DEATH((void)parser.format(utils::smanip::parser::EETPContent{EM, {}}), "ABORTED");
}
