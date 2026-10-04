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
##  @file ClientServer.cpp

File Description:
##  Unit tests of the network Client & Server (status, connection, exchange, buffering)
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <algorithm>
#include <filesystem>
#include <atomic>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>


template<typename Fn>
static bool waitFor(Fn condition, std::chrono::milliseconds timeout = std::chrono::milliseconds{2000})
{
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now() + timeout;
    while (!condition()) {
        if (std::chrono::steady_clock::now() > end) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }
    return true;
}

// Number of fds opened by the process
static std::size_t openFds(void)
{
    std::size_t count = 0;
    for (_unused const std::filesystem::directory_entry& entry: std::filesystem::directory_iterator("/proc/self/fd")) ++count;
    return count;
}

static bool contains(const utils::network::Payloads& payloads, const std::string& s)
{
    return std::find(payloads.begin(), payloads.end(), s) != payloads.end();
}

// A started server on a free port & a client pointing on it
class ClientServerTest: public ::testing::Test {
    protected:
        std::shared_ptr<utils::network::TCPSocket> _serverSocket = std::make_shared<utils::network::TCPSocket>();
        std::unique_ptr<utils::network::Server> _server;
        std::unique_ptr<utils::network::Client> _client;
        std::uint16_t _port = 0;

        void SetUp(void) override
        {
            utils::verbose::verbose = utils::verbose::Verbose::None;
            this->_server = std::make_unique<utils::network::Server>(this->_serverSocket, utils::network::Address{{"", ""}, 0});
            ASSERT_NO_THROW(this->_server->start());

            sockaddr_in addr{};
            socklen_t len = sizeof(addr);
            ::getsockname(this->_serverSocket->getFd(), reinterpret_cast<sockaddr*>(&addr), &len);
            this->_port = ntohs(addr.sin_port);

            this->_client = std::make_unique<utils::network::Client>(
                std::make_shared<utils::network::TCPSocket>(),
                utils::network::Address{{"127.0.0.1", ""}, this->_port}
            );
        };
        void TearDown(void) override
        {
            this->_client.reset();
            this->_server.reset();
            utils::verbose::verbose = utils::verbose::Verbose::Basic;
        };

        // Start the client and wait for the server to accept it, return the client fd on the server
        int connect(void)
        {
            this->_client->start();
            if (!waitFor([&](void) {(void)this->_server->listen(); return this->_server->getFds().size() == 1;})) return -1;
            return this->_server->getFds().front();
        };

        // Listen on the server until the payload is received from the fd
        bool serverReceive(int fd, const std::string& payload)
        {
            return waitFor([&](void) {
                const std::unordered_map<int, utils::network::Payloads>& all = this->_server->listen();
                return all.contains(fd) && contains(all.at(fd), payload);
            });
        };

        // Listen on the client until the payload is received
        bool clientReceive(const std::string& payload)
        {
            return waitFor([&](void) {return contains(this->_client->listen(), payload);});
        };
};

/* status */
TEST_F(ClientServerTest, InitialStatus) {
    EXPECT_EQ(this->_server->getStatus(), utils::network::Status::Up);
    EXPECT_EQ(this->_client->getStatus(), utils::network::Status::Down);
    EXPECT_TRUE(this->_server->getFds().empty());
}

TEST_F(ClientServerTest, StartTwice) {
    try {
        this->_server->start();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::AlreadyRunning);
    }
}

TEST_F(ClientServerTest, StopAndRestart) {
    this->_server->stop();
    EXPECT_EQ(this->_server->getStatus(), utils::network::Status::Down);
    ASSERT_NO_THROW(this->_server->start());
    EXPECT_EQ(this->_server->getStatus(), utils::network::Status::Up);
}

TEST_F(ClientServerTest, KillIsDefinitive) {
    this->_server->kill();
    EXPECT_EQ(this->_server->getStatus(), utils::network::Status::Terminated);
    try {
        this->_server->start();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Killed);
    }
}

