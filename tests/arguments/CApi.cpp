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
##  Unit tests of the C API of utils::arguments (ArgParser, Settings)
\**************************************************************/

#include "utils/arguments/CApi.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <string>

class ArgumentsCApiTest: public ::testing::Test {
    protected:
        utils_argparser_t* _parser = nullptr;

        void SetUp(void) override
        {
            const utils_argparser_flag_option_t value = {"value", true, utils_argparser_check_int32, nullptr};
            const utils_argparser_flag_option_t pair = {"value", true, even, nullptr};
            const char* ids[] = {"number", "verbose", "pair", "even"};
            const bool mandatory[] = {true, false, false, false};

            utils_exception_clear();
            this->_parser = utils_argparser_create("bin", "A test binary");
            ASSERT_NE(this->_parser, nullptr);
            ASSERT_EQ(utils_argparser_set_flag(this->_parser, "number", "n", "num", "number", nullptr, &value, 1, "A number", false, false, false), UTILS_OK);
            ASSERT_EQ(utils_argparser_set_flag(this->_parser, "verbose", "v", nullptr, "verbose", nullptr, nullptr, 0, nullptr, false, false, false), UTILS_OK);
            ASSERT_EQ(utils_argparser_set_flag(this->_parser, "pair", "p", nullptr, nullptr, nullptr, &pair, 1, "An even number", false, false, false), UTILS_OK);
            ASSERT_EQ(utils_argparser_set_option(this->_parser, "even", "EVEN", even, nullptr, "An even number", false), UTILS_OK);
            ASSERT_EQ(utils_argparser_set_usage(this->_parser, "main", "main", false, ids, mandatory, 4, "Main usage", false), UTILS_OK);
        };

        void TearDown(void) override
        {
            utils_argparser_destroy(this->_parser);
        };

        // Check of the C side: an even number
        static int even(const char* option, _unused void* data)
        {
            if (std::strlen(option) > 0 && (option[std::strlen(option) - 1] - '0') % 2 == 0) return UTILS_OK;
            return utils_fail(7, "not an even number");
        };
};

/* help hook of the C side */
static int help(const utils_argparser_t* parser, void* data)
{
    *static_cast<std::string*>(data) = utils_argparser_get_binary(parser);
    return UTILS_OK;
}

//----------------------------------------------------------------//
/* ARGPARSER */

TEST_F(ArgumentsCApiTest, ParseFlagsAndOptions)
{
    const char* argv[] = {"bin", "-n", "42", "-v", "8"};
    utils_argparser_result_t* result = utils_argparser_parse(this->_parser, 5, argv, false);
    bool number = false;

    ASSERT_NE(result, nullptr) << utils_exception.info;
    ASSERT_EQ(utils_argparser_result_count(result), 1u);
    EXPECT_STREQ(utils_argparser_result_usage(result, 0), "main");
    for (size_t j = 0; j < utils_argparser_result_arguments_count(result, 0); ++j) {
        if (std::strcmp(utils_argparser_result_argument_id(result, 0, j), "number") != 0) continue;
        number = true;
        EXPECT_FALSE(utils_argparser_result_argument_is_option(result, 0, j));
        ASSERT_EQ(utils_argparser_result_values_count(result, 0, j), 1u);
        EXPECT_STREQ(utils_argparser_result_value(result, 0, j, 0), "42");
    }
    EXPECT_TRUE(number);
    EXPECT_EQ(utils_argparser_result_value(result, 0, 0, 9), nullptr);
    utils_argparser_result_destroy(result);
}

TEST_F(ArgumentsCApiTest, ChecksOfTheCAndCppSides)
{
    const char* notInt[] = {"bin", "-n", "abc"};
    const char* notEven[] = {"bin", "-n", "1", "-p", "7"};

    EXPECT_EQ(utils_argparser_parse(this->_parser, 3, notInt, false), nullptr);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::FlagOption));
    EXPECT_EQ(utils_argparser_parse(this->_parser, 5, notEven, false), nullptr);
    EXPECT_NE(std::string(utils_exception.info).find("not an even number"), std::string::npos) << utils_exception.info;
    EXPECT_EQ(utils_argparser_check_int32("12", nullptr), UTILS_OK);
    EXPECT_EQ(utils_argparser_check_int32("x", nullptr), UTILS_KO);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_C);
    EXPECT_EQ(utils_argparser_check_true(nullptr, nullptr), UTILS_KO);
}

