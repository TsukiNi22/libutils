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
##  Unit tests of the C API of utils::smanip (byte strings, format, codecs, EETP parser)
\**************************************************************/

#include "utils/manip/smanip/CApi.h"
#include "tools/TempDir.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

#define SYN std::string(1, static_cast<char>(utils::iomanip::Char::SYN))
#define SO  std::string(1, static_cast<char>(utils::iomanip::Char::SO))

class SmanipCApiTest: public ::testing::Test {
    protected:
        void SetUp(void) override
        {
            utils_exception_clear();
        };
};

/* codec of the C side: reversed bytes */
static int reverse(const char* in, size_t size, utils_string_t* out, void* data)
{
    ++*static_cast<int*>(data);
    for (size_t i = 0; i < size; ++i)
        if (utils_string_append(out, in + size - 1 - i, 1) != UTILS_OK) return UTILS_KO;
    return UTILS_OK;
}

static int failing(_unused const char* in, _unused size_t size, _unused utils_string_t* out, _unused void* data)
{
    return utils_fail(9, "codec failed on purpose");
}

//----------------------------------------------------------------//
/* STRING */

TEST_F(SmanipCApiTest, StringHoldsBytes)
{
    utils_string_t* str = utils_string_create("a\0b", 3);
    char buf[8] = {};

    ASSERT_NE(str, nullptr);
    EXPECT_EQ(utils_string_size(str), 3u);
    EXPECT_EQ(std::memcmp(utils_string_data(str), "a\0b", 3), 0);
    EXPECT_EQ(utils_string_append(str, "cd", 2), UTILS_OK);
    EXPECT_EQ(utils_string_size(str), 5u);
    EXPECT_EQ(utils_string_set(str, "xyz", 3), UTILS_OK);
    EXPECT_EQ(utils_string_copy_r(str, buf, sizeof(buf)), 3u);
    EXPECT_STREQ(buf, "xyz");
    EXPECT_EQ(utils_string_clear(str), UTILS_OK);
    EXPECT_STREQ(utils_string_data(str), "");
    EXPECT_EQ(utils_string_set(str, nullptr, 0), UTILS_OK);
    EXPECT_EQ(utils_string_set(str, nullptr, 2), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The data is NULL");
    utils_string_destroy(str);
    utils_string_destroy(nullptr);
}

TEST_F(SmanipCApiTest, NullStringIsReported)
{
    EXPECT_EQ(utils_string_data(nullptr), nullptr);
    EXPECT_EQ(utils_string_size(nullptr), 0u);
    EXPECT_STREQ(utils_exception.info, "The string is NULL");
}

//----------------------------------------------------------------//
/* FORMAT */

TEST_F(SmanipCApiTest, FormatIsTheCppOne)
{
    const std::string expected = utils::smanip::format("<strong>text<>");
    char buf[64] = {};

    EXPECT_EQ(std::string(utils_smanip_format("<strong>text<>")), expected);
    EXPECT_EQ(utils_smanip_format_r("<strong>text<>", buf, sizeof(buf)), expected.size());
    EXPECT_EQ(std::string(buf), expected);
    EXPECT_EQ(utils_smanip_format(nullptr), nullptr);
    EXPECT_TRUE(utils_exception.exception);
}

//----------------------------------------------------------------//
/* CODEC */

TEST_F(SmanipCApiTest, Base64RoundTrip)
{
    utils_codec_t* codec = utils_codec_create_base64();
    utils_string_t* encoded = utils_string_create(nullptr, 0);
    utils_string_t* decoded = utils_string_create(nullptr, 0);

    ASSERT_NE(codec, nullptr);
    EXPECT_EQ(utils_codec_encode(codec, "hello", 5, encoded), UTILS_OK);
    EXPECT_STREQ(utils_string_data(encoded), utils::smanip::codec::Base64Codec().encode("hello").c_str());
    EXPECT_EQ(utils_codec_decode(codec, utils_string_data(encoded), utils_string_size(encoded), decoded), UTILS_OK);
    EXPECT_STREQ(utils_string_data(decoded), "hello");
    EXPECT_EQ(utils_codec_encode(codec, "x", 1, nullptr), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The output is NULL");
    utils_string_destroy(decoded);
    utils_string_destroy(encoded);
    utils_codec_destroy(codec);
}

TEST_F(SmanipCApiTest, CodecOfTheCSide)
{
    int calls = 0;
    utils_codec_t* codec = utils_codec_create(reverse, reverse, &calls);
    utils_string_t* out = utils_string_create(nullptr, 0);

    ASSERT_NE(codec, nullptr);
    EXPECT_EQ(utils_codec_encode(codec, "abc", 3, out), UTILS_OK);
    EXPECT_STREQ(utils_string_data(out), "cba");
    EXPECT_EQ(utils_codec_decode(codec, "cba", 3, out), UTILS_OK);
    EXPECT_STREQ(utils_string_data(out), "abc");
    EXPECT_EQ(calls, 2);
    EXPECT_EQ(utils_codec_create(reverse, nullptr, nullptr), nullptr);
    EXPECT_STREQ(utils_exception.info, "The decode callback is NULL");
    utils_string_destroy(out);
    utils_codec_destroy(codec);
}

TEST_F(SmanipCApiTest, ErrorOfTheCodecOfTheCSide)
{
    utils_codec_t* codec = utils_codec_create(failing, failing, nullptr);
    utils_string_t* out = utils_string_create(nullptr, 0);

    EXPECT_EQ(utils_codec_encode(codec, "abc", 3, out), UTILS_KO);
    EXPECT_STREQ(utils_exception.what, "codec failed on purpose");
    utils_string_destroy(out);
    utils_codec_destroy(codec);
}

//----------------------------------------------------------------//
/* EETP */

TEST_F(SmanipCApiTest, EETPContent)
{
    utils_eetp_content_t* content = utils_eetp_content_create("A");
    size_t size = 0;

    ASSERT_NE(content, nullptr);
    EXPECT_STREQ(utils_eetp_content_get_type(content), "A");
    EXPECT_EQ(utils_eetp_content_add_data(content, "x\0y", 3), UTILS_OK);
    EXPECT_EQ(utils_eetp_content_data_count(content), 1u);
    EXPECT_EQ(std::memcmp(utils_eetp_content_get_data(content, 0, &size), "x\0y", 3), 0);
    EXPECT_EQ(size, 3u);
    EXPECT_EQ(utils_eetp_content_get_data(content, 1, nullptr), nullptr);
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_eetp_content_set_type(content, "B"), UTILS_OK);
    EXPECT_EQ(utils_eetp_content_clear_data(content), UTILS_OK);
    EXPECT_EQ(utils_eetp_content_data_count(content), 0u);
    utils_eetp_content_destroy(content);
}

TEST_F(SmanipCApiTest, EETPParserWithoutCommonKey)
{
    tests::tools::TempDir home;
    tests::tools::ScopedEnv env("HOME", home.path().string());

    EXPECT_EQ(utils_eetp_parser_create(), nullptr);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::Encryption));
}

