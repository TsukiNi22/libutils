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
##  Test of the C API from a pure C program (built in C11 & C23: headers, status, utils_fail, verbose, platform, iomanip, smanip, math, type, encapsulation, cli)
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

static void test_iomanip(void)
{
    const utils_iomanip_style_t styles[] = {C_API_IOMANIP_STYLE_STRONG, C_API_IOMANIP_STYLE_ITALIC};
    char buf[32];

    CHECK(strcmp(utils_iomanip_reset(), "\x1b[0m") == 0);
    CHECK(strcmp(utils_iomanip_color(C_API_IOMANIP_COLOR_RED), "\x1b[31m") == 0);
    CHECK(strcmp(utils_iomanip_set_styles(styles, 2), "\x1b[1;3m") == 0);
    CHECK(utils_iomanip_pos_r(2, 3, buf, sizeof(buf)) == strlen("\x1b[2;3H") && strcmp(buf, "\x1b[2;3H") == 0);
}

static void test_smanip(void)
{
    utils_codec_t* codec = utils_codec_create_base64();
    utils_string_t* out = utils_string_create(NULL, 0);

    CHECK(codec != NULL && out != NULL);
    CHECK(utils_codec_encode(codec, "hello", 5, out) == UTILS_OK);
    CHECK(strcmp(utils_string_data(out), "aGVsbG8=") == 0);
    CHECK(utils_string_size(out) == 8);
    CHECK(strcmp(utils_smanip_format("<>"), utils_iomanip_reset()) == 0);
    utils_string_destroy(out);
    utils_codec_destroy(codec);
}

static void test_math(void)
{
    utils_math_coord2d_t origin = {0.0, 0.0};
    utils_math_coord2d_t point = {2.0, 0.0};
    utils_math_coord2d_t res = utils_math_rotate_point_2d(origin, point, 180.0, false);

    CHECK(res.x < -1.999 && res.x > -2.001);
    CHECK(utils_math_rad_to_deg(utils_math_deg_to_rad(90.0)) > 89.999);
}

static void test_type(void)
{
    utils_vector2_f64_t v = {3.0, 4.0};
    utils_matrix_i32_t* m = utils_matrix_i32_create(2, 2);
    utils_blt_str_i32_t* blt = utils_blt_str_i32_create();
    int32_t value = 0;

    CHECK(utils_vector2_f64_length(v) == 5.0);
    CHECK(m != NULL && utils_matrix_i32_set(m, 1, 1, 7) == UTILS_OK);
    CHECK(utils_matrix_i32_trace(m, &value) == UTILS_OK && value == 7);
    CHECK(utils_blt_str_i32_add(blt, "seven", 7) == UTILS_OK);
    CHECK(utils_blt_str_i32_get_right(blt, "seven", &value) == UTILS_OK && value == 7);
    CHECK(utils_freezable_freeze(blt) == UTILS_OK && utils_freezable_is_frozen(blt));
    utils_blt_str_i32_destroy(blt);
    utils_matrix_i32_destroy(m);
}

static void test_encapsulation(void)
{
    const char* args[] = {"-c", "exit 4"};
    utils_process_t* process = utils_process_create();
    utils_process_status_t status;

    CHECK(process != NULL);
    CHECK(utils_process_spawn_exec(process, "/bin/sh", args, 2) > 0);
    CHECK(utils_process_wait(process, &status) == UTILS_OK && status.exited && status.code == 4);
    utils_process_destroy(process);
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
    test_iomanip();
    test_smanip();
    test_math();
    test_type();
    test_encapsulation();
    test_cli();
    if (failed) fprintf(stderr, "%d check(s) failed\n", failed);
    return (failed ? 1 : 0);
}