TEST_F(ArgumentsCApiTest, GettersAndHooks)
{
    const char* short_flag = nullptr;
    const char* name = nullptr;
    bool mandatory = false;
    std::string called;

    EXPECT_STREQ(utils_argparser_get_binary(this->_parser), "bin");
    EXPECT_STREQ(utils_argparser_get_description(this->_parser), "A test binary");
    EXPECT_EQ(utils_argparser_flags_count(this->_parser), 3u);
    EXPECT_EQ(utils_argparser_options_count(this->_parser), 1u);
    EXPECT_EQ(utils_argparser_usages_count(this->_parser), 1u);
    EXPECT_STREQ(utils_argparser_usage_id(this->_parser, 0), "main");
    EXPECT_EQ(utils_argparser_usage_ids_count(this->_parser, "main"), 4u);
    EXPECT_EQ(utils_argparser_get_flag(this->_parser, "number", &short_flag, nullptr, nullptr, nullptr, nullptr, nullptr), UTILS_OK);
    EXPECT_STREQ(short_flag, "n");
    EXPECT_EQ(utils_argparser_flag_get_option(this->_parser, "number", 0, &name, &mandatory), UTILS_OK);
    EXPECT_STREQ(name, "value");
    EXPECT_TRUE(mandatory);
    EXPECT_EQ(utils_argparser_get_flag(this->_parser, "unknown", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr), UTILS_KO);
    EXPECT_EQ(utils_argparser_set_help_hook(this->_parser, help, &called), UTILS_OK);
    EXPECT_EQ(utils_argparser_help(this->_parser), UTILS_OK);
    EXPECT_EQ(called, "bin");
    EXPECT_EQ(utils_argparser_set_flag(this->_parser, "verbose", "V", nullptr, nullptr, nullptr, nullptr, 0, nullptr, false, false, false), UTILS_KO); // already there
    EXPECT_EQ(utils_argparser_set_flag(this->_parser, "verbose", "V", nullptr, nullptr, nullptr, nullptr, 0, nullptr, false, false, true), UTILS_OK);
    EXPECT_EQ(utils_argparser_remove_flag(this->_parser, "verbose"), UTILS_OK);
    EXPECT_EQ(utils_argparser_flags_count(this->_parser), 2u);
    EXPECT_EQ(utils_argparser_reset_flags(this->_parser), UTILS_OK);
    EXPECT_EQ(utils_argparser_flags_count(this->_parser), 0u);
}

//----------------------------------------------------------------//
/* SETTINGS */

TEST_F(ArgumentsCApiTest, SettingsOfTheFixedTypes)
{
    utils_settings_t* settings = utils_settings_create();
    int32_t i = 0;
    const char* str = nullptr;
    double f = 0.0;

    ASSERT_NE(settings, nullptr);
    EXPECT_EQ(utils_settings_add_i32(settings, "count", 42), UTILS_OK);
    EXPECT_EQ(utils_settings_add_i32(settings, "count", 1), UTILS_KO); // already there
    EXPECT_EQ(utils_settings_get_i32(settings, "count", &i), UTILS_OK);
    EXPECT_EQ(i, 42);
    EXPECT_TRUE(utils_settings_is_i32(settings, "count"));
    EXPECT_FALSE(utils_settings_is_i64(settings, "count"));
    EXPECT_EQ(utils_settings_get_type(settings, "count"), C_API_SETTINGS_CAST_INT32);
    EXPECT_EQ(utils_settings_set_str(settings, "name", "libutils", true), UTILS_OK);
    EXPECT_EQ(utils_settings_get_str(settings, "name", &str), UTILS_OK);
    EXPECT_STREQ(str, "libutils");
    EXPECT_EQ(utils_settings_get_type(settings, "name"), C_API_SETTINGS_CAST_NONE);
    EXPECT_EQ(utils_settings_get_f64(settings, "name", &f), UTILS_KO);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::BadCast));
    EXPECT_EQ(utils_settings_get_i32(settings, "unknown", &i), UTILS_KO);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::UnknownId));
    EXPECT_TRUE(utils_settings_contains(settings, "name"));
    EXPECT_EQ(utils_settings_remove(settings, "name", false), UTILS_OK);
    EXPECT_EQ(utils_settings_remove(settings, "name", false), UTILS_KO);
    EXPECT_EQ(utils_settings_remove(settings, "name", true), UTILS_OK);
    EXPECT_EQ(utils_settings_clear(settings), UTILS_OK);
    EXPECT_FALSE(utils_settings_contains(settings, "count"));
    utils_settings_destroy(settings);
}

