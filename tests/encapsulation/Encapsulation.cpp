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
##  @file Encapsulation.cpp

File Description:
##  Unit tests of the encapsulation part (Pipe, Dup, Process, Poll, SharedObject)
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <cstring>
#include <string>
#include <vector>
#include <array>

// Read everything available from a fd until EOF
static std::string readAll(int fd)
{
    std::string s;
    char buf[256];
    ssize_t n = 0;
    while ((n = ::read(fd, buf, sizeof(buf))) > 0) s.append(buf, n);
    return s;
}

static bool isOpen(int fd) {return ::fcntl(fd, F_GETFD) != -1;}

/* --------------------------------- Pipe ---------------------------------- */
TEST(Pipe, DefaultClosed) {
    utils::encapsulation::Pipe pipe;
    EXPECT_EQ(pipe.getRead(), -1);
    EXPECT_EQ(pipe.getWrite(), -1);
}

TEST(Pipe, TriggerOpenFds) {
    utils::encapsulation::Pipe pipe;
    ASSERT_NO_THROW(pipe.trigger());
    EXPECT_NE(pipe.getRead(), -1);
    EXPECT_NE(pipe.getWrite(), -1);
    EXPECT_EQ(pipe.getFds()[0], pipe.getRead());

    ASSERT_EQ(::write(pipe.getWrite(), "data", 4), 4);
    pipe.closeWrite();
    EXPECT_EQ(pipe.getWrite(), -1);
    EXPECT_EQ(readAll(pipe.getRead()), "data");
}

TEST(Pipe, TriggerTwiceThrows) {
    utils::encapsulation::Pipe pipe;
    pipe.trigger();
    try {
        pipe.trigger();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Pipe);
    }
}

TEST(Pipe, DestructorClose) {
    int read = -1, write = -1;
    {
        utils::encapsulation::Pipe pipe;
        pipe.trigger();
        read = pipe.getRead();
        write = pipe.getWrite();
        EXPECT_TRUE(isOpen(read));
    }
    EXPECT_FALSE(isOpen(read));
    EXPECT_FALSE(isOpen(write));
}

TEST(Pipe, MoveTransferOwnership) {
    utils::encapsulation::Pipe a;
    a.trigger();
    int read = a.getRead();
    utils::encapsulation::Pipe b(std::move(a));
    EXPECT_EQ(a.getRead(), -1);
    EXPECT_EQ(b.getRead(), read);
    utils::encapsulation::Pipe c;
    c = std::move(b);
    EXPECT_EQ(b.getRead(), -1);
    EXPECT_EQ(c.getRead(), read);
    EXPECT_TRUE(isOpen(read));
}

TEST(Pipe, ClearDoesNotClose) {
    utils::encapsulation::Pipe pipe;
    pipe.trigger();
    std::array<int, 2> fds = pipe.getFds();
    pipe.clear();
    EXPECT_EQ(pipe.getRead(), -1);
    EXPECT_TRUE(isOpen(fds[0]));
    ::close(fds[0]);
    ::close(fds[1]);
}

TEST(Pipe, Setters) {
    utils::encapsulation::Pipe pipe;
    pipe.setRead(-1);
    pipe.setWrite(-1);
    pipe.setFds({-1, -1});
    EXPECT_EQ(pipe.getRead(), -1);
}

/* ---------------------------------- Dup ---------------------------------- */
TEST(Dup, TriggerCreateClone) {
    int fds[2];
    ASSERT_EQ(::pipe(fds), 0);
    utils::encapsulation::Dup dup(fds[1]);
    ASSERT_NO_THROW(dup.trigger());
    EXPECT_NE(dup.getClone(), -1);
    EXPECT_NE(dup.getClone(), fds[1]);

    ASSERT_EQ(::write(dup.getClone(), "x", 1), 1);
    dup.close(); // close origin & clone
    EXPECT_EQ(readAll(fds[0]), "x");
    ::close(fds[0]);
}

TEST(Dup, TriggerOnGivenClone) {
    int a[2], b[2];
    ASSERT_EQ(::pipe(a), 0);
    ASSERT_EQ(::pipe(b), 0);
    utils::encapsulation::Dup dup(a[1], b[1]); // b[1] now refer to a[1]
    ASSERT_NO_THROW(dup.trigger());
    EXPECT_EQ(dup.getClone(), b[1]);
    ASSERT_EQ(::write(b[1], "y", 1), 1);
    dup.close();
    EXPECT_EQ(readAll(a[0]), "y");
    ::close(a[0]);
    ::close(b[0]);
}

TEST(Dup, InvalidOrigin) {
    utils::encapsulation::Dup dup;
    try {
        dup.trigger();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Dup);
    }
}

