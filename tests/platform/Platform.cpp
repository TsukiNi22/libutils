/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 12/10/2026 by @author Tsukini

File Name:
##  @file Platform.cpp

File Description:
##  Unit tests of the POSIX calls of utils::platform (Unix & Windows: same results)
\**************************************************************/

#define _Platform
#include "utils.hpp"
#include "tools/RawSocket.hpp"
#include <gtest/gtest.h>
#include <cstring>
#include <cerrno>
#include <string>

// Listening socket on 127.0.0.1 (free port)
static int listener(std::uint16_t& port)
{
    const int fd = utils::platform::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (fd < 0 || utils::platform::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0 || utils::platform::listen(fd, 4) < 0) return -1;
    port = tests::tools::local_port(fd);
    return fd;
}

/* ------------------------------- Unistd --------------------------------- */
TEST(PlatformUnistd, StrError) {
    EXPECT_STRNE(utils::platform::strerror(EINVAL), "");
    const std::string refused = utils::platform::strerror(ECONNREFUSED); // socket error: known on both platforms
    EXPECT_FALSE(refused.empty());
    EXPECT_EQ(refused.find("Unknown"), std::string::npos) << refused;
}

TEST(PlatformUnistd, PipeReadWriteClose) {
    int fds[2];
    ASSERT_EQ(utils::platform::pipe(fds), 0);
    EXPECT_EQ(utils::platform::isatty(fds[0]), 0);
    ASSERT_EQ(utils::platform::write(fds[1], "abc", 3), 3);
    EXPECT_EQ(utils::platform::close(fds[1]), 0);
    char buf[8] = {};
    EXPECT_EQ(utils::platform::read(fds[0], buf, sizeof(buf)), 3);
    EXPECT_STREQ(buf, "abc");
    EXPECT_EQ(utils::platform::read(fds[0], buf, sizeof(buf)), 0); // EOF
    EXPECT_EQ(utils::platform::close(fds[0]), 0);
}

TEST(PlatformUnistd, DupAndDup2) {
    int fds[2];
    ASSERT_EQ(utils::platform::pipe(fds), 0);
    const int copy = utils::platform::dup(fds[1]);
    ASSERT_GE(copy, 0);
    const int other[2] = {utils::platform::dup(fds[0]), -1};
    ASSERT_GE(other[0], 0);
    EXPECT_EQ(utils::platform::dup2(copy, other[0]), other[0]); // other[0] now writes in the pipe
    ASSERT_EQ(utils::platform::write(other[0], "x", 1), 1);
    utils::platform::close(other[0]);
    utils::platform::close(copy);
    utils::platform::close(fds[1]);
    char c = 0;
    EXPECT_EQ(utils::platform::read(fds[0], &c, 1), 1);
    EXPECT_EQ(c, 'x');
    utils::platform::close(fds[0]);
}

TEST(PlatformUnistd, CloseInvalid) {
    errno = 0;
    EXPECT_EQ(utils::platform::close(-1), -1);
    EXPECT_EQ(errno, EBADF);
}

TEST(PlatformUnistd, PollPipe) {
    int fds[2];
    ASSERT_EQ(utils::platform::pipe(fds), 0);
    struct pollfd pfd{};
    pfd.fd = fds[0];
    pfd.events = POLLIN;
    EXPECT_EQ(utils::platform::poll(&pfd, 1, 0), 0); // nothing to read
    ASSERT_EQ(utils::platform::write(fds[1], "x", 1), 1);
    EXPECT_EQ(utils::platform::poll(&pfd, 1, 1000), 1);
    EXPECT_TRUE(pfd.revents & POLLIN);
    utils::platform::close(fds[0]);
    utils::platform::close(fds[1]);
}

TEST(PlatformUnistd, ProcessInfo) {
    EXPECT_GT(utils::platform::getpid(), 0);
    char buf[4096];
    const ssize_t len = utils::platform::readlink("/proc/self/exe", buf, sizeof(buf));
    ASSERT_GT(len, 0);
    EXPECT_NE(std::string(buf, static_cast<std::size_t>(len)).find("unit_tests"), std::string::npos);
    errno = 0;
    EXPECT_EQ(utils::platform::readlink("/this/path/does/not/exist/utils", buf, sizeof(buf)), -1);
    EXPECT_EQ(errno, ENOENT);
}