TEST_F(ArgumentsCApiTest, SettingsCasts)
{
    utils_settings_t* settings = utils_settings_create();
    utils_settings_cast_t type = C_API_SETTINGS_CAST_OTHER;
    uint64_t u = 0;
    bool b = false;
    uint8_t byte = 0;
    const char* path = nullptr;

    EXPECT_EQ(utils_settings_auto_cast(settings, "flag", "true", false, &type), UTILS_OK);
    EXPECT_EQ(type, C_API_SETTINGS_CAST_BOOL);
    EXPECT_EQ(utils_settings_get_bool(settings, "flag", &b), UTILS_OK);
    EXPECT_TRUE(b);
    EXPECT_EQ(utils_settings_cast(settings, "big", "123", C_API_SETTINGS_CAST_UINT64, false), UTILS_OK);
    EXPECT_EQ(utils_settings_get_u64(settings, "big", &u), UTILS_OK);
    EXPECT_EQ(u, 123u);
    EXPECT_EQ(utils_settings_cast(settings, "bad", "x", static_cast<utils_settings_cast_t>(77), false), UTILS_KO);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::UnknownCast));
    EXPECT_EQ(utils_settings_set_byte(settings, "byte", 0xAB, false), UTILS_OK);
    EXPECT_EQ(utils_settings_get_byte(settings, "byte", &byte), UTILS_OK);
    EXPECT_EQ(byte, 0xAB);
    EXPECT_EQ(utils_settings_get_type(settings, "byte"), C_API_SETTINGS_CAST_BYTE);
    EXPECT_EQ(utils_settings_cast(settings, "dir", "/tmp", C_API_SETTINGS_CAST_PATH, false), UTILS_OK);
    EXPECT_EQ(utils_settings_get_path(settings, "dir", &path), UTILS_OK);
    EXPECT_STREQ(path, "/tmp");
    EXPECT_TRUE(utils_settings_is_path(settings, "dir"));
    utils_settings_destroy(settings);
}

TEST_F(ArgumentsCApiTest, SettingsStringsAndBytes)
{
    utils_settings_t* settings = utils_settings_create();
    const uint_least16_t u16[] = {0x48, 0x49};
    const uint_least16_t* outU16 = nullptr;
    const wchar_t* outW = nullptr;
    const void* data = nullptr;
    size_t size = 0;

    EXPECT_EQ(utils_settings_set_u16string(settings, "u16", u16, 2, false), UTILS_OK);
    EXPECT_EQ(utils_settings_get_u16string(settings, "u16", &outU16, &size), UTILS_OK);
    EXPECT_EQ(size, 2u);
    EXPECT_EQ(outU16[1], 0x49);
    EXPECT_EQ(utils_settings_get_type(settings, "u16"), C_API_SETTINGS_CAST_U16STRING);
    EXPECT_EQ(utils_settings_set_wstring(settings, "w", L"wide", 4, false), UTILS_OK);
    EXPECT_EQ(utils_settings_get_wstring(settings, "w", &outW, nullptr), UTILS_OK);
    EXPECT_EQ(std::wstring(outW), L"wide");
    EXPECT_EQ(utils_settings_set_bytes(settings, "raw", "a\0b", 3, false), UTILS_OK);
    EXPECT_EQ(utils_settings_get_bytes(settings, "raw", &data, &size), UTILS_OK);
    EXPECT_EQ(size, 3u);
    EXPECT_EQ(std::memcmp(data, "a\0b", 3), 0);
    EXPECT_TRUE(utils_settings_is_bytes(settings, "raw"));
    EXPECT_EQ(utils_settings_get_type(settings, "raw"), C_API_SETTINGS_CAST_OTHER);
    utils_settings_destroy(settings);
}
