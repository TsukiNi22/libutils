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
##  Unit tests of the C API of the Cli (handle, status, commands, callbacks, utils_fail, strings, parsed data)
\**************************************************************/

#include "tools/TempDir.hpp"
#include "utils/cli/CApi.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

/* commands and callbacks of the C side (data: std::vector<std::string>* of the calls) */
static int record(const utils_cli_t*, size_t argc, const char* const* argv, void* data)
{
    std::vector<std::string>* calls = static_cast<std::vector<std::string>*>(data);
    std::string call = argv[0];

    for (std::size_t i = 1; i < argc; ++i)
        call += " " + std::string(argv[i]);
    calls->push_back(call);
    return (argv[argc] == nullptr ? UTILS_OK : UTILS_KO); // argv[argc] = NULL like main
}

static int raw(const utils_cli_t*, const char* input, void* data)
{
    static_cast<std::vector<std::string>*>(data)->push_back(input);
    return UTILS_OK;
}

static int fail(const utils_cli_t*, size_t, const char* const*, void*)
{
    return utils_fail(42, "fail on purpose");
}

static int stop(const utils_cli_t*, size_t, const char* const*, void*)
{
    return C_API_CLI_EXIT;
}

static int eof(char* c, void*)
{
    *c = '\x04'; // never read the real stdin
    return UTILS_OK;
}

static int failingMiddleware(const char*, void*)
{
    return utils_fail(9, "middleware failed");
}

class CliCApiTest: public ::testing::Test {
    protected:
        tests::tools::TempDir _home;
        std::unique_ptr<tests::tools::ScopedEnv> _env;
        utils_cli_t* _cli = nullptr;
        std::vector<std::string> _calls;

        void SetUp(void) override
        {
            this->_env = std::make_unique<tests::tools::ScopedEnv>("HOME", this->_home.path().string());
            this->_cli = utils_cli_create(false);
            ASSERT_NE(this->_cli, nullptr);
            ASSERT_EQ(utils_cli_set_getc_hook(this->_cli, eof, nullptr), UTILS_OK);
            utils_cli_set_flags(this->_cli, C_API_CLI_FLAG_NO_TTY | C_API_CLI_FLAG_PARSED | C_API_CLI_FLAG_TRIM | C_API_CLI_FLAG_NOECHO);
            ASSERT_EQ(utils_cli_set_parsed_command(this->_cli, "record", record, 0, C_API_CLI_NO_LIMIT, &this->_calls, false), UTILS_OK);
            ASSERT_EQ(utils_cli_set_parsed_command(this->_cli, "fail", fail, 0, 0, nullptr, false), UTILS_OK);
            ASSERT_EQ(utils_cli_set_parsed_command(this->_cli, "stop", stop, C_API_CLI_NO_LIMIT, C_API_CLI_NO_LIMIT, nullptr, false), UTILS_OK);
            utils_exception_clear();
        };
        void TearDown(void) override
        {
            utils_cli_destroy(this->_cli);
            this->_env.reset();
        };
        int run(const std::vector<const char*>& inputs) {return utils_cli_start(this->_cli, inputs.data(), inputs.size(), 0);}; // inputs then until stop / EOF
};

//----------------------------------------------------------------//
/* HANDLE & STATUS */

TEST_F(CliCApiTest, DestroyNullIsNoop)
{
    utils_cli_destroy(nullptr);
    EXPECT_FALSE(utils_exception.exception);
}

