/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 07/10/2026 by @author Tsukini

File Name:
##  @file Windows.cpp

File Description:
##  Unit tests of the Windows ports (Pipe, Dup, SharedObject, Process, Poll, TCPSocket, ANSI, module paths)
##  Only built for Windows (the Unix versions are tested in their own files)
\**************************************************************/

#include "tools/RawSocket.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fcntl.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <io.h>

// Read everything available from a fd until EOF
static std::string readAll(int fd)
{
    std::string s;
    char buf[256];
    int n = 0;
    while ((n = ::_read(fd, buf, sizeof(buf))) > 0) s.append(buf, static_cast<std::size_t>(n));
    return s;
}

/* ------------------------------- Platform -------------------------------- */
TEST(Platform, Detection) {
    EXPECT_STREQ(UTILS_PLATFORM, "windows");
}

TEST(Platform, ExecutablePath) {
    char buf[4096];
    const ssize_t len = utils::platform::readlink("/proc/self/exe", buf, sizeof(buf));
    ASSERT_GT(len, 0);
    EXPECT_EQ(std::filesystem::path(std::string(buf, static_cast<std::size_t>(len))).extension(), ".exe");
}

TEST(Platform, ModulePath) {
    // An address of the binary is in the binary
    char buf[4096];
    const ssize_t len = utils::platform::readlink("/proc/self/exe", buf, sizeof(buf));
    Dl_info info{};
    ASSERT_NE(utils::platform::dladdr(reinterpret_cast<const void*>(&readAll), &info), 0);
    EXPECT_EQ(std::string(info.dli_fname), std::string(buf, static_cast<std::size_t>(len)));
    EXPECT_EQ(utils::platform::dladdr(nullptr, &info), 0);
}

/* --------------------------------- Pipe ---------------------------------- */
TEST(Pipe, TriggerReadWrite) {
    utils::encapsulation::Pipe pipe;
    ASSERT_NO_THROW(pipe.trigger());
    ASSERT_NE(pipe.getRead(), -1);
    ASSERT_NE(pipe.getWrite(), -1);

    ASSERT_EQ(::_write(pipe.getWrite(), "data", 4), 4);
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

/* ---------------------------------- Dup ---------------------------------- */
TEST(Dup, TriggerCreateClone) {
    utils::encapsulation::Pipe pipe;
    pipe.trigger();

    utils::encapsulation::Dup dup(pipe.getWrite());
    ASSERT_NO_THROW(dup.trigger());
    ASSERT_NE(dup.getClone(), -1);
    ASSERT_EQ(::_write(dup.getClone(), "x", 1), 1);
    dup.closeClone();
    pipe.closeWrite();
    EXPECT_EQ(readAll(pipe.getRead()), "x");
}

TEST(Dup, InvalidOriginThrows) {
    utils::encapsulation::Dup dup;
    try {
        dup.trigger();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Dup);
    }
}

/* ----------------------------- SharedObject ------------------------------ */
TEST(SharedObject, LoadKernel32) {
    utils::encapsulation::SharedObject so("kernel32.dll");
    EXPECT_TRUE(so.isLoaded());
    EXPECT_EQ(so.path(), "kernel32.dll");

    using GetCurrentProcessIdFn = unsigned long (__stdcall *)(void);
    GetCurrentProcessIdFn fn = so.loadFunction<GetCurrentProcessIdFn>("GetCurrentProcessId");
    ASSERT_NE(fn, nullptr);
    EXPECT_NE(fn(), 0u);
}

TEST(SharedObject, UnknownLibrary) {
    try {
        utils::encapsulation::SharedObject so("libdoesnotexist-utils.dll");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Dlopen);
    }
}

TEST(SharedObject, UnknownSymbol) {
    utils::encapsulation::SharedObject so("kernel32.dll");
    try {
        (void)so.loadFunction<void(*)(void)>("this_symbol_does_not_exist_utils");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Dlsym);
    }
}

TEST(SharedObject, MoveTransferOwnership) {
    utils::encapsulation::SharedObject a("kernel32.dll");
    utils::encapsulation::SharedObject b(std::move(a));
    EXPECT_FALSE(a.isLoaded());
    EXPECT_TRUE(b.isLoaded());
}