TEST(Dup, Move) {
    utils::encapsulation::Dup a(5, 6);
    utils::encapsulation::Dup b(std::move(a));
    EXPECT_EQ(a.getOrigin(), -1);
    EXPECT_EQ(b.getOrigin(), 5);
    EXPECT_EQ(b.getClone(), 6);
    b.clear();
    EXPECT_EQ(b.getClone(), -1);
}

/* -------------------------------- Process -------------------------------- */
TEST(Process, SpawnExecExitCode) {
    utils::encapsulation::Process process;
    pid_t pid = process.spawn("sh", {"-c", "exit 3"});
    EXPECT_GT(pid, 0);
    EXPECT_TRUE(process.isParent());
    utils::encapsulation::Status status = process.wait();
    EXPECT_TRUE(status.exited);
    EXPECT_FALSE(status.unknown);
    EXPECT_EQ(status.code, 3);
    EXPECT_EQ(status.sig, 0);
    EXPECT_EQ(process.getPid(), -1);
}

TEST(Process, SpawnExecTrue) {
    utils::encapsulation::Process process;
    (void)process.spawn("true", {});
    EXPECT_EQ(process.wait().code, 0);
}

TEST(Process, SpawnUnknownBinary) {
    utils::encapsulation::Process process;
    (void)process.spawn("/this/binary/does/not/exist", {});
    utils::encapsulation::Status status = process.wait();
    EXPECT_TRUE(status.exited);
    EXPECT_EQ(status.code, 127);
}

TEST(Process, SpawnFork) {
    utils::encapsulation::Process process;
    pid_t pid = process.spawn();
    if (pid == 0) ::_exit(5); // child
    EXPECT_EQ(process.wait().code, 5);
}

TEST(Process, Kill) {
    utils::encapsulation::Process process;
    (void)process.spawn("sleep", {"10"});
    EXPECT_TRUE(process.is());
    process.kill();
    EXPECT_EQ(process.getPid(), -1);
}

TEST(Process, WaitKilledBySignal) {
    utils::encapsulation::Process process;
    pid_t pid = process.spawn("sleep", {"10"});
    ::kill(pid, SIGTERM);
    utils::encapsulation::Status status = process.wait();
    EXPECT_FALSE(status.exited);
    EXPECT_EQ(status.sig, SIGTERM);
}

TEST(Process, IsAfterWait) {
    utils::encapsulation::Process process;
    (void)process.spawn("true", {});
    (void)process.wait();
    EXPECT_FALSE(process.is()); // no more process
}

TEST(Process, SpawnTwiceThrows) {
    utils::encapsulation::Process process;
    (void)process.spawn("sleep", {"10"});
    try {
        (void)process.spawn("true", {});
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Process);
    }
    process.kill();
}

TEST(Process, WaitWithoutProcess) {
    utils::encapsulation::Process process;
    EXPECT_THROW((void)process.wait(), utils::exception::IException);
}

TEST(Process, RedirectOutputWithDup) {
    utils::encapsulation::Pipe pipe;
    pipe.trigger();
    utils::encapsulation::Process process;
    process.dup(pipe.getWrite(), STDOUT_FILENO); // child: stdout -> pipe
    (void)process.spawn("echo", {"hello"});
    pipe.closeWrite();
    EXPECT_EQ(readAll(pipe.getRead()), "hello\n");
    EXPECT_EQ(process.wait().code, 0);
}

TEST(Process, PipesAreTriggeredOnSpawn) {
    utils::encapsulation::Process process;
    process.pipe(-1, -1);
    (void)process.spawn("true", {});
    ASSERT_EQ(process.getPipes().size(), 1u);
    EXPECT_NE(process.getPipes()[0].getRead(), -1);
    (void)process.wait();
    process.clear();
    EXPECT_TRUE(process.getPipes().empty());
}

TEST(Process, DestructorKillChild) {
    pid_t pid = -1;
    {
        utils::encapsulation::Process process;
        pid = process.spawn("sleep", {"10"});
    }
    EXPECT_EQ(::kill(pid, 0), -1); // reaped
}

/* --------------------------------- Poll ---------------------------------- */
TEST(Poll, InitFd) {
    utils::encapsulation::Poll poll;
    EXPECT_NE(poll.getFd(), -1);
    EXPECT_EQ(poll.size(), 0u);
}

