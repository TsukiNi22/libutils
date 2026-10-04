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
##  @file Socket.cpp

File Description:
##  Unit tests of the sockets (ASocket buffering/framing, TCPSocket, address tools)
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <vector>

// TCPSocket that can adopt an existing fd (socketpair) to test the buffering logic
// (TCPSocket hide the high level recv/send/accept of ASocket with its raw overloads, the using restore them)
class PairSocket: public utils::network::TCPSocket {
    public:
        using utils::network::ASocket::recv;
        using utils::network::ASocket::send;
        using utils::network::ASocket::accept;
        using utils::network::TCPSocket::recv;
        using utils::network::TCPSocket::send;
        using utils::network::TCPSocket::accept;
        void adopt(int fd, bool server = false) {this->_fd = fd; this->_mode = server;};
};

static std::string readAvailable(int fd)
{
    std::string s;
    char buf[256];
    ssize_t n = 0;
    while ((n = ::recv(fd, buf, sizeof(buf), MSG_DONTWAIT)) > 0) s.append(buf, n);
    return s;
}

static bool isOpen(int fd) {return ::fcntl(fd, F_GETFD) != -1;}

class SocketTest: public ::testing::Test {
    protected:
        int _fds[2] = {-1, -1}; // [0] adopted by the socket, [1] peer
        PairSocket _socket;

        void SetUp(void) override
        {
            utils::verbose::verbose = utils::verbose::Verbose::None;
            ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, this->_fds), 0);
            this->_socket.adopt(this->_fds[0]);
        };
        void TearDown(void) override
        {
            if (this->_fds[1] != -1) ::close(this->_fds[1]);
            utils::verbose::verbose = utils::verbose::Verbose::Basic;
        };
        void peerWrite(const std::string& s) {ASSERT_EQ(::write(this->_fds[1], s.data(), s.size()), static_cast<ssize_t>(s.size()));};
};

/* recv */
TEST_F(SocketTest, RecvOnePayload) {
    this->peerWrite("hello\n");
    EXPECT_EQ(this->_socket.recv(), "hello");
    EXPECT_TRUE(this->_socket.empty());
}

TEST_F(SocketTest, RecvMultiplePayloads) {
    this->peerWrite("a\nb\nc\n");
    EXPECT_EQ(this->_socket.recv(), "a");
    EXPECT_FALSE(this->_socket.empty());
    EXPECT_EQ(this->_socket.recvAll(), (std::vector<std::string>{"b", "c"}));
    EXPECT_TRUE(this->_socket.empty());
}

TEST_F(SocketTest, RecvKeepIncompletePayload) {
    this->peerWrite("a\nincomplete");
    EXPECT_EQ(this->_socket.recv(), "a");
    EXPECT_TRUE(this->_socket.empty()); // no valid payload left
    this->peerWrite("-end\n");
    EXPECT_EQ(this->_socket.recv(), "incomplete-end");
}

TEST_F(SocketTest, RecvSmallChunks) {
    this->_socket.setChunckSize(2);
    this->peerWrite("a long payload\n");
    EXPECT_EQ(this->_socket.recv(), "a long payload");
}

TEST_F(SocketTest, RecvEmptyPayload) {
    this->peerWrite("\n");
    EXPECT_EQ(this->_socket.recv(), "");
}

TEST_F(SocketTest, RecvClosedPeer) {
    ::close(this->_fds[1]);
    this->_fds[1] = -1;
    try {
        (void)this->_socket.recv();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_TRUE(e.isNone());
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::SocketClosed);
    }
}

TEST_F(SocketTest, RecvOverflow) {
    this->_socket.setOverflow(4);
    this->peerWrite("123456789\n");
    try {
        (void)this->_socket.recv();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Socket);
    }
}

TEST_F(SocketTest, CustomStringSeparator) {
    this->_socket.setPayloadSeparator(std::string("\r\n"));
    this->peerWrite("one\r\ntwo\r\n");
    EXPECT_EQ(this->_socket.recv(), "one");
    EXPECT_EQ(this->_socket.recv(), "two");
}

TEST_F(SocketTest, CustomCharSeparator) {
    this->_socket.setPayloadSeparator('|');
    this->_socket.send("x");
    EXPECT_EQ(readAvailable(this->_fds[1]), "x|");
}

/* send */
TEST_F(SocketTest, SendAddSeparator) {
    this->_socket.send("hello");
    EXPECT_EQ(readAvailable(this->_fds[1]), "hello\n");
}

TEST_F(SocketTest, SendKeepExistingSeparator) {
    this->_socket.send("hello\n");
    EXPECT_EQ(readAvailable(this->_fds[1]), "hello\n");
}

TEST_F(SocketTest, SendBufferedThenFlush) {
    this->_socket.sendBuffered("a");
    this->_socket.sendBuffered("b");
    EXPECT_EQ(readAvailable(this->_fds[1]), "");
    this->_socket.flush();
    EXPECT_EQ(readAvailable(this->_fds[1]), "a\nb\n");
    this->_socket.flush(); // nothing more
    EXPECT_EQ(readAvailable(this->_fds[1]), "");
}