/* ------------------------------- TCPSocket ------------------------------- */
TEST(TCPSocket, ResolveLocalhost) {
    EXPECT_TRUE(utils::network::is_ip("127.0.0.1"));
    EXPECT_FALSE(utils::network::is_ip("localhost"));
    EXPECT_EQ(utils::network::resolve_hostname("localhost"), "127.0.0.1");
}

TEST(TCPSocket, LoopbackRoundTrip) {
    utils::network::TCPSocket server;
    utils::network::TCPSocket client;
    utils::network::Address address;
    address.ip.first = "127.0.0.1";

    // Find a free port
    bool listening = false;
    for (std::uint16_t port = 47100; port < 47200 && !listening; ++port) {
        address.port = port;
        try {
            server.listen(address);
            listening = true;
        } catch (const utils::exception::IException&) {
            server.reset();
        }
    }
    ASSERT_TRUE(listening);

    // Connect, accept & exchange payloads in both directions
    ASSERT_NO_THROW(client.connect(address));
    int peer = server.accept();
    ASSERT_GE(peer, 0);
    client.send("hello");
    EXPECT_EQ(server.recv(peer), "hello");
    server.send("world", peer);
    EXPECT_EQ(client.recv(), "world");

    // Closed peer: the receiver gets 'SocketClosed'
    client.close();
    try {
        (void)server.recv(peer);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::SocketClosed);
    }
}

TEST(TCPSocket, ConnectRefused) {
    utils::network::TCPSocket client;
    utils::network::Address address;
    address.ip.first = "127.0.0.1";
    address.port = 1; // nothing listening
    try {
        client.connect(address);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::SocketInit);
    }
    EXPECT_EQ(client.getFd(), -1);
}

/* --------------------------------- ANSI ---------------------------------- */
// Redirect the standard input on a pipe filled with the given string
class StdinRedirect {
    private:
        int _saved = -1;

    public:
        StdinRedirect(const std::string& s)
        {
            int fds[2];
            if (::_pipe(fds, 4096, _O_BINARY) == -1) throw std::runtime_error("pipe");
            (void)::_write(fds[1], s.data(), static_cast<unsigned int>(s.size()));
            ::_close(fds[1]);
            this->_saved = ::_dup(0);
            ::_dup2(fds[0], 0);
            ::_close(fds[0]);
        };
        ~StdinRedirect() {::_dup2(this->_saved, 0); ::_close(this->_saved);};
};

TEST(ANSI, EnableDoesNotThrow) {
    EXPECT_NO_THROW((void)utils::iomanip::enable_ansi()); // false without a console (CI, redirected output)
}

TEST(ANSI, ReadCursorPosition) {
    StdinRedirect redirect("\x1b[12;34R");
    std::pair<int, int> pos = utils::iomanip::read_cursor_position();
    EXPECT_EQ(pos.first, 12);
    EXPECT_EQ(pos.second, 34);
}

/* --------------------------------- Poll ---------------------------------- */
// Connected pair of TCP sockets through a listening TCPSocket
struct SocketPair {
    utils::network::TCPSocket server;
    utils::network::TCPSocket client;
    int peer = -1;

    SocketPair(void)
    {
        utils::network::Address address;
        address.ip.first = "127.0.0.1";
        address.port = 0; // free port
        this->server.listen(address);
        address.port = tests::tools::local_port(this->server.getFd());
        this->client.connect(address);
        this->peer = this->server.accept();
    };
};

TEST(Poll, ReadableSocket) {
    SocketPair pair;
    utils::encapsulation::Poll poll;
    poll.link(pair.peer, EPOLLIN);
    EXPECT_TRUE(poll.contains(pair.peer));
    EXPECT_TRUE(poll.wait(0).empty()); // nothing sent yet

    pair.client.send("data");
    std::vector<struct epoll_event> events = poll.wait(1000);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].data.fd, pair.peer);
    EXPECT_TRUE(events[0].events & EPOLLIN);
}

TEST(Poll, DataPointer) {
    SocketPair pair;
    utils::encapsulation::Poll poll;
    int marker = 42;
    poll.link(pair.peer, EPOLLIN, &marker);
    pair.client.send("data");
    std::vector<struct epoll_event> events = poll.wait(1000);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].data.ptr, &marker);
}

