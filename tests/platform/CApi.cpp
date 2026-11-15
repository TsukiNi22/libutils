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
##  Unit tests of the C API of utils::platform (Unix: pipe, dup, poll, epoll, socket, dlfcn, termios)
\**************************************************************/

#include "utils/platform/CApi.h"
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

//----------------------------------------------------------------//
/* FD */

TEST(PlatformCApi, PipeWriteReadClose)
{
    int fds[2] = {-1, -1};
    char buf[8] = {};

    ASSERT_EQ(utils_platform_pipe(fds), 0);
    EXPECT_EQ(utils_platform_write(fds[1], "abc", 3), 3);
    EXPECT_EQ(utils_platform_read(fds[0], buf, sizeof(buf)), 3);
    EXPECT_STREQ(buf, "abc");
    EXPECT_EQ(utils_platform_isatty(fds[0]), 0);
    EXPECT_EQ(utils_platform_close(fds[0]), 0);
    EXPECT_EQ(utils_platform_close(fds[1]), 0);
    EXPECT_EQ(utils_platform_close(fds[1]), -1);
    EXPECT_EQ(errno, EBADF);
}

TEST(PlatformCApi, DupAndPoll)
{
    int fds[2] = {-1, -1};
    struct pollfd pfd = {};

    ASSERT_EQ(utils_platform_pipe(fds), 0);
    const int copy = utils_platform_dup(fds[1]);
    ASSERT_GE(copy, 0);
    EXPECT_EQ(utils_platform_write(copy, "x", 1), 1);
    pfd.fd = fds[0];
    pfd.events = POLLIN;
    EXPECT_EQ(utils_platform_poll(&pfd, 1, 0), 1);
    EXPECT_TRUE(pfd.revents & POLLIN);
    EXPECT_EQ(utils_platform_dup2(fds[0], copy), copy);
    for (const int fd: {fds[0], fds[1], copy})
        EXPECT_EQ(utils_platform_close(fd), 0);
}

TEST(PlatformCApi, ProcessAndError)
{
    char buf[512] = {};

    EXPECT_EQ(utils_platform_getpid(), ::getpid());
    EXPECT_GT(utils_platform_readlink("/proc/self/exe", buf, sizeof(buf) - 1), 0);
    EXPECT_STREQ(utils_platform_strerror(ENOENT), std::strerror(ENOENT));
}

//----------------------------------------------------------------//
/* EPOLL */

TEST(PlatformCApi, EpollWaitsOnAPipe)
{
    int fds[2] = {-1, -1};
    struct epoll_event event = {};
    struct epoll_event ready[2] = {};

    ASSERT_EQ(utils_platform_pipe(fds), 0);
    const int epfd = utils_platform_epoll_create1(0);
    ASSERT_GE(epfd, 0);
    event.events = EPOLLIN;
    event.data.fd = fds[0];
    EXPECT_EQ(utils_platform_epoll_ctl(epfd, EPOLL_CTL_ADD, fds[0], &event), 0);
    EXPECT_EQ(utils_platform_epoll_wait(epfd, ready, 2, 0), 0);
    EXPECT_EQ(utils_platform_write(fds[1], "x", 1), 1);
    EXPECT_EQ(utils_platform_epoll_wait(epfd, ready, 2, 0), 1);
    EXPECT_EQ(ready[0].data.fd, fds[0]);
    for (const int fd: {epfd, fds[0], fds[1]})
        EXPECT_EQ(utils_platform_close(fd), 0);
}

//----------------------------------------------------------------//
/* SOCKET */

TEST(PlatformCApi, SocketPairOfTCP)
{
    struct addrinfo hints = {};
    struct addrinfo* res = nullptr;
    struct sockaddr_storage addr = {};
    socklen_t len = sizeof(addr);
    const int one = 1;
    char buf[4] = {};

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    ASSERT_EQ(utils_platform_getaddrinfo("127.0.0.1", "0", &hints, &res), 0);
    const int server = utils_platform_socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(server, 0);
    EXPECT_EQ(utils_platform_setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)), 0);
    EXPECT_EQ(utils_platform_bind(server, res->ai_addr, res->ai_addrlen), 0);
    EXPECT_EQ(utils_platform_listen(server, 1), 0);
    EXPECT_EQ(utils_platform_getsockname(server, reinterpret_cast<struct sockaddr*>(&addr), &len), 0);
    utils_platform_freeaddrinfo(res);

    const int client = utils_platform_socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(client, 0);
    EXPECT_EQ(utils_platform_connect(client, reinterpret_cast<struct sockaddr*>(&addr), len), 0);
    const int accepted = utils_platform_accept(server, nullptr, nullptr);
    ASSERT_GE(accepted, 0);
    len = sizeof(addr);
    EXPECT_EQ(utils_platform_getpeername(client, reinterpret_cast<struct sockaddr*>(&addr), &len), 0);
    EXPECT_EQ(utils_platform_send(client, "abc", 3, MSG_NOSIGNAL), 3);
    EXPECT_EQ(utils_platform_recv(accepted, buf, 3, 0), 3);
    EXPECT_STREQ(buf, "abc");
    for (const int fd: {accepted, client, server})
        EXPECT_EQ(utils_platform_close(fd), 0);
}

TEST(PlatformCApi, AddrinfoError)
{
    struct addrinfo* res = nullptr;
    const int code = utils_platform_getaddrinfo("invalid..host..name", nullptr, nullptr, &res);

    EXPECT_NE(code, 0);
    EXPECT_NE(utils_platform_gai_strerror(code), nullptr);
}

//----------------------------------------------------------------//
/* DLFCN */

TEST(PlatformCApi, DlopenDlsymDladdr)
{
    utils_platform_dl_info_t info = {};

    EXPECT_EQ(utils_platform_dlopen("/nonexistent/libnothing.so", RTLD_NOW), nullptr);
    EXPECT_NE(utils_platform_dlerror(), nullptr);
    EXPECT_EQ(utils_platform_dlerror(), nullptr); // cleared by the call
    void* self = utils_platform_dlopen(nullptr, RTLD_NOW);
    ASSERT_NE(self, nullptr);
    EXPECT_EQ(utils_platform_dlsym(self, "symbol_that_does_not_exist"), nullptr);
    EXPECT_EQ(utils_platform_dlclose(self), 0);
    EXPECT_NE(utils_platform_dladdr(reinterpret_cast<const void*>(&utils_platform_close), &info), 0);
    EXPECT_NE(info.dli_fname, nullptr);
    EXPECT_EQ(utils_platform_dladdr(reinterpret_cast<const void*>(&utils_platform_close), nullptr), 0);
}

//----------------------------------------------------------------//
/* TERMIOS */

TEST(PlatformCApi, TermiosOnAPipeIsNotATty)
{
    int fds[2] = {-1, -1};
    struct termios term = {};

    ASSERT_EQ(utils_platform_pipe(fds), 0);
    EXPECT_EQ(utils_platform_tcgetattr(fds[0], &term), -1);
    EXPECT_EQ(errno, ENOTTY);
    EXPECT_EQ(utils_platform_tcsetattr(fds[0], TCSANOW, &term), -1);
    for (const int fd: {fds[0], fds[1]})
        EXPECT_EQ(utils_platform_close(fd), 0);
}
