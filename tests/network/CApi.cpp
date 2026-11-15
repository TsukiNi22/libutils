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
##  Unit tests of the C API of utils::network (Unix: tools, TCP client / server, socket of the C side)
\**************************************************************/

#include "utils/network/CApi.h"
#include "tools/RawSocket.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <string>
#include <thread>

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

static bool contains(const utils_payloads_t* payloads, const std::string& s)
{
    for (size_t i = 0; i < utils_payloads_count(payloads); ++i)
        if (s == utils_payloads_get(payloads, i, nullptr)) return true;
    return false;
}

class NetworkCApiTest: public ::testing::Test {
    protected:
        void SetUp(void) override
        {
            utils::verbose::verbose = utils::verbose::Verbose::None;
            utils_exception_clear();
        };

        void TearDown(void) override
        {
            utils::verbose::verbose = utils::verbose::Verbose::Basic;
        };
};

/* socket of the C side on a socketpair: connect gives the first fd, listen fails */
static int pairConnect(_unused const utils_network_address_t* address, void* data)
{
    return static_cast<int*>(data)[0];
}

static int noListen(const utils_network_address_t* address, _unused void* data)
{
    (void)utils_fail(5, (std::string("no listen on port ") + std::to_string(address->port)).c_str());
    return -1;
}

//----------------------------------------------------------------//
/* TOOLS */

TEST_F(NetworkCApiTest, Tools)
{
    char buf[32] = {};

    EXPECT_TRUE(utils_network_is_ip("127.0.0.1"));
    EXPECT_FALSE(utils_network_is_ip("localhost"));
    EXPECT_STREQ(utils_network_resolve_hostname("localhost"), "127.0.0.1");
    EXPECT_EQ(utils_network_resolve_hostname_r("localhost", buf, sizeof(buf)), 9u);
    EXPECT_STREQ(buf, "127.0.0.1");
    EXPECT_FALSE(utils_network_is_ip(nullptr));
    EXPECT_TRUE(utils_exception.exception);
}

//----------------------------------------------------------------//
/* CLIENT / SERVER */

TEST_F(NetworkCApiTest, ClientServerExchange)
{
    const utils_network_address_t any = {"", "", 0};
    utils_socket_t* socket = utils_socket_create_tcp();
    utils_server_t* server = utils_server_create(socket, &any);
    utils_payloads_t* payloads = utils_payloads_create();
    utils_network_messages_t* messages = utils_network_messages_create();
    int fd = -1;

    ASSERT_NE(server, nullptr);
    ASSERT_EQ(utils_server_start(server), UTILS_OK);
    EXPECT_EQ(utils_server_get_status(server), C_API_NETWORK_STATUS_UP);
    const utils_network_address_t local = {"127.0.0.1", nullptr, tests::tools::local_port(utils_socket_get_fd(socket))};
    utils_client_t* client = utils_client_create(nullptr, &local);
    ASSERT_NE(client, nullptr);
    EXPECT_EQ(utils_client_get_status(client), C_API_NETWORK_STATUS_DOWN);
    ASSERT_EQ(utils_client_start(client), UTILS_OK);
    ASSERT_TRUE(waitFor([&](void) {(void)utils_server_listen(server, messages); return utils_server_get_fds(server, &fd, 1) == 1;}));

    // client -> server
    EXPECT_EQ(utils_client_send(client, "hello", 5), UTILS_OK);
    EXPECT_TRUE(waitFor([&](void) {
        if (utils_server_listen(server, messages) != UTILS_OK) return false;
        for (size_t i = 0; i < utils_network_messages_count(messages); ++i)
            for (size_t j = 0; utils_network_messages_fd(messages, i) == fd && j < utils_network_messages_size(messages, i); ++j)
                if (std::string(utils_network_messages_get(messages, i, j, nullptr)) == "hello") return true;
        return false;
    }));

    // server -> client
    EXPECT_EQ(utils_server_send_buffered(server, fd, "wor", 3), UTILS_OK);
    EXPECT_EQ(utils_server_send(server, fd, "ld", 2), UTILS_OK);
    EXPECT_TRUE(waitFor([&](void) {return utils_client_listen(client, payloads) == UTILS_OK && contains(payloads, "ld");}));
    EXPECT_EQ(utils_client_stop(client), UTILS_OK);
    EXPECT_EQ(utils_client_get_status(client), C_API_NETWORK_STATUS_DOWN);
    EXPECT_EQ(utils_server_kill(server), UTILS_OK);
    EXPECT_EQ(utils_server_get_status(server), C_API_NETWORK_STATUS_TERMINATED);
    EXPECT_EQ(utils_server_start(server), UTILS_KO);

    utils_network_messages_destroy(messages);
    utils_payloads_destroy(payloads);
    utils_client_destroy(client);
    utils_server_destroy(server);
    utils_socket_destroy(socket);
}

//----------------------------------------------------------------//
/* SOCKET OF THE C SIDE */

TEST_F(NetworkCApiTest, SocketOfTheCSide)
{
    int fds[2] = {-1, -1};
    const utils_socket_callbacks_t callbacks = {pairConnect, noListen, nullptr, nullptr, nullptr};
    utils_socket_t* socket = nullptr;
    utils_string_t* payload = utils_string_create(nullptr, 0);
    const utils_network_address_t address = {"127.0.0.1", nullptr, 42};
    char buf[8] = {};

    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    socket = utils_socket_create(&callbacks, fds);
    ASSERT_NE(socket, nullptr);
    EXPECT_TRUE(utils_socket_has_recv_overload(socket));
    EXPECT_EQ(utils_socket_listen(socket, &address), UTILS_KO);
    EXPECT_STREQ(utils_exception.what, "no listen on port 42");
    ASSERT_EQ(utils_socket_connect(socket, &address), UTILS_OK);
    EXPECT_EQ(utils_socket_get_fd(socket), fds[0]);
    EXPECT_EQ(utils_socket_set_payload_separator(socket, ";"), UTILS_OK);
    EXPECT_EQ(utils_socket_send(socket, "ab", 2, C_API_NETWORK_SELF), UTILS_OK);
    EXPECT_EQ(::read(fds[1], buf, sizeof(buf)), 3);
    EXPECT_STREQ(buf, "ab;");
    EXPECT_EQ(::write(fds[1], "xyz;", 4), 4);
    EXPECT_EQ(utils_socket_recv(socket, C_API_NETWORK_SELF, payload), UTILS_OK);
    EXPECT_STREQ(utils_string_data(payload), "xyz");
    EXPECT_TRUE(utils_socket_empty(socket, C_API_NETWORK_SELF));
    EXPECT_EQ(utils_socket_create(nullptr, nullptr), nullptr);
    EXPECT_STREQ(utils_exception.info, "The callbacks is NULL");
    utils_socket_destroy(socket); // closes fds[0]
    ::close(fds[1]);
    utils_string_destroy(payload);
}
