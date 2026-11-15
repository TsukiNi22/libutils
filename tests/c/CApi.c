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
##  @file CApi.c

File Description:
##  Test of the C API from a pure C program (built in C11 & C23: headers, status, utils_fail, verbose, platform, cli)
\**************************************************************/

#define _POSIX_C_SOURCE 200809L // POSIX structures of the C API of platform (strict C11 / C23)
#include <string.h>
#include <utils.h>
#include <stdio.h>

/* number of failed checks */
static int failed = 0;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "%s:%d: CHECK(%s) failed (info: %s)\n", __FILE__, __LINE__, #cond, utils_exception.info); \
            ++failed; \
        } \
    } while (0)

//----------------------------------------------------------------//
/* CALLBACKS */

static int record(const utils_cli_t* cli, size_t argc, const char* const* argv, void* data)
{
    int* calls = data;

    (void)cli;
    (void)argv;
    *calls += (int)argc;
    return UTILS_OK;
}

static int fail(const utils_cli_t* cli, size_t argc, const char* const* argv, void* data)
{
    (void)cli;
    (void)argc;
    (void)argv;
    (void)data;
    return utils_fail(42, "fail on purpose");
}

static int stop(const utils_cli_t* cli, size_t argc, const char* const* argv, void* data)
{
    (void)cli;
    (void)argc;
    (void)argv;
    (void)data;
    return C_API_CLI_EXIT;
}

static int eof(char* c, void* data)
{
    (void)data;
    *c = '\x04'; // never read the real stdin
    return UTILS_OK;
}

//----------------------------------------------------------------//
/* TESTS */

static void test_status(void)
{
    char buf[8];

    utils_exception_clear();
    CHECK(!utils_exception.exception);
    CHECK(strcmp(utils_exception_format(), "") == 0);

    /* utils_fail: ignorable return, emplacement of the call */
    utils_fail(7, "ignored return");
    CHECK(utils_exception.exception);
    CHECK(utils_exception.origin == C_API_EXCEPTION_ORIGIN_C);
    CHECK(utils_exception.code == 7);
    CHECK(strcmp(utils_exception.function, "test_status") == 0);
    CHECK(utils_exception_set(C_API_EXCEPTION_TYPE_WARNING, 8, "what", "info") == UTILS_KO);
    CHECK(utils_exception.type == C_API_EXCEPTION_TYPE_WARNING);

    /* format: same as the c++, the status is kept */
    CHECK(strstr(utils_exception_format(), "info") != NULL);
    CHECK(utils_exception_format_r(buf, sizeof(buf)) == strlen(utils_exception_format()));
    CHECK(strlen(buf) == sizeof(buf) - 1);
    CHECK(utils_exception.exception);
}

static int count(void* data)
{
    ++*(int*)data;
    return UTILS_OK;
}

static void test_verbose(void)
{
    int calls = 0;
    utils_verbose_t old = utils_verbose_get();

    utils_verbose_set(C_API_VERBOSE_BASIC);
    CHECK(utils_verbose_get() == C_API_VERBOSE_BASIC);
    CHECK(utils_verbose_is(C_API_VERBOSE_BASIC) && !utils_verbose_is(C_API_VERBOSE_DEBUG));
    CHECK(utils_verbose_log(C_API_VERBOSE_DEBUG, "hidden %d", 1) == UTILS_OK);
    CHECK(utils_verbose_locked(count, &calls) == UTILS_OK && calls == 1);
    CHECK(utils_verbose_on(C_API_VERBOSE_DEBUG, count, &calls) == UTILS_OK && calls == 1);
    utils_verbose_set(old);
}

static void test_platform(void)
{
    int fds[2] = {-1, -1};
    char buf[4] = {0};

    CHECK(utils_platform_pipe(fds) == 0);
    CHECK(utils_platform_write(fds[1], "ok", 2) == 2);
    CHECK(utils_platform_read(fds[0], buf, sizeof(buf)) == 2 && strcmp(buf, "ok") == 0);
    CHECK(utils_platform_close(fds[0]) == 0 && utils_platform_close(fds[1]) == 0);
    CHECK(utils_platform_getpid() > 0);
    CHECK(utils_platform_strerror(2) != NULL);
}

static void test_cli(void)
{
    int calls = 0;
    const char* inputs[] = {"record a b", "fail"};
    const char* end[] = {"stop"};
    utils_cli_t* cli = utils_cli_create(false);

    CHECK(cli != NULL);
    CHECK(utils_cli_set_getc_hook(cli, eof, NULL) == UTILS_OK);
    utils_cli_set_flags(cli, C_API_CLI_FLAG_NO_TTY | C_API_CLI_FLAG_PARSED | C_API_CLI_FLAG_NOECHO);
    CHECK(!utils_exception.exception);
    CHECK(utils_cli_set_parsed_command(cli, "record", record, 0, C_API_CLI_NO_LIMIT, &calls, false) == UTILS_OK);
    CHECK(utils_cli_set_parsed_command(cli, "fail", fail, 0, 0, NULL, false) == UTILS_OK);
    CHECK(utils_cli_set_parsed_command(cli, "stop", stop, C_API_CLI_NO_LIMIT, C_API_CLI_NO_LIMIT, NULL, false) == UTILS_OK);

    /* utils_fail in a command (no CATCH): the error stops the cli, wrapped by the Cli (info in its message) */
    CHECK(utils_cli_start(cli, inputs, 2, 0) == UTILS_KO);
    CHECK(calls == 3);
    CHECK(utils_exception.origin == C_API_EXCEPTION_ORIGIN_INTERNAL);
    CHECK(strstr(utils_exception.info, "fail on purpose") != NULL);

    /* exit */
    CHECK(utils_cli_start(cli, end, 1, 0) == UTILS_OK);
    CHECK(utils_cli_was_stopped(cli));

    /* NULL handle of a void function: only the status tells */
    utils_cli_interrupt(NULL);
    CHECK(utils_exception.exception);
    CHECK(utils_exception.origin == C_API_EXCEPTION_ORIGIN_INTERNAL);

    utils_cli_destroy(cli);
    CHECK(!utils_exception.exception);
}

int main(void)
{
    test_status();
    test_verbose();
    test_platform();
    test_cli();
    if (failed) fprintf(stderr, "%d check(s) failed\n", failed);
    return (failed ? 1 : 0);
}