/* -------------------------------- Socket --------------------------------- */
TEST(PlatformSocket, RoundTrip) {
    std::uint16_t port = 0;
    const int server = listener(port);
    ASSERT_GE(server, 0);
    ASSERT_GT(port, 0);

    const int client = tests::tools::raw_connect(port);
    ASSERT_GE(client, 0);
    sockaddr_in peer{};
    socklen_t len = sizeof(peer);
    const int accepted = utils::platform::accept(server, reinterpret_cast<sockaddr*>(&peer), &len);
    ASSERT_GE(accepted, 0);

    EXPECT_EQ(utils::platform::send(client, "ping", 4, MSG_NOSIGNAL), 4);
    char buf[8] = {};
    EXPECT_EQ(utils::platform::recv(accepted, buf, sizeof(buf), 0), 4);
    EXPECT_STREQ(buf, "ping");
    EXPECT_EQ(utils::platform::write(accepted, "pong", 4), 4); // write/read work on a socket too
    std::memset(buf, 0, sizeof(buf));
    EXPECT_EQ(utils::platform::read(client, buf, sizeof(buf)), 4);
    EXPECT_STREQ(buf, "pong");

    len = sizeof(peer);
    EXPECT_EQ(utils::platform::getpeername(accepted, reinterpret_cast<sockaddr*>(&peer), &len), 0);
    EXPECT_EQ(ntohl(peer.sin_addr.s_addr), INADDR_LOOPBACK);

    struct timeval tv{};
    tv.tv_sec = 1;
    EXPECT_EQ(utils::platform::setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)), 0);

    utils::platform::close(client);
    EXPECT_EQ(utils::platform::recv(accepted, buf, sizeof(buf), 0), 0); // closed peer
    EXPECT_EQ(utils::platform::close(accepted), 0);
    EXPECT_EQ(utils::platform::close(server), 0);
}

TEST(PlatformSocket, ConnectRefused) {
    std::uint16_t port = 0;
    const int server = listener(port);
    ASSERT_GE(server, 0);
    utils::platform::close(server); // nothing listening anymore
    const int fd = utils::platform::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    errno = 0;
    EXPECT_EQ(utils::platform::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)), -1);
    EXPECT_EQ(errno, ECONNREFUSED);
    utils::platform::close(fd);
}

TEST(PlatformSocket, AddrInfo) {
    struct addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* res = nullptr;
    ASSERT_EQ(utils::platform::getaddrinfo("localhost", nullptr, &hints, &res), 0);
    ASSERT_NE(res, nullptr);
    utils::platform::freeaddrinfo(res);
    EXPECT_STRNE(utils::platform::gai_strerror(EAI_NONAME), "");
}

/* -------------------------------- Epoll ---------------------------------- */
TEST(PlatformEpoll, SocketEvents) {
    std::uint16_t port = 0;
    const int server = listener(port);
    const int client = tests::tools::raw_connect(port);
    const int accepted = utils::platform::accept(server, nullptr, nullptr);
    ASSERT_GE(accepted, 0);

    const int epfd = utils::platform::epoll_create1(EPOLL_CLOEXEC);
    ASSERT_GE(epfd, 0);
    struct epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = accepted;
    ASSERT_EQ(utils::platform::epoll_ctl(epfd, EPOLL_CTL_ADD, accepted, &ev), 0);
    struct epoll_event out[4];
    EXPECT_EQ(utils::platform::epoll_wait(epfd, out, 4, 0), 0);
    ASSERT_EQ(utils::platform::write(client, "x", 1), 1);
    ASSERT_EQ(utils::platform::epoll_wait(epfd, out, 4, 1000), 1);
    EXPECT_EQ(out[0].data.fd, accepted);
    EXPECT_TRUE(out[0].events & EPOLLIN);
    errno = 0;
    EXPECT_EQ(utils::platform::epoll_ctl(epfd, EPOLL_CTL_ADD, accepted, &ev), -1);
    EXPECT_EQ(errno, EEXIST);
    EXPECT_EQ(utils::platform::epoll_ctl(epfd, EPOLL_CTL_DEL, accepted, nullptr), 0);
    EXPECT_EQ(utils::platform::close(epfd), 0);

    utils::platform::close(client);
    utils::platform::close(accepted);
    utils::platform::close(server);
}

/* -------------------------------- Dlfcn ---------------------------------- */
TEST(PlatformDlfcn, Dladdr) {
    Dl_info info{};
    ASSERT_NE(utils::platform::dladdr(reinterpret_cast<const void*>(&listener), &info), 0);
    ASSERT_NE(info.dli_fname, nullptr);
    EXPECT_NE(std::string(info.dli_fname).find("unit_tests"), std::string::npos) << info.dli_fname;
}

TEST(PlatformDlfcn, UnknownLibrary) {
    (void)utils::platform::dlerror(); // clear
    EXPECT_EQ(utils::platform::dlopen("libdoesnotexist-utils", RTLD_NOW), nullptr);
    EXPECT_NE(utils::platform::dlerror(), nullptr);
    EXPECT_EQ(utils::platform::dlerror(), nullptr); // cleared by the call
}

/* ------------------------------- Termios --------------------------------- */
TEST(PlatformTermios, NotATerminal) {
    int fds[2];
    ASSERT_EQ(utils::platform::pipe(fds), 0);
    struct termios term{};
    errno = 0;
    EXPECT_EQ(utils::platform::tcgetattr(fds[0], &term), -1);
    EXPECT_EQ(errno, ENOTTY);
    utils::platform::close(fds[0]);
    utils::platform::close(fds[1]);
}