TEST(Poll, OneShot) {
    SocketPair pair;
    utils::encapsulation::Poll poll;
    poll.link(pair.peer, EPOLLIN | EPOLLONESHOT);
    pair.client.send("data");
    EXPECT_EQ(poll.wait(1000).size(), 1u);
    EXPECT_TRUE(poll.wait(50).empty()); // disabled until edit
    poll.edit(pair.peer, EPOLLIN);
    EXPECT_EQ(poll.wait(1000).size(), 1u); // the data is still unread
}

TEST(Poll, ClosedPeerIsReadable) {
    SocketPair pair;
    utils::encapsulation::Poll poll;
    poll.link(pair.peer, EPOLLIN);
    pair.client.close();
    std::vector<struct epoll_event> events = poll.wait(1000);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_TRUE(events[0].events & EPOLLIN); // recv returns 0 like on Linux
}

TEST(Poll, UnlinkAndErrors) {
    SocketPair pair;
    utils::encapsulation::Poll poll;
    poll.link(pair.peer, EPOLLIN);
    EXPECT_THROW(poll.link(pair.peer, EPOLLIN), utils::exception::IException); // already linked
    poll.unlink(pair.peer);
    EXPECT_FALSE(poll.contains(pair.peer));
    EXPECT_THROW(poll.edit(pair.peer, EPOLLIN), utils::exception::IException); // not linked
    EXPECT_THROW(poll.link(pair.peer, EPOLLIN | EPOLLET), utils::exception::IException); // edge triggered: no equivalent

    utils::encapsulation::Pipe pipe;
    pipe.trigger();
    try {
        poll.link(pipe.getRead(), EPOLLIN); // WSAPoll only handles sockets
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::EPoll);
    }
}

/* -------------------------------- Process -------------------------------- */
TEST(Process, SpawnExitCode) {
    utils::encapsulation::Process process;
    ASSERT_GT(process.spawn("cmd.exe", {"/c", "exit", "3"}), 0);
    EXPECT_TRUE(process.is());
    EXPECT_TRUE(process.isParent());
    utils::encapsulation::Status status = process.wait();
    EXPECT_TRUE(status.exited);
    EXPECT_EQ(status.code, 3);
    EXPECT_FALSE(process.is());
}

TEST(Process, RedirectStdout) {
    utils::encapsulation::Pipe pipe;
    pipe.trigger();
    utils::encapsulation::Process process;
    process.dup(pipe.getWrite(), 1);
    ASSERT_GT(process.spawn("cmd.exe", {"/c", "echo", "hello"}), 0);
    pipe.closeWrite(); // the child has its own copy: EOF when it exits
    std::string out = readAll(pipe.getRead());
    EXPECT_EQ(process.wait().code, 0);
    EXPECT_NE(out.find("hello"), std::string::npos) << out;
}

TEST(Process, Kill) {
    utils::encapsulation::Pipe input;
    input.trigger(); // never written: the child waits
    utils::encapsulation::Process process;
    process.dup(input.getRead(), 0);
    ASSERT_GT(process.spawn("cmd.exe", {"/c", "pause"}), 0);
    EXPECT_TRUE(process.is());
    EXPECT_NO_THROW(process.kill());
    EXPECT_FALSE(process.is());
    EXPECT_EQ(process.getPid(), -1);
}

TEST(Process, UnknownProgram) {
    utils::encapsulation::Process process;
    try {
        (void)process.spawn("this-program-does-not-exist-utils.exe", {});
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Fork);
    }
}

TEST(ProcessDeathTest, ForkIsUndefined) {
    EXPECT_DEATH({utils::encapsulation::Process process; (void)process.spawn();}, "");
}

TEST(ProcessDeathTest, DupOutsideStandardFdsIsUndefined) {
    EXPECT_DEATH({
        utils::encapsulation::Pipe pipe;
        pipe.trigger();
        utils::encapsulation::Process process;
        process.dup(pipe.getWrite(), 5);
        (void)process.spawn("cmd.exe", {"/c", "exit", "0"});
    }, "");
}
