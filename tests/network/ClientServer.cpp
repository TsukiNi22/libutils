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
#include <sys/socket.h>
#include <algorithm>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

template<typename Fn>
static bool waitFor(Fn condition, std::chrono::milliseconds timeout = 2000ms)
{
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now() + timeout;
    while (!condition()) {
        if (std::chrono::steady_clock::now() > end) return false;
        std::this_thread::sleep_for(1ms);
    }
    return true;
}

static bool contains(const utils::network::Payloads& payloads, const std::string& s)
{
    return std::find(payloads.begin(), payloads.end(), s) != payloads.end();
}

// A started server on a free port & a client pointing on it
class ClientServerTest : public ::testing::Test {
    protected:
        std::shared_ptr<utils::network::socket::TCPSocket> _serverSocket = std::make_shared<utils::network::socket::TCPSocket>();
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
                std::make_shared<utils::network::socket::TCPSocket>(),
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
            if (!waitFor([&] {(void)this->_server->listen(); return this->_server->getFds().size() == 1;})) return -1;
            return this->_server->getFds().front();
        };

        // Listen on the server until the payload is received from the fd
        bool serverReceive(int fd, const std::string& payload)
        {
            return waitFor([&] {
                const std::unordered_map<int, utils::network::Payloads>& all = this->_server->listen();
                return all.contains(fd) && contains(all.at(fd), payload);
            });
        };

        // Listen on the client until the payload is received
        bool clientReceive(const std::string& payload)
        {
            return waitFor([&] {return contains(this->_client->listen(), payload);});
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
    utils::network::Client other(std::make_shared<utils::network::socket::TCPSocket>(), {{"127.0.0.1", ""}, this->_port});
    other.start();
    EXPECT_TRUE(waitFor([&] {(void)this->_server->listen(); return this->_server->getFds().size() == 2;}));
}

TEST_F(ClientServerTest, ClientHostname) {
    utils::network::Client client(std::make_shared<utils::network::socket::TCPSocket>(), {{"localhost", ""}, this->_port});
    EXPECT_NO_THROW(client.start());
    EXPECT_EQ(client.getStatus(), utils::network::Status::Up);
}

TEST_F(ClientServerTest, DefaultClientResolveLocalhost) {
    // The default client target localhost:8080, the hostname must be resolved before connecting
    utils::network::Client client;
    try {
        client.start();
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(std::string(e.info()).find("Invalid ip given"), std::string::npos) << e.info();
    }
}

TEST_F(ClientServerTest, ClientDisconnection) {
    ASSERT_NE(this->connect(), -1);
    this->_client->kill();
    EXPECT_EQ(this->_client->getStatus(), utils::network::Status::Terminated);
    EXPECT_TRUE(waitFor([&] {(void)this->_server->listen(); return this->_server->getFds().empty();}));
}

TEST_F(ClientServerTest, ServerDisconnection) {
    ASSERT_NE(this->connect(), -1);
    this->_server->kill();
    EXPECT_TRUE(waitFor([&] {(void)this->_client->listen(); return this->_client->getStatus() != utils::network::Status::Up;}));
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
    EXPECT_TRUE(waitFor([&] {return contains(this->_server->listen(fd), "direct");}));
}

TEST_F(ClientServerTest, ListenUnknownFd) {
    try {
        (void)this->_server->listen(12345);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::UnknownId);
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
    EXPECT_TRUE(waitFor([&] {
        const auto& all = this->_server->listen();
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
    const auto& all = this->_server->listen();
    EXPECT_TRUE(!all.contains(fd) || all.at(fd).empty());
}

/* buffering */
TEST_F(ClientServerTest, ClientBuffered) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    this->_client->send<true>("a");
    this->_client->send<true>("b");
    std::this_thread::sleep_for(20ms);
    const auto& before = this->_server->listen();
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
    EXPECT_TRUE(waitFor([&] {
        const auto& p = this->_client->listen();
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
    std::thread sender([&] {std::this_thread::sleep_for(30ms); this->_server->send(fd, "wake");});
    this->_client->join();
    EXPECT_TRUE(this->clientReceive("wake"));
    sender.join();
}

TEST_F(ClientServerTest, ServerJoinWakeOnData) {
    int fd = this->connect();
    ASSERT_NE(fd, -1);
    std::thread sender([&] {std::this_thread::sleep_for(30ms); this->_client->send("wake");});
    this->_server->join(fd);
    EXPECT_TRUE(this->serverReceive(fd, "wake"));
    sender.join();
}

TEST_F(ClientServerTest, JoinWhenDownReturnImmediately) {
    utils::network::Client client;
    EXPECT_NO_THROW(client.join());
    EXPECT_TRUE(client.listen().empty());
}