TEST_F(SocketTest, SendOverflow) {
    this->_socket.setOverflow(3);
    try {
        this->_socket.send("too long");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Socket);
    }
}

TEST_F(SocketTest, RoundTripBetweenSockets) {
    PairSocket other;
    other.adopt(this->_fds[1]);
    this->_fds[1] = -1; // owned by 'other'
    this->_socket.send("ping");
    EXPECT_EQ(other.recv(), "ping");
    other.send("pong");
    EXPECT_EQ(this->_socket.recv(), "pong");
}

/* server mode (fd as id) */
TEST_F(SocketTest, ServerModeRecvAllOnClientFd) {
    int peer[2];
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, peer), 0);
    PairSocket server;
    server.adopt(this->_fds[1], true); // fake listening fd
    this->_fds[1] = -1;

    ASSERT_EQ(::write(peer[1], "a\nb\nc\n", 6), 6);
    EXPECT_EQ(server.recv(peer[0]), "a");
    EXPECT_FALSE(server.empty(peer[0]));
    EXPECT_EQ(server.recvAll(peer[0]), (std::vector<std::string>{"b", "c"}));
    ::close(peer[0]);
    ::close(peer[1]);
}

TEST_F(SocketTest, ServerModeSendOnClientFd) {
    int peer[2];
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, peer), 0);
    PairSocket server;
    server.adopt(this->_fds[1], true);
    this->_fds[1] = -1;

    server.send("to client", peer[0]);
    EXPECT_EQ(readAvailable(peer[1]), "to client\n");
    ::close(peer[0]);
    ::close(peer[1]);
}

/* fd handling */
TEST(SocketFd, InvalidFd) {
    PairSocket socket;
    EXPECT_EQ(socket.getFd(), -1);
    try {
        (void)socket.recv();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::InvalidFd);
    }
    EXPECT_THROW((void)socket.empty(), utils::exception::IException);
    EXPECT_THROW(socket.send("x"), utils::exception::IException);
    EXPECT_THROW(socket.flush(), utils::exception::IException);
    EXPECT_THROW((void)socket.recvAll(), utils::exception::IException);
}

TEST(SocketFd, AcceptChecks) {
    PairSocket closed;
    try {
        (void)closed.accept();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::InvalidFd);
    }

    int fds[2];
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    PairSocket client;
    client.adopt(fds[0], false);
    try {
        (void)client.accept();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::InvalidAction);
    }
    ::close(fds[1]);
}

TEST(SocketFd, CloseAndReset) {
    int fds[2];
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    {
        PairSocket socket;
        socket.adopt(fds[0]);
        socket.reset(); // does not close
        EXPECT_EQ(socket.getFd(), -1);
        EXPECT_TRUE(isOpen(fds[0]));

        socket.adopt(fds[0]);
        socket.close();
        EXPECT_EQ(socket.getFd(), -1);
        EXPECT_FALSE(isOpen(fds[0]));
    }
    {
        ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
        PairSocket socket;
        socket.adopt(fds[0]);
    } // destructor close
    EXPECT_FALSE(isOpen(fds[0]));
    ::close(fds[1]);
}

TEST(SocketFd, OverloadFlags) {
    PairSocket socket;
    EXPECT_TRUE(socket.hasAcceptOverload());
    EXPECT_TRUE(socket.hasRecvOverload());
    EXPECT_TRUE(socket.hasSendOverload());
}

/* tools */
TEST(SocketTools, IsIp) {
    EXPECT_TRUE(utils::network::is_ip("127.0.0.1"));
    EXPECT_TRUE(utils::network::is_ip("255.255.255.255"));
    EXPECT_FALSE(utils::network::is_ip("256.0.0.1"));
    EXPECT_FALSE(utils::network::is_ip("localhost"));
    EXPECT_FALSE(utils::network::is_ip(""));
    EXPECT_FALSE(utils::network::is_ip("1.2.3"));
}

TEST(SocketTools, ResolveHostname) {
    utils::verbose::verbose = utils::verbose::Verbose::None;
    EXPECT_EQ(utils::network::resolve_hostname("localhost"), "127.0.0.1");
    EXPECT_THROW((void)utils::network::resolve_hostname("this.host.does.not.exist.invalid"), utils::exception::IException);
    utils::verbose::verbose = utils::verbose::Verbose::Basic;
}

TEST(SocketTools, ResolveAddress) {
    utils::verbose::verbose = utils::verbose::Verbose::None;
    utils::network::Address address{{"localhost", ""}, 1234};
    utils::network::resolve_address(address);
    EXPECT_EQ(address.ip.first, "127.0.0.1");
    EXPECT_EQ(address.ip.second, "localhost");
    EXPECT_EQ(address.port, 1234);

    utils::network::Address ip{{"10.0.0.1", ""}, 1};
    utils::network::resolve_address(ip);
    EXPECT_EQ(ip.ip.first, "10.0.0.1"); // already an ip, untouched
    utils::verbose::verbose = utils::verbose::Verbose::Basic;
}