TEST(Poll, LinkWaitUnlink) {
    utils::encapsulation::Pipe pipe;
    pipe.trigger();
    utils::encapsulation::Poll poll;
    poll.link(pipe.getRead(), EPOLLIN);
    EXPECT_EQ(poll.size(), 1u);
    EXPECT_TRUE(poll.contains(pipe.getRead()));

    EXPECT_TRUE(poll.wait(0).empty()); // nothing to read

    ASSERT_EQ(::write(pipe.getWrite(), "z", 1), 1);
    std::vector<struct epoll_event> events = poll.wait(100);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].data.fd, pipe.getRead());
    EXPECT_TRUE(events[0].events & EPOLLIN);

    poll.unlink(pipe.getRead());
    EXPECT_EQ(poll.size(), 0u);
    EXPECT_FALSE(poll.contains(pipe.getRead()));
    EXPECT_TRUE(poll.wait(0).empty());
}

TEST(Poll, UserData) {
    utils::encapsulation::Pipe pipe;
    pipe.trigger();
    int marker = 42;
    utils::encapsulation::Poll poll;
    poll.link(pipe.getRead(), EPOLLIN, &marker);
    ASSERT_EQ(::write(pipe.getWrite(), "z", 1), 1);
    std::vector<struct epoll_event> events = poll.wait(100);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].data.ptr, &marker);
}

TEST(Poll, Edit) {
    utils::encapsulation::Pipe pipe;
    pipe.trigger();
    utils::encapsulation::Poll poll;
    poll.link(pipe.getWrite(), 0);
    EXPECT_TRUE(poll.wait(0).empty());
    poll.edit(pipe.getWrite(), EPOLLOUT);
    EXPECT_EQ(poll.wait(0).size(), 1u);
}

TEST(Poll, LinkTwiceThrows) {
    utils::encapsulation::Pipe pipe;
    pipe.trigger();
    utils::encapsulation::Poll poll;
    poll.link(pipe.getRead(), EPOLLIN);
    try {
        poll.link(pipe.getRead(), EPOLLIN);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::EPoll);
    }
}

TEST(Poll, UnlinkClosedFdIsIgnored) {
    utils::encapsulation::Pipe pipe;
    pipe.trigger();
    utils::encapsulation::Poll poll;
    int fd = pipe.getRead();
    poll.link(fd, EPOLLIN);
    pipe.closeRead();
    EXPECT_NO_THROW(poll.unlink(fd));
    EXPECT_EQ(poll.size(), 0u);
}

TEST(Poll, WaitLimits) {
    utils::encapsulation::Pipe a, b;
    a.trigger();
    b.trigger();
    utils::encapsulation::Poll poll;
    poll.link(a.getRead(), EPOLLIN);
    poll.link(b.getRead(), EPOLLIN);
    ASSERT_EQ(::write(a.getWrite(), "1", 1), 1);
    ASSERT_EQ(::write(b.getWrite(), "2", 1), 1);
    EXPECT_EQ(poll.wait(100).size(), 2u);
    EXPECT_EQ(poll.wait(100, 1).size(), 1u);
}

TEST(Poll, Move) {
    utils::encapsulation::Poll a;
    int fd = a.getFd();
    utils::encapsulation::Poll b(std::move(a));
    EXPECT_EQ(a.getFd(), -1);
    EXPECT_EQ(b.getFd(), fd);
    utils::encapsulation::Poll c;
    c = std::move(b);
    EXPECT_EQ(c.getFd(), fd);
    EXPECT_TRUE(isOpen(fd));
}

TEST(Poll, CloseOnDestruction) {
    int fd = -1;
    {
        utils::encapsulation::Poll poll;
        fd = poll.getFd();
    }
    EXPECT_FALSE(isOpen(fd));
}

/* ----------------------------- SharedObject ------------------------------ */
TEST(SharedObject, LoadLibc) {
    utils::encapsulation::SharedObject so("libc.so.6");
    EXPECT_TRUE(so.isloaded());
    EXPECT_EQ(so.path(), "libc.so.6");
    EXPECT_NE(so.get(), nullptr);

    using StrlenFn = std::size_t(*)(const char*);
    StrlenFn fn = so.loadFunction<StrlenFn>("strlen");
    ASSERT_NE(fn, nullptr);
    EXPECT_EQ(fn("hello"), 5u);
}

TEST(SharedObject, UnknownLibrary) {
    try {
        utils::encapsulation::SharedObject so("libdoesnotexist-utils.so");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Dlopen);
    }
}

TEST(SharedObject, UnknownSymbol) {
    utils::encapsulation::SharedObject so("libc.so.6");
    try {
        (void)so.loadFunction<void(*)(void)>("this_symbol_does_not_exist_utils");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Dlsym);
    }
}

TEST(SharedObject, Move) {
    utils::encapsulation::SharedObject a("libc.so.6");
    void* handle = a.get();
    utils::encapsulation::SharedObject b(std::move(a));
    EXPECT_FALSE(a.isloaded());
    EXPECT_EQ(b.get(), handle);
    EXPECT_EQ(b.path(), "libc.so.6");
}