TEST_F(CliCApiTest, NullHandleFillsTheStatusEverywhere)
{
    char buf[4];

    EXPECT_EQ(utils_cli_start(nullptr, nullptr, 0, 1), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The cli handle is NULL");

    // void functions: only the status tells
    utils_exception_clear();
    utils_cli_interrupt(nullptr);
    EXPECT_TRUE(utils_exception.exception);
    utils_exception_clear();
    utils_cli_kill(nullptr);
    EXPECT_TRUE(utils_exception.exception);
    utils_exception_clear();
    utils_cli_join(nullptr);
    EXPECT_TRUE(utils_exception.exception);
    utils_exception_clear();
    utils_cli_set_flags(nullptr, C_API_CLI_FLAGS_DEFAULT);
    EXPECT_TRUE(utils_exception.exception);
    utils_exception_clear();
    utils_cli_set_input_delimitor(nullptr, ';');
    EXPECT_TRUE(utils_exception.exception);

    // getters: the fallback value + the status
    EXPECT_FALSE(utils_cli_is_running(nullptr));
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_cli_get_flags(nullptr), 0u);
    EXPECT_EQ(utils_cli_get_input_delimitor(nullptr), '\0');
    EXPECT_EQ(utils_cli_history_size(nullptr), 0u);
    EXPECT_EQ(utils_cli_strcode(nullptr, 0), nullptr);
    EXPECT_EQ(utils_cli_strcode_r(nullptr, 0, buf, sizeof(buf)), 0u);
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_INTERNAL);
    EXPECT_NE(std::strstr(utils_exception.function, "utils_cli_strcode_r"), nullptr); // emplacement of the C API function
}

TEST_F(CliCApiTest, SuccessResetsTheFlag)
{
    utils_cli_interrupt(nullptr);
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_cli_get_flags(this->_cli), static_cast<std::uint32_t>(C_API_CLI_FLAG_NO_TTY | C_API_CLI_FLAG_PARSED | C_API_CLI_FLAG_TRIM | C_API_CLI_FLAG_NOECHO));
    EXPECT_FALSE(utils_exception.exception);
}

//----------------------------------------------------------------//
/* COMMANDS */

TEST_F(CliCApiTest, CommandsAreCalledLikeMain)
{
    EXPECT_EQ(this->run({"record a b", "record", "stop"}), UTILS_OK);
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"record a b", "record"}));
    EXPECT_TRUE(utils_cli_was_stopped(this->_cli));
    EXPECT_EQ(utils_cli_history_size(this->_cli), 3u);
    EXPECT_STREQ(utils_cli_history_get(this->_cli, 0), "record a b");
}

TEST_F(CliCApiTest, RawCommands)
{
    utils_cli_sub_flags(this->_cli, C_API_CLI_FLAG_PARSED);
    ASSERT_EQ(utils_cli_set_raw_command(this->_cli, "say", raw, &this->_calls, false), UTILS_OK);
    EXPECT_EQ(this->run({"say hello world"}), UTILS_OK);
    ASSERT_EQ(this->_calls.size(), 1u);
    EXPECT_NE(this->_calls[0].find("hello world"), std::string::npos);
}

TEST_F(CliCApiTest, ExistingCommandNeedsForce)
{
    EXPECT_EQ(utils_cli_set_parsed_command(this->_cli, "record", record, 0, 0, &this->_calls, false), UTILS_KO);
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_cli_set_parsed_command(this->_cli, "record", record, 0, 0, &this->_calls, true), UTILS_OK);
    EXPECT_FALSE(utils_exception.exception);
}

TEST_F(CliCApiTest, NullCommandNameIsRefused)
{
    EXPECT_EQ(utils_cli_set_parsed_command(this->_cli, nullptr, record, 0, 0, nullptr, false), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The command name is NULL");
}

//----------------------------------------------------------------//
/* UTILS_FAIL */

TEST_F(CliCApiTest, FailInACommandReachesTheCaller)
{
    // No CATCH: the error stops the cli, wrapped by the Cli (the info of utils_fail is in its message)
    EXPECT_EQ(this->run({"fail"}), UTILS_KO);
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_INTERNAL);
    EXPECT_NE(std::strstr(utils_exception.info, "fail on purpose"), nullptr);
}

TEST_F(CliCApiTest, FailCaughtByTheCliIsNotAnErrorOfStart)
{
    utils_cli_add_flags(this->_cli, C_API_CLI_FLAG_CATCH);
    EXPECT_EQ(this->run({"fail", "record x", "stop"}), UTILS_OK);
    EXPECT_FALSE(utils_exception.exception);
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"record x"}));
}

