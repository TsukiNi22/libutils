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
##  Unit tests of the exception status of the C API (guard, origin, utils_fail, reset, format)
\**************************************************************/

#include "utils/exception/CApi.hpp"
#include "utils/exception/CApi.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <string>

class ExceptionCApiTest: public ::testing::Test {
    protected:
        void SetUp(void) override
        {
            utils_exception_clear();
        };
};

//----------------------------------------------------------------//
/* GUARD */

TEST_F(ExceptionCApiTest, GuardSuccessOnlyResetsTheFlag)
{
    EXPECT_FALSE(utils::exception::c_api::guard([](void) {throw std::runtime_error("boom");}));
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_TRUE(utils::exception::c_api::guard([](void) { /* Nothing */ }));
    EXPECT_FALSE(utils_exception.exception);
    EXPECT_STREQ(utils_exception.what, "boom"); // no full reset on success
}

TEST_F(ExceptionCApiTest, GuardLibutilsExceptionIsInternal)
{
    EXPECT_FALSE(utils::exception::c_api::guard([](void) {throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "bad");}));
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_INTERNAL);
    EXPECT_EQ(utils_exception.type, C_API_EXCEPTION_TYPE_ERROR);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::InvalidArgument));
    EXPECT_STREQ(utils_exception.info, "bad");
    EXPECT_NE(std::strstr(utils_exception.file, "CApi.cpp"), nullptr);
    EXPECT_GT(utils_exception.line, 0u);
}

TEST_F(ExceptionCApiTest, GuardStdExceptionIsSystem)
{
    EXPECT_FALSE(utils::exception::c_api::guard([](void) {throw std::out_of_range("range");}));
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_STD);
    EXPECT_EQ(utils_exception.type, C_API_EXCEPTION_TYPE_SYSTEM);
    EXPECT_EQ(utils_exception.code, C_API_OTHER_EXCEPTION_CODE);
    EXPECT_STREQ(utils_exception.what, "range");
    EXPECT_STREQ(utils_exception.file, "");
}

TEST_F(ExceptionCApiTest, GuardUnknownException)
{
    EXPECT_FALSE(utils::exception::c_api::guard([](void) {throw 42;}));
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_UNKNOWN);
    EXPECT_EQ(utils_exception.code, C_API_OTHER_EXCEPTION_CODE);
    EXPECT_STREQ(utils_exception.what, "Unknown exception");
}

TEST_F(ExceptionCApiTest, GuardFallbackReturnsTheResultOrTheFallback)
{
    EXPECT_EQ(utils::exception::c_api::guard_fallback(-1, [](void) -> int {return 7;}), 7);
    EXPECT_FALSE(utils_exception.exception);
    EXPECT_EQ(utils::exception::c_api::guard_fallback(-1, [](void) -> int {throw std::runtime_error("no");}), -1);
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils::exception::c_api::guard_fallback(nullptr, [](void) -> const char* {throw std::runtime_error("no");}), nullptr);
}

TEST_F(ExceptionCApiTest, NotNullGivesTheEmplacementOfTheCaller)
{
    int value = 3;
    int* null = nullptr;

    EXPECT_EQ(utils::exception::c_api::not_null(&value, "value"), &value);
    const bool ok = utils::exception::c_api::guard([&](void) {(void)utils::exception::c_api::not_null(null, "value");}); const std::uint64_t line = __LINE__;
    EXPECT_FALSE(ok);
    EXPECT_STREQ(utils_exception.info, "The value is NULL");
    EXPECT_EQ(utils_exception.line, line);
    EXPECT_NE(std::strstr(utils_exception.file, "exception/CApi.cpp"), nullptr);
}

//----------------------------------------------------------------//
/* C SIDE */