TEST_F(ClientServerTest, ClientConnectionRefused) {
    this->_server->kill();
    EXPECT_THROW(this->_client->start(), utils::exception::IException);
    EXPECT_EQ(this->_client->getStatus(), utils::network::Status::Crashed);
}

/* connection */
TEST_F(ClientServerTest, Connect) {
    int fd = this->connect();
    EXPECT_NE(fd, -1);
    EXPECT_EQ(this->_client->getStatus(), utils::network::Status::Up);
}

TEST_F(ClientServerTest, MultipleClients) {
    ASSERT_NE(this->connect(), -1);
    utils::network::Client other(std::make_shared<utils::network::TCPSocket>(), {{"127.0.0.1", ""}, this->_port});
    other.start();
    EXPECT_TRUE(waitFor([&](void) {(void)this->_server->listen(); return this->_server->getFds().size() == 2;}));
}

TEST_F(ClientServerTest, ClientHostname) {
    utils::network::Client client(std::make_shared<utils::network::TCPSocket>(), {{"localhost", ""}, this->_port});
    EXPECT_NO_THROW(client.start());
    EXPECT_EQ(client.getStatus(), utils::network::Status::Up);
}

TEST_F(ClientServerTest, ClientDisconnection) {
    ASSERT_NE(this->connect(), -1);
    this->_client->kill();
    EXPECT_EQ(this->_client->getStatus(), utils::network::Status::Terminated);
    EXPECT_TRUE(waitFor([&](void) {(void)this->_server->listen(); return this->_server->getFds().empty();}));
}

TEST_F(ClientServerTest, ServerDisconnection) {
    ASSERT_NE(this->connect(), -1);
    this->_server->kill();
    EXPECT_TRUE(waitFor([&](void) {(void)this->_client->listen(); return this->_client->getStatus() != utils::network::Status::Up;}));
    EXPECT_EQ(this->_client->getStatus(), utils::network::Status::Down);
}

/* exchange */
TEST_F(ClientServerTest, ClientToServer) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    this->_client->send("hello");
    EXPECT_TRUE(this->serverReceive(fd, "hello"));
}

TEST_F(ClientServerTest, ServerToClient) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    this->_server->send(fd, "world");
    EXPECT_TRUE(this->clientReceive("world"));
}

TEST_F(ClientServerTest, ListenSpecificFd) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    this->_client->send("direct");
    EXPECT_TRUE(waitFor([&](void) {return contains(this->_server->listen(fd), "direct");}));
}

TEST_F(ClientServerTest, ListenUnknownFd) {
    try {
        (void)this->_server->listen(12345);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::UnknownFd);
    }
}

TEST_F(ClientServerTest, SendUnknownFd) {
    EXPECT_THROW(this->_server->send(12345, "x"), utils::exception::IException);
}

TEST_F(ClientServerTest, ManyPayloadsInOrder) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    for (int i = 0; i < 50; ++i) this->_client->send("msg" + std::to_string(i));

    std::vector<std::string> received;
    EXPECT_TRUE(waitFor([&](void) {
        const std::unordered_map<int, utils::network::Payloads>& all = this->_server->listen();
        if (all.contains(fd)) received.insert(received.end(), all.at(fd).begin(), all.at(fd).end());
        return received.size() >= 50;
    }));
    ASSERT_EQ(received.size(), 50u);
    for (int i = 0; i < 50; ++i) EXPECT_EQ(received[i], "msg" + std::to_string(i));
}

TEST_F(ClientServerTest, ListenClearBetweenCalls) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    this->_client->send("once");
    ASSERT_TRUE(this->serverReceive(fd, "once"));
    const std::unordered_map<int, utils::network::Payloads>& all = this->_server->listen();
    EXPECT_TRUE(!all.contains(fd) || all.at(fd).empty());
}

