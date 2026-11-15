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
##  Unit tests of the C API of utils::encapsulation (Unix: dup, pipe, poll, process, shared object, shared memory)
\**************************************************************/

#include "utils/encapsulation/CApi.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <thread>
#include <string>
#include <chrono>

class EncapsulationCApiTest: public ::testing::Test {
    protected:
        void SetUp(void) override
        {
            utils_exception_clear();
        };
};

//----------------------------------------------------------------//
/* DUP / PIPE */

TEST_F(EncapsulationCApiTest, PipeAndDup)
{
    utils_pipe_t* pipe = utils_pipe_create(-1, -1);
    char buf[4] = {};

    ASSERT_NE(pipe, nullptr);
    ASSERT_EQ(utils_pipe_trigger(pipe), UTILS_OK);
    EXPECT_GE(utils_pipe_get_read(pipe), 0);
    EXPECT_EQ(utils_pipe_trigger(pipe), UTILS_KO); // already open
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::Pipe));

    utils_dup_t* dup = utils_dup_create(utils_pipe_get_write(pipe), -1);
    ASSERT_NE(dup, nullptr);
    ASSERT_EQ(utils_dup_trigger(dup), UTILS_OK);
    EXPECT_GE(utils_dup_get_clone(dup), 0);
    EXPECT_EQ(::write(utils_dup_get_clone(dup), "ab", 2), 2);
    EXPECT_EQ(::read(utils_pipe_get_read(pipe), buf, sizeof(buf)), 2);
    EXPECT_STREQ(buf, "ab");
    EXPECT_EQ(utils_dup_close_clone(dup), UTILS_OK);
    EXPECT_EQ(utils_dup_get_clone(dup), -1);
    EXPECT_EQ(utils_dup_clear(dup), UTILS_OK); // the origin is the write fd of the pipe
    EXPECT_EQ(utils_pipe_close(pipe), UTILS_OK);
    EXPECT_EQ(utils_pipe_get_read(pipe), -1);
    EXPECT_EQ(utils_dup_trigger(dup), UTILS_KO);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::Dup));
    utils_dup_destroy(dup);
    utils_pipe_destroy(pipe);
}

//----------------------------------------------------------------//
/* POLL */

TEST_F(EncapsulationCApiTest, PollWaitsOnAPipe)
{
    utils_pipe_t* pipe = utils_pipe_create(-1, -1);
    utils_poll_t* poll = utils_poll_create(EPOLL_CLOEXEC);
    struct epoll_event events[2] = {};

    ASSERT_NE(poll, nullptr);
    ASSERT_EQ(utils_pipe_trigger(pipe), UTILS_OK);
    EXPECT_GE(utils_poll_get_fd(poll), 0);
    EXPECT_EQ(utils_poll_link(poll, utils_pipe_get_read(pipe), EPOLLIN, nullptr), UTILS_OK);
    EXPECT_TRUE(utils_poll_contains(poll, utils_pipe_get_read(pipe)));
    EXPECT_EQ(utils_poll_size(poll), 1u);
    EXPECT_EQ(utils_poll_wait(poll, 0, events, 2), 0u);
    EXPECT_FALSE(utils_exception.exception);
    EXPECT_EQ(::write(utils_pipe_get_write(pipe), "x", 1), 1);
    EXPECT_EQ(utils_poll_wait(poll, 100, events, 2), 1u);
    EXPECT_TRUE(events[0].events & EPOLLIN);
    EXPECT_EQ(utils_poll_wait(poll, 0, events, 0), 0u);
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_poll_unlink(poll, utils_pipe_get_read(pipe)), UTILS_OK);
    EXPECT_EQ(utils_poll_size(poll), 0u);
    utils_poll_destroy(poll);
    utils_pipe_destroy(pipe);
}

//----------------------------------------------------------------//
/* PROCESS */

TEST_F(EncapsulationCApiTest, ProcessSpawnExecAndWait)
{
    const char* args[] = {"-c", "exit 3"};
    utils_process_t* process = utils_process_create();
    utils_process_status_t status = {};

    ASSERT_NE(process, nullptr);
    EXPECT_FALSE(utils_process_is(process));
    EXPECT_EQ(utils_process_get_pid(process), -1);
    ASSERT_GT(utils_process_spawn_exec(process, "/bin/sh", args, 2), 0);
    EXPECT_TRUE(utils_process_is_parent(process));
    EXPECT_FALSE(utils_process_is_child(process));
    ASSERT_EQ(utils_process_wait(process, &status), UTILS_OK);
    EXPECT_TRUE(status.exited);
    EXPECT_EQ(status.code, 3);
    EXPECT_EQ(utils_process_spawn_exec(process, nullptr, args, 2), -1);
    EXPECT_STREQ(utils_exception.info, "The path is NULL");
    utils_process_destroy(process);
}

