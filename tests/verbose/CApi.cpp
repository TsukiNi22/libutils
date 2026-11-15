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
##  Unit tests of the C API of utils::verbose (level, logs, locked callbacks)
\**************************************************************/

#include "utils/verbose/CApi.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <cstdint>
#include <cstdio>
#include <string>

class VerboseCApiTest: public ::testing::Test {
    protected:
        utils_verbose_t _old = C_API_VERBOSE_NONE;

        void SetUp(void) override
        {
            this->_old = utils_verbose_get();
            utils_exception_clear();
        };

        void TearDown(void) override
        {
            utils_verbose_set(this->_old);
        };
};

/* callbacks */
static int count(void* data)
{
    ++*static_cast<int*>(data);
    return UTILS_OK;
}

static int fail(_unused void* data)
{
    return utils_fail(42, "callback failed");
}

static int failNoInfo(_unused void* data)
{
    return UTILS_KO;
}

//----------------------------------------------------------------//
/* LEVEL */

TEST_F(VerboseCApiTest, SetGetAndIs)
{
    utils_verbose_set(C_API_VERBOSE_ADVANCED);
    EXPECT_EQ(utils_verbose_get(), C_API_VERBOSE_ADVANCED);
    EXPECT_EQ(utils::verbose::verbose.load(), utils::verbose::Verbose::Advanced);
    EXPECT_TRUE(utils_verbose_is(C_API_VERBOSE_BASIC));
    EXPECT_TRUE(utils_verbose_is(C_API_VERBOSE_ADVANCED));
    EXPECT_FALSE(utils_verbose_is(C_API_VERBOSE_DEBUG));
    EXPECT_FALSE(utils_exception.exception);
}

TEST_F(VerboseCApiTest, InvalidLevelIsReported)
{
    utils_verbose_set(C_API_VERBOSE_BASIC);
    utils_verbose_set(static_cast<utils_verbose_t>(42));
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::InvalidArgument));
    EXPECT_EQ(utils_verbose_get(), C_API_VERBOSE_BASIC);
    EXPECT_FALSE(utils_verbose_is(static_cast<utils_verbose_t>(42)));
    EXPECT_TRUE(utils_exception.exception);
}

//----------------------------------------------------------------//
/* LOG */

TEST_F(VerboseCApiTest, LogOnlyWhenDisplayed)
{
    utils_verbose_set(C_API_VERBOSE_BASIC);
    testing::internal::CaptureStdout();
    EXPECT_EQ(utils_verbose_log(C_API_VERBOSE_BASIC, "value %d of %s", 7, "test"), UTILS_OK);
    EXPECT_EQ(utils_verbose_log(C_API_VERBOSE_ADVANCED, "hidden"), UTILS_OK);
    utils_verbose_set(C_API_VERBOSE_DEBUG);
    EXPECT_EQ(utils_verbose_log(C_API_VERBOSE_DEBUG, "dbg"), UTILS_OK);
    EXPECT_EQ(testing::internal::GetCapturedStdout(), "value 7 of test\ndebug: dbg\n");
}

TEST_F(VerboseCApiTest, LogErrors)
{
    utils_verbose_set(C_API_VERBOSE_BASIC);
    EXPECT_EQ(utils_verbose_log(C_API_VERBOSE_BASIC, nullptr), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The format is NULL");
    EXPECT_EQ(utils_verbose_log_to(nullptr, C_API_VERBOSE_BASIC, "x"), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The output is NULL");
}

TEST_F(VerboseCApiTest, LogToAFile)
{
    FILE* file = std::tmpfile();
    char buf[64] = {};

    ASSERT_NE(file, nullptr);
    utils_verbose_set(C_API_VERBOSE_ADVANCED);
    EXPECT_EQ(utils_verbose_log_to(file, C_API_VERBOSE_ADVANCED, "%s=%u", "key", 3u), UTILS_OK);
    EXPECT_EQ(utils_verbose_log_to(file, C_API_VERBOSE_DEBUG, "hidden"), UTILS_OK);
    std::rewind(file);
    EXPECT_EQ(std::fread(buf, 1, sizeof(buf) - 1, file), 6u);
    EXPECT_STREQ(buf, "key=3\n");
    std::fclose(file);
}

//----------------------------------------------------------------//
/* LOCKED */

TEST_F(VerboseCApiTest, LockedRunsTheCallback)
{
    int calls = 0;

    EXPECT_EQ(utils_verbose_locked(count, &calls), UTILS_OK);
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(utils_verbose_locked(nullptr, nullptr), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The callback is NULL");
}

TEST_F(VerboseCApiTest, LockedKeepsTheErrorOfTheCallback)
{
    EXPECT_EQ(utils_verbose_locked(fail, nullptr), UTILS_KO);
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_C);
    EXPECT_EQ(utils_exception.code, 42u);
    EXPECT_STREQ(utils_exception.info, "callback failed");
    EXPECT_EQ(utils_verbose_locked(failNoInfo, nullptr), UTILS_KO);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_C);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(C_API_OTHER_EXCEPTION_CODE));
}

TEST_F(VerboseCApiTest, OnOnlyWhenDisplayed)
{
    int calls = 0;

    utils_verbose_set(C_API_VERBOSE_BASIC);
    EXPECT_EQ(utils_verbose_on(C_API_VERBOSE_DEBUG, count, &calls), UTILS_OK);
    EXPECT_EQ(calls, 0);
    EXPECT_EQ(utils_verbose_on(C_API_VERBOSE_BASIC, count, &calls), UTILS_OK);
    EXPECT_EQ(calls, 1);
}