TEST_F(ExceptionCApiTest, SetFromCWithTheEmplacement)
{
    const int ret = utils_exception_set(C_API_EXCEPTION_TYPE_WARNING, 7, "my warning", "details"); const std::uint64_t line = __LINE__;

    EXPECT_EQ(ret, UTILS_KO);
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_C);
    EXPECT_EQ(utils_exception.type, C_API_EXCEPTION_TYPE_WARNING);
    EXPECT_EQ(utils_exception.code, 7u);
    EXPECT_STREQ(utils_exception.what, "my warning");
    EXPECT_STREQ(utils_exception.info, "details");
    EXPECT_EQ(utils_exception.line, line);
    EXPECT_STREQ(utils_exception.module, "");
}

TEST_F(ExceptionCApiTest, FailIsAnError)
{
    EXPECT_EQ(utils_fail(42, "failed"), UTILS_KO);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_C);
    EXPECT_EQ(utils_exception.type, C_API_EXCEPTION_TYPE_ERROR);
    EXPECT_EQ(utils_exception.code, 42u);
    EXPECT_STREQ(utils_exception.what, "C error");
    EXPECT_STREQ(utils_exception.info, "failed");
}

TEST_F(ExceptionCApiTest, StringsAreTruncated)
{
    const std::string longInfo(C_API_MAX_CHAR_EXCEPTION_INFORMATION * 2, 'a');

    (void)utils_fail(1, longInfo.c_str());
    EXPECT_EQ(std::strlen(utils_exception.info), static_cast<std::size_t>(C_API_MAX_CHAR_EXCEPTION_INFORMATION - 1));
    EXPECT_STREQ(utils_exception.info + C_API_MAX_CHAR_EXCEPTION_INFORMATION - 4, "...");
}

TEST_F(ExceptionCApiTest, ClearResetsEverything)
{
    (void)utils_fail(1, "x");
    utils_exception_clear();
    EXPECT_FALSE(utils_exception.exception);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_NONE);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(C_API_NO_EXCEPTION));
    EXPECT_STREQ(utils_exception.info, "");
    EXPECT_EQ(utils_exception.line, 0u);
}

//----------------------------------------------------------------//
/* CALLBACK */

TEST_F(ExceptionCApiTest, ResetStatusOnlyResetsTheFlag)
{
    (void)utils_fail(1, "old error");
    utils::exception::c_api::reset_status();
    EXPECT_FALSE(utils_exception.exception);
    EXPECT_EQ(utils_exception.code, 1u); // the fields are kept
    EXPECT_STREQ(utils_exception.info, "old error");
}

//----------------------------------------------------------------//
/* FORMAT */

TEST_F(ExceptionCApiTest, FormatIsEmptyWithoutException)
{
    char buf[8] = "x";

    EXPECT_STREQ(utils_exception_format(), "");
    EXPECT_EQ(utils_exception_format_r(buf, sizeof(buf)), 0u);
    EXPECT_STREQ(buf, "");
}

TEST_F(ExceptionCApiTest, FormatLikeFormatedAndKeepsTheStatus)
{
    (void)utils::exception::c_api::guard([](void) {throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "bad");});
    const std::string formated = utils_exception_format();

    EXPECT_NE(formated.find("Error"), std::string::npos);
    EXPECT_NE(formated.find(utils_exception.what), std::string::npos);
    EXPECT_NE(formated.find("bad"), std::string::npos);
    EXPECT_TRUE(utils_exception.exception); // reading the status doesn't change it
}

TEST_F(ExceptionCApiTest, FormatROfTheCSideLikeSnprintf)
{
    char buf[8];

    (void)utils_fail(3, "info of the C side");
    const std::string formated = utils_exception_format();
    EXPECT_NE(formated.find("info of the C side"), std::string::npos);
    EXPECT_EQ(utils_exception_format_r(buf, sizeof(buf)), formated.size());
    EXPECT_EQ(std::strlen(buf), sizeof(buf) - 1);
    EXPECT_EQ(utils_exception_format_r(nullptr, 0), formated.size());
    EXPECT_EQ(utils_exception_format_r(nullptr, 4), 0u);
    EXPECT_TRUE(utils_exception.exception);
}