/* buffering */
TEST_F(ClientServerTest, ClientBuffered) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    this->_client->send<true>("a");
    this->_client->send<true>("b");
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
    const std::unordered_map<int, utils::network::Payloads>& before = this->_server->listen();
    EXPECT_TRUE(!before.contains(fd) || before.at(fd).empty());

    this->_client->flush();
    EXPECT_TRUE(this->serverReceive(fd, "b"));
}

TEST_F(ClientServerTest, ServerBuffered) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    this->_server->send<true>(fd, "x");
    this->_server->send<true>(fd, "y");
    this->_server->flush(fd);
    std::vector<std::string> received;
    EXPECT_TRUE(waitFor([&](void) {
        const utils::network::Payloads& p = this->_client->listen();
        received.insert(received.end(), p.begin(), p.end());
        return received.size() >= 2;
    }));
    EXPECT_EQ(received, (std::vector<std::string>{"x", "y"}));
}

TEST_F(ClientServerTest, ServerFlushAll) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    this->_server->send<true>(fd, "all");
    this->_server->flush();
    EXPECT_TRUE(this->clientReceive("all"));
}

/* join */
TEST_F(ClientServerTest, ClientJoinWakeOnData) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    std::thread sender([&](void) {std::this_thread::sleep_for(std::chrono::milliseconds{30}); this->_server->send(fd, "wake");});
    this->_client->join();
    EXPECT_TRUE(this->clientReceive("wake"));
    sender.join();
}

TEST_F(ClientServerTest, ServerJoinWakeOnData) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    std::thread sender([&](void) {std::this_thread::sleep_for(std::chrono::milliseconds{30}); this->_client->send("wake");});
    this->_server->join(fd);
    EXPECT_TRUE(this->serverReceive(fd, "wake"));
    sender.join();
}

TEST_F(ClientServerTest, JoinWhenDownReturnImmediately) {
    utils::network::Client client;
    EXPECT_NO_THROW(client.join());
    EXPECT_TRUE(client.listen().empty());
}

/* -------------------------------- edge cases -------------------------------- */
// Raw tcp client (to send partial payloads)
static int rawConnect(std::uint16_t port)
{
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {::close(fd); return -1;}
    return fd;
}

TEST_F(ClientServerTest, SendToClosedPeerNoSigpipe) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    this->_client->kill();
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
    for (int i = 0; i < 5; ++i) {
        try {this->_server->send(fd, "hello");}
        catch (const utils::exception::IException& e) {EXPECT_EQ(e.getCode(), utils::exception::InternalCode::UnknownFd);} // already removed
    }
    EXPECT_TRUE(this->_server->getFds().empty());
}

TEST_F(ClientServerTest, PartialPayloadDoesNotBlock) {
    int fd = rawConnect(this->_port);
    ASSERT_NE(fd, -1);
    ASSERT_TRUE(waitFor([&](void) {(void)this->_server->listen(); return this->_server->getFds().size() == 1;}));
    int sfd = this->_server->getFds().front();
    ASSERT_EQ(::write(fd, "partial", 7), 7);
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    (void)this->_server->listen();
    EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::milliseconds{500});
    ASSERT_EQ(::write(fd, " end\n", 5), 5);
    EXPECT_TRUE(this->serverReceive(sfd, "partial end"));
    ::close(fd);
}

TEST_F(ClientServerTest, ListenFdWithOtherClientsPending) {
    int a = rawConnect(this->_port), b = rawConnect(this->_port);
    ASSERT_TRUE(waitFor([&](void) {(void)this->_server->listen(); return this->_server->getFds().size() == 2;}));
    ASSERT_EQ(::write(a, "x\n", 2), 2);
    ASSERT_EQ(::write(b, "y\n", 2), 2);
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
    std::vector<int> fds = this->_server->getFds();
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    (void)this->_server->listen(fds[0]);
    (void)this->_server->listen(fds[1]);
    EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::milliseconds{500});
    ::close(a);
    ::close(b);
}