TEST_F(SmanipCApiTest, EETPHandshakeAndMessage)
{
    tests::tools::TempDir home;
    tests::tools::ScopedEnv env("HOME", home.path().string());
    utils::security::encryption::RSAKey key;

    std::filesystem::create_directories(home / ".ssh");
    key.generate();
    std::ofstream(home / ".ssh/common", std::ios::binary) << key.get().priv;
    std::ofstream(home / ".ssh/common.pub", std::ios::binary) << key.get().pub;

    utils_eetp_parser_t* client = utils_eetp_parser_create();
    utils_eetp_parser_t* server = utils_eetp_parser_create_with(utils_codec_create_base64(), 1);
    utils_eetp_content_t* content = utils_eetp_content_create(SYN.c_str());
    utils_eetp_content_t* read = utils_eetp_content_create("");
    utils_string_t* framed = utils_string_create(nullptr, 0);
    ASSERT_NE(client, nullptr);
    ASSERT_NE(server, nullptr);
    EXPECT_TRUE(utils_eetp_parser_has_id_overload(client));
    EXPECT_FALSE(utils_eetp_parser_has_no_id_overload(client));

    // handshake
    ASSERT_EQ(utils_eetp_parser_format(client, "server", content, framed), UTILS_OK);
    ASSERT_EQ(utils_eetp_parser_parse(server, "client", utils_string_data(framed), utils_string_size(framed), read), UTILS_OK);
    EXPECT_EQ(std::string(utils_eetp_content_get_type(read)), SYN);
    EXPECT_EQ(utils_eetp_content_set_type(content, SO.c_str()), UTILS_OK);
    ASSERT_EQ(utils_eetp_parser_format(server, "client", content, framed), UTILS_OK);
    ASSERT_EQ(utils_eetp_parser_parse(client, "server", utils_string_data(framed), utils_string_size(framed), read), UTILS_OK);

    // message
    EXPECT_EQ(utils_eetp_content_set_type(content, "A"), UTILS_OK);
    EXPECT_EQ(utils_eetp_content_add_data(content, "hello", 5), UTILS_OK);
    ASSERT_EQ(utils_eetp_parser_format(client, "server", content, framed), UTILS_OK);
    ASSERT_EQ(utils_eetp_parser_parse(server, "client", utils_string_data(framed), utils_string_size(framed), read), UTILS_OK);
    EXPECT_STREQ(utils_eetp_content_get_type(read), "A");
    ASSERT_EQ(utils_eetp_content_data_count(read), 1u);
    EXPECT_STREQ(utils_eetp_content_get_data(read, 0, nullptr), "hello");

    utils_string_destroy(framed);
    utils_eetp_content_destroy(read);
    utils_eetp_content_destroy(content);
    utils_eetp_parser_destroy(server);
    utils_eetp_parser_destroy(client);
}

TEST_F(SmanipCApiTest, EETPParserKeepsTheCodecOnError)
{
    tests::tools::TempDir home;
    tests::tools::ScopedEnv env("HOME", home.path().string());
    utils_codec_t* codec = utils_codec_create_base64();

    EXPECT_EQ(utils_eetp_parser_create_with(codec, 1), nullptr); // no common key: the codec is still ours
    EXPECT_EQ(utils_eetp_parser_set_codec(nullptr, codec), UTILS_KO);
    utils_codec_destroy(codec);
}