TEST_F(EncapsulationCApiTest, ProcessPipesAndDups)
{
    utils_process_t* process = utils_process_create();
    utils_pipe_t* pipe = utils_pipe_create(-1, -1);
    int read = 0;
    int clone = 0;

    EXPECT_EQ(utils_process_add_pipe(process, pipe), UTILS_OK);
    EXPECT_EQ(utils_pipe_get_read(pipe), -1); // moved in the process
    EXPECT_EQ(utils_process_dup(process, 1, 7), UTILS_OK);
    EXPECT_EQ(utils_process_pipes_count(process), 1u);
    EXPECT_EQ(utils_process_dups_count(process), 1u);
    EXPECT_EQ(utils_process_get_pipe(process, 0, &read, nullptr), UTILS_OK);
    EXPECT_EQ(read, -1);
    EXPECT_EQ(utils_process_get_dup(process, 0, nullptr, &clone), UTILS_OK);
    EXPECT_EQ(clone, 7);
    EXPECT_EQ(utils_process_get_dup(process, 1, nullptr, &clone), UTILS_KO);
    EXPECT_EQ(utils_process_get_dup(process, 0, nullptr, nullptr), UTILS_OK);
    utils_pipe_destroy(pipe);
    utils_process_destroy(process);
}

//----------------------------------------------------------------//
/* SHARED OBJECT */

TEST_F(EncapsulationCApiTest, SharedObject)
{
    utils_shared_object_t* so = utils_shared_object_create("libm.so.6");

    ASSERT_NE(so, nullptr);
    EXPECT_TRUE(utils_shared_object_is_loaded(so));
    EXPECT_STREQ(utils_shared_object_path(so), "libm.so.6");
    EXPECT_NE(utils_shared_object_get(so), nullptr);
    void* fn = utils_shared_object_load_function(so, "cos");
    ASSERT_NE(fn, nullptr);
    EXPECT_DOUBLE_EQ(reinterpret_cast<double (*)(double)>(fn)(0.0), 1.0);
    EXPECT_EQ(utils_shared_object_load_function(so, "not_a_function_of_libm"), nullptr);
    EXPECT_TRUE(utils_exception.exception);
    utils_shared_object_destroy(so);
    EXPECT_EQ(utils_shared_object_create("/nonexistent/libnothing.so"), nullptr);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::Dlopen));
}

//----------------------------------------------------------------//
/* SHARED MEMORY */

static void SharedMemoryScenario(const std::string& name)
{
    utils_shm_t* server = utils_shm_create(1001);
    utils_shm_t* client = utils_shm_create(1002);
    const utils_shm_target_t target = {0, 1, 1002, false};
    utils_shm_id_t id = {};
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now() + std::chrono::seconds{2};
    size_t size = 0;

    ASSERT_NE(server, nullptr);
    ASSERT_EQ(utils_shm_init(server, name.c_str(), 64, 4, true, C_API_SHM_LAYOUT_COMPACT_SEMI_ALIGNED, 2, false), UTILS_OK);
    ASSERT_EQ(utils_shm_init(client, name.c_str(), 0, 0, false, C_API_SHM_LAYOUT_COMPACT, 2, false), UTILS_OK);
    EXPECT_EQ(utils_shm_get_ownership(client), 1002);
    EXPECT_FALSE(utils_shm_readable(client, C_API_SHM_READ_ALL));
    ASSERT_EQ(utils_shm_send(server, "hello", 5, &target, false, false, true, &id), UTILS_OK);
    EXPECT_NE(id.id, 0u);
    while (!utils_shm_readable(client, C_API_SHM_READ_ALL) && std::chrono::steady_clock::now() < end)
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
    utils_shm_messages_t* messages = utils_shm_read(client, C_API_SHM_READ_ALL);
    ASSERT_NE(messages, nullptr);
    ASSERT_EQ(utils_shm_messages_count(messages), 1u);
    ASSERT_EQ(utils_shm_messages_size(messages, 0), 1u);
    const void* payload = utils_shm_messages_get(messages, 0, 0, &size);
    ASSERT_NE(payload, nullptr);
    EXPECT_EQ(size, 5u);
    EXPECT_EQ(std::memcmp(payload, "hello", 5), 0);
    EXPECT_EQ(utils_shm_messages_get(messages, 0, 1, &size), nullptr);
    utils_shm_messages_destroy(messages);
    EXPECT_EQ(utils_shm_read(client, C_API_SHM_READ_ALL), nullptr);
    EXPECT_EQ(utils_shm_init(server, name.c_str(), 64, 4, true, C_API_SHM_LAYOUT_COMPACT_SEMI_ALIGNED, 3, false), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The group size must be a power of 2 from 1 to 64");
    utils_shm_destroy(client);
    utils_shm_destroy(server);
}

TEST_F(EncapsulationCApiTest, SharedMemory)
{
    const std::string name = "/utils-capi-tests-" + std::to_string(::getpid());

    EXPECT_EXIT({::alarm(10); SharedMemoryScenario(name); std::exit(::testing::Test::HasFailure() ? 1 : 0);}, ::testing::ExitedWithCode(0), "");
    ::shm_unlink(name.c_str());
}