TEST_F(ClientServerTest, ClosedConnectionBufferNotReused) {
    int a = rawConnect(this->_port);
    ASSERT_TRUE(waitFor([&](void) {(void)this->_server->listen(); return this->_server->getFds().size() == 1;}));
    ASSERT_EQ(::write(a, "ok\nGARBAGE", 10), 10);
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
    (void)this->_server->listen();
    ::close(a);
    ASSERT_TRUE(waitFor([&](void) {(void)this->_server->listen(); return this->_server->getFds().empty();}));

    int b = rawConnect(this->_port); // usually the same fd number
    ASSERT_TRUE(waitFor([&](void) {(void)this->_server->listen(); return this->_server->getFds().size() == 1;}));
    int fd = this->_server->getFds().front();
    ASSERT_EQ(::write(b, "hello\n", 6), 6);
    EXPECT_TRUE(this->serverReceive(fd, "hello"));
    ::close(b);
}

TEST_F(ClientServerTest, RemovedClientNotResurrected) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    this->_client->kill();
    EXPECT_TRUE(waitFor([&](void) {
        try {(void)this->_server->listen(fd);} catch (const utils::exception::IException&) {} // unknown once removed
        return this->_server->getFds().empty();
    }));
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
    EXPECT_TRUE(this->_server->getFds().empty());
}

TEST_F(ClientServerTest, FlushAllWithClosedClients) {
    std::vector<int> raw;
    for (int i = 0; i < 4; ++i) raw.push_back(rawConnect(this->_port));
    ASSERT_TRUE(waitFor([&](void) {(void)this->_server->listen(); return this->_server->getFds().size() == 4;}));
    for (int fd: this->_server->getFds()) this->_server->send<true>(fd, std::string(1000, 'x'));
    for (int fd: raw) {
        struct linger l{1, 0}; // reset the connection
        ::setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
        ::close(fd);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
    EXPECT_NO_THROW(this->_server->flush());
    EXPECT_NO_THROW(this->_server->flush());
}

TEST_F(ClientServerTest, ClientRestartAfterFailedStart) {
    this->_server->kill();
    EXPECT_THROW(this->_client->start(), utils::exception::IException);
    EXPECT_THROW(this->_client->start(), utils::exception::IException);
    EXPECT_NE(this->_client->getStatus(), utils::network::Status::Up);
}

TEST_F(ClientServerTest, StopFromAnotherThreadDuringJoin) {
    ASSERT_NE(this->connect(), -1);
    std::thread stopper([&](void) {std::this_thread::sleep_for(std::chrono::milliseconds{30}); this->_server->stop();});
    EXPECT_NO_THROW(this->_server->join());
    stopper.join();
    EXPECT_EQ(this->_server->getStatus(), utils::network::Status::Down);
}

TEST_F(ClientServerTest, KillFromAnotherThreadDuringJoin) {
    ASSERT_NE(this->connect(), -1);
    std::thread killer([&](void) {std::this_thread::sleep_for(std::chrono::milliseconds{30}); this->_server->kill();});
    EXPECT_NO_THROW(this->_server->join());
    killer.join();
    EXPECT_EQ(this->_server->getStatus(), utils::network::Status::Terminated);
}

TEST_F(ClientServerTest, ClientStopFromAnotherThreadDuringJoin) {
    ASSERT_NE(this->connect(), -1);
    std::thread stopper([&](void) {std::this_thread::sleep_for(std::chrono::milliseconds{30}); this->_client->stop();});
    EXPECT_NO_THROW(this->_client->join());
    stopper.join();
    EXPECT_EQ(this->_client->getStatus(), utils::network::Status::Down);
}

TEST_F(ClientServerTest, StopDuringJoinClosesEpoll) {
    std::size_t before = openFds();
    std::atomic<bool> joined = false;
    std::thread joiner([&](void) {this->_server->join(); joined = true;});
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
    this->_server->stop(); // the epoll is still used by the join: closed when it leaves
    EXPECT_TRUE(waitFor([&](void) {return joined.load();}));
    joiner.join();
    EXPECT_EQ(openFds(), before - 2); // server socket & epoll closed, nothing leaked
}