TEST_F(CliCApiTest, FailInAMiddleware)
{
    // Handled (printed) by the Cli: start succeeds, the info of the C error stays readable
    ASSERT_EQ(utils_cli_command_middlewares_add_before(this->_cli, failingMiddleware, nullptr), UTILS_OK);
    EXPECT_EQ(this->run({"record x", "stop"}), UTILS_OK);
    EXPECT_FALSE(utils_exception.exception);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_C);
    EXPECT_EQ(utils_exception.code, 9u);
    EXPECT_STREQ(utils_exception.info, "middleware failed");
    EXPECT_TRUE(this->_calls.empty());
}

TEST_F(CliCApiTest, NullMiddlewareIsRefused)
{
    EXPECT_EQ(utils_cli_exec_middlewares_add_after(this->_cli, nullptr, nullptr), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The middleware is NULL");
}

//----------------------------------------------------------------//
/* STRINGS */

TEST_F(CliCApiTest, StrcodeAndHistoryBounds)
{
    char buf[4];
    const std::size_t len = std::strlen(utils_cli_strcode(this->_cli, 0));

    EXPECT_GT(len, 0u);
    EXPECT_EQ(utils_cli_strcode_r(this->_cli, 0, buf, sizeof(buf)), len);
    EXPECT_EQ(std::strlen(buf), std::min(len, sizeof(buf) - 1));
    EXPECT_EQ(utils_cli_history_get(this->_cli, 99), nullptr);
    EXPECT_EQ(utils_exception.origin, C_API_EXCEPTION_ORIGIN_INTERNAL);
    EXPECT_STREQ(utils_exception.info, "History index out of range");
    EXPECT_EQ(utils_cli_strcode_r(this->_cli, 0, nullptr, 4), 0u);
    EXPECT_TRUE(utils_exception.exception);
}

//----------------------------------------------------------------//
/* PARSED DATA */

TEST_F(CliCApiTest, ParsedData)
{
    utils_cli_parsed_t* parsed = utils_cli_parsed_create();

    ASSERT_NE(parsed, nullptr);
    EXPECT_EQ(utils_cli_parsed_add_arg(parsed, "x"), UTILS_KO); // no group yet
    EXPECT_EQ(utils_cli_parsed_add_group(parsed), UTILS_OK);
    EXPECT_EQ(utils_cli_parsed_add_arg(parsed, "cmd"), UTILS_OK);
    EXPECT_EQ(utils_cli_parsed_add_arg(parsed, "arg"), UTILS_OK);
    EXPECT_EQ(utils_cli_parsed_groups(parsed), 1u);
    EXPECT_EQ(utils_cli_parsed_size(parsed, 0), 2u);
    EXPECT_STREQ(utils_cli_parsed_get(parsed, 0, 1), "arg");
    EXPECT_EQ(utils_cli_parsed_get(parsed, 3, 0), nullptr);
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_cli_parsed_groups(nullptr), 0u);
    EXPECT_STREQ(utils_exception.info, "The parsed data is NULL");
    EXPECT_EQ(utils_cli_parsed_clear(parsed), UTILS_OK);
    EXPECT_EQ(utils_cli_parsed_groups(parsed), 0u);
    utils_cli_parsed_destroy(parsed);
    utils_cli_parsed_destroy(nullptr);
    EXPECT_FALSE(utils_exception.exception);
}

TEST_F(CliCApiTest, DefaultParserHookFromC)
{
    utils_cli_parsed_t* parsed = utils_cli_parsed_create();

    ASSERT_NE(parsed, nullptr);
    EXPECT_EQ(utils_cli_default_parser_hook("a b && c", true, true, true, parsed, nullptr), UTILS_OK);
    EXPECT_EQ(utils_cli_parsed_groups(parsed), 2u);
    EXPECT_EQ(utils_cli_default_parser_hook(nullptr, true, true, true, parsed, nullptr), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The input is NULL");
    utils_cli_parsed_destroy(parsed);
}