/* TCP connection */
class TCPSocketTest: public ::testing::Test {
    protected:
        void SetUp(void) override {utils::verbose::verbose = utils::verbose::Verbose::None;};
        void TearDown(void) override {utils::verbose::verbose = utils::verbose::Verbose::Basic;};

        static std::uint16_t portOf(int fd)
        {
            sockaddr_in addr{};
            socklen_t len = sizeof(addr);
            ::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len);
            return ntohs(addr.sin_port);
        };
};

TEST_F(TCPSocketTest, ListenConnectAccept) {
    PairSocket server, client;
    ASSERT_NO_THROW(server.listen({{"", ""}, 0}));
    std::uint16_t port = portOf(server.getFd());
    ASSERT_NE(port, 0);

    ASSERT_NO_THROW(client.connect({{"127.0.0.1", ""}, port}));
    int fd = server.accept();
    EXPECT_GE(fd, 0);

    client.send("hello");
    EXPECT_EQ(server.recv(fd), "hello");
    server.send("world", fd);
    EXPECT_EQ(client.recv(), "world");
    ::close(fd);
}

TEST_F(TCPSocketTest, ListenTwiceThrows) {
    PairSocket server;
    server.listen({{"", ""}, 0});
    try {
        server.listen({{"", ""}, 0});
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::SocketInit);
    }
}

TEST_F(TCPSocketTest, ConnectInvalidIp) {
    PairSocket client;
    try {
        client.connect({{"not an ip", ""}, 80});
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::SocketInit);
    }
}

TEST_F(TCPSocketTest, ConnectRefused) {
    // Get a free port and close it
    std::uint16_t port = 0;
    {
        PairSocket tmp;
        tmp.listen({{"", ""}, 0});
        port = portOf(tmp.getFd());
    }
    PairSocket client;
    EXPECT_THROW(client.connect({{"127.0.0.1", ""}, port}), utils::exception::IException);
}

TEST_F(TCPSocketTest, ConnectTwiceThrows) {
    PairSocket server, client;
    server.listen({{"", ""}, 0});
    client.connect({{"127.0.0.1", ""}, portOf(server.getFd())});
    EXPECT_THROW(client.connect({{"127.0.0.1", ""}, portOf(server.getFd())}), utils::exception::IException);
}

TEST_F(TCPSocketTest, ReconnectAfterClose) {
    PairSocket server, client;
    server.listen({{"", ""}, 0});
    std::uint16_t port = portOf(server.getFd());
    client.connect({{"127.0.0.1", ""}, port});
    client.close();
    EXPECT_NO_THROW(client.connect({{"127.0.0.1", ""}, port}));
}

/* -------------------------------- edge cases -------------------------------- */
TEST_F(SocketTest, OverflowZeroIsUnlimited) {
    this->_socket.setOverflow(0);
    EXPECT_NO_THROW(this->_socket.send(std::string(10000, 'a')));
    this->peerWrite(std::string(10000, 'b') + "\n");
    EXPECT_EQ(this->_socket.recv().size(), 10000u);
}

TEST_F(SocketTest, RawModeEmptySeparator) {
    this->_socket.setPayloadSeparator(std::string(""));
    this->peerWrite("data");
    EXPECT_EQ(this->_socket.recv(), "data");
    EXPECT_TRUE(this->_socket.empty());
    this->peerWrite("more");
    EXPECT_EQ(this->_socket.receive(), 4u);
    EXPECT_EQ(this->_socket.recvAll(), (std::vector<std::string>{"more"}));
}

TEST_F(SocketTest, ReceiveReadOnce) {
    this->peerWrite("partial");
    EXPECT_EQ(this->_socket.receive(), 7u); // never wait for the separator
    EXPECT_TRUE(this->_socket.empty());
    this->peerWrite(" end\nnext");
    (void)this->_socket.receive();
    EXPECT_EQ(this->_socket.recvAll(), (std::vector<std::string>{"partial end"}));
}

TEST_F(SocketTest, DiscardForgetBuffers) {
    this->peerWrite("left\nover");
    (void)this->_socket.receive();
    this->_socket.discard(this->_fds[0]);
    EXPECT_TRUE(this->_socket.empty());
}

TEST_F(SocketTest, DiscardEveryFd) {
    this->peerWrite("left\nover");
    (void)this->_socket.receive();
    this->_socket.discard();
    EXPECT_TRUE(this->_socket.empty());
}

TEST_F(TCPSocketTest, FailedConnectCloseTheSocket) {
    std::uint16_t port = 0;
    {
        utils::network::TCPSocket tmp;
        tmp.listen({{"", ""}, 0});
        port = portOf(tmp.getFd());
    }
    utils::network::TCPSocket client;
    EXPECT_THROW(client.connect({{"127.0.0.1", ""}, port}), utils::exception::IException);
    EXPECT_EQ(client.getFd(), -1);
}
