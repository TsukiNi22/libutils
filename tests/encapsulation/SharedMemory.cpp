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
##  @file SharedMemory.cpp

File Description:
##  Unit tests of the SharedMemory (shm communication between owners)
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdlib>
#include <chrono>
#include <cstddef>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>

using namespace std::chrono_literals;
namespace shm = utils::encapsulation::shm;

static std::vector<std::byte> toBytes(const std::string& s)
{
    std::vector<std::byte> bytes(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) bytes[i] = static_cast<std::byte>(s[i]);
    return bytes;
}

static std::string toString(const std::vector<std::byte>& bytes)
{
    std::string s;
    for (std::byte b: bytes) s += static_cast<char>(b);
    return s;
}

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

// Run the scenario in a sub-process with a timeout: a hang (or crash) of the shared memory is reported as a failure
#define SHM_ISOLATED(...) EXPECT_EXIT({::alarm(10); __VA_ARGS__; std::exit(::testing::Test::HasFailure() ? 1 : 0);}, ::testing::ExitedWithCode(0), "")

class SharedMemoryTest : public ::testing::Test {
    protected:
        std::string _name;

        void SetUp(void) override
        {
            static int counter = 0;
            this->_name = "/utils-tests-" + std::to_string(::getpid()) + "-" + std::to_string(counter++);
        };
        void TearDown(void) override {::shm_unlink(this->_name.c_str());};
};

static void CreateAndOpenScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    ASSERT_NO_THROW(server.template init<true>(name, 64, 4));
    utils::encapsulation::SharedMemory client(pid_t{1002});
    ASSERT_NO_THROW(client.init(name));
    EXPECT_EQ(server.ownership(), 1001);
    EXPECT_EQ(client.ownership(), 1002);
    EXPECT_FALSE(client.readable());
}
TEST_F(SharedMemoryTest, CreateAndOpen) {SHM_ISOLATED(CreateAndOpenScenario(this->_name));}

static void DefaultOwnershipIsPidScenario(_unused const std::string& name)
{
    utils::encapsulation::SharedMemory memory;
    EXPECT_EQ(memory.ownership(), ::getpid());
    memory.ownership(12);
    EXPECT_EQ(memory.ownership(), 12);
}
TEST_F(SharedMemoryTest, DefaultOwnershipIsPid) {SHM_ISOLATED(DefaultOwnershipIsPidScenario(this->_name));}

static void CreateEmptyThrowsScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    try {
        server.template init<true>(name, 0, 1);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::InvalidArgument);
    }
}
TEST_F(SharedMemoryTest, CreateEmptyThrows) {SHM_ISOLATED(CreateEmptyThrowsScenario(this->_name));}

static void OpenUnknownThrowsScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory client(pid_t{1002});
    try {
        client.init(name);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::ShmOpen);
    }
}
TEST_F(SharedMemoryTest, OpenUnknownThrows) {SHM_ISOLATED(OpenUnknownThrowsScenario(this->_name));}

static void SendDefaultScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 64, 4);
    utils::encapsulation::SharedMemory client(pid_t{1002});
    client.init(name);

    shm::Target target(1, pid_t{1002});
    EXPECT_NO_THROW((void)server.send(toBytes("hello"), target)); // default: failsafe = false
    EXPECT_TRUE(waitFor([&] {return client.readable();}));
}
TEST_F(SharedMemoryTest, SendDefault) {SHM_ISOLATED(SendDefaultScenario(this->_name));}

static void SendToSpecificReaderScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 64, 4);
    utils::encapsulation::SharedMemory client(pid_t{1002});
    client.init(name);

    shm::Target target(1, pid_t{1002});
    shm::Id id;
    ASSERT_NO_THROW(id = server.send(toBytes("hello"), target, false, false, true));
    EXPECT_NE(id.id, 0u);
    EXPECT_EQ(id.ownership, 1001);

    ASSERT_TRUE(waitFor([&] {return client.readable();}));
    EXPECT_TRUE(client.readable(id));
    std::optional<std::vector<std::vector<std::byte>>> data = client.read(id);
    ASSERT_TRUE(data.has_value());
    ASSERT_EQ(data->size(), 1u);
    EXPECT_EQ(toString(data->front()), "hello");
    EXPECT_FALSE(client.readable(id));
    EXPECT_FALSE(client.read(id).has_value());
}
TEST_F(SharedMemoryTest, SendToSpecificReader) {SHM_ISOLATED(SendToSpecificReaderScenario(this->_name));}

static void SenderDoesNotReadItselfScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 64, 4);
    utils::encapsulation::SharedMemory client(pid_t{1002});
    client.init(name);

    shm::Target target(1, pid_t{1002});
    (void)server.send(toBytes("hello"), target, false, false, true);
    ASSERT_TRUE(waitFor([&] {return client.readable();}));
    EXPECT_FALSE(server.readable());
}
TEST_F(SharedMemoryTest, SenderDoesNotReadItself) {SHM_ISOLATED(SenderDoesNotReadItselfScenario(this->_name));}

static void OtherReaderIgnoreTargetedMessageScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 64, 4);
    utils::encapsulation::SharedMemory client(pid_t{1002}), other(pid_t{1003});
    client.init(name);
    other.init(name);

    shm::Target target(1, pid_t{1002});
    (void)server.send(toBytes("only for 1002"), target, false, false, true);
    ASSERT_TRUE(waitFor([&] {return client.readable();}));
    std::this_thread::sleep_for(20ms);
    EXPECT_FALSE(other.readable());
}
TEST_F(SharedMemoryTest, OtherReaderIgnoreTargetedMessage) {SHM_ISOLATED(OtherReaderIgnoreTargetedMessageScenario(this->_name));}

static void GlobalMessageScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 64, 4);
    utils::encapsulation::SharedMemory a(pid_t{1002}), b(pid_t{1003});
    a.init(name);
    b.init(name);

    shm::Target target(2); // global, read by 2 readers
    (void)server.send(toBytes("broadcast"), target, false, false, true);
    ASSERT_TRUE(waitFor([&] {return a.readable() && b.readable();}));

    std::optional<std::unordered_map<shm::Id, std::vector<std::vector<std::byte>>>> da = a.read();
    std::optional<std::unordered_map<shm::Id, std::vector<std::vector<std::byte>>>> db = b.read();
    ASSERT_TRUE(da.has_value());
    ASSERT_TRUE(db.has_value());
    EXPECT_EQ(toString(da->begin()->second.front()), "broadcast");
    EXPECT_EQ(toString(db->begin()->second.front()), "broadcast");
}
TEST_F(SharedMemoryTest, GlobalMessage) {SHM_ISOLATED(GlobalMessageScenario(this->_name));}

static void ReadAllClearStorageScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 64, 4);
    utils::encapsulation::SharedMemory client(pid_t{1002});
    client.init(name);

    shm::Target t1(1, pid_t{1002}), t2(1, pid_t{1002});
    shm::Id id1 = server.send(toBytes("one"), t1, false, false, true);
    shm::Id id2 = server.send(toBytes("two"), t2, false, false, true);
    ASSERT_TRUE(waitFor([&] {return client.readable(id1) && client.readable(id2);}));

    auto data = client.read();
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->size(), 2u);
    EXPECT_EQ(toString(data->at(id1).front()), "one");
    EXPECT_EQ(toString(data->at(id2).front()), "two");
    EXPECT_FALSE(client.readable());
    EXPECT_FALSE(client.read().has_value());
}
TEST_F(SharedMemoryTest, ReadAllClearStorage) {SHM_ISOLATED(ReadAllClearStorageScenario(this->_name));}

static void ReadFilterZeroScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 64, 4);
    utils::encapsulation::SharedMemory client(pid_t{1002});
    client.init(name);

    shm::Target t1(1, pid_t{1002}), t2(1, pid_t{1002});
    shm::Id zero = server.send(toBytes("unilateral"), t1, false, true, true); // last transmission -> id 0
    shm::Id normal = server.send(toBytes("normal"), t2, false, false, true);
    EXPECT_EQ(zero.id, 0u);
    ASSERT_TRUE(waitFor([&] {return client.readable(zero) && client.readable(normal);}));

    EXPECT_TRUE(client.readable(shm::ReadFilter::ZeroOnly));
    EXPECT_TRUE(client.readable(shm::ReadFilter::NonZeroOnly));
    auto zeros = client.read(shm::ReadFilter::ZeroOnly);
    ASSERT_TRUE(zeros.has_value());
    EXPECT_EQ(zeros->size(), 1u);
    EXPECT_TRUE(zeros->contains(zero));
    EXPECT_FALSE(client.readable(shm::ReadFilter::ZeroOnly));
    EXPECT_TRUE(client.readable(shm::ReadFilter::NonZeroOnly));
}
TEST_F(SharedMemoryTest, ReadFilterZero) {SHM_ISOLATED(ReadFilterZeroScenario(this->_name));}

static void ReadFilterLastOnlyScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 64, 4);
    utils::encapsulation::SharedMemory client(pid_t{1002});
    client.init(name);

    shm::Target t1(1, pid_t{1002}), t2(1, pid_t{1002});
    shm::Id last = server.send(toBytes("last"), t1, true, false, true);
    shm::Id other = server.send(toBytes("other"), t2, false, false, true);
    ASSERT_TRUE(waitFor([&] {return client.readable(last) && client.readable(other);}));

    auto lasts = client.read(shm::ReadFilter::LastOnly);
    ASSERT_TRUE(lasts.has_value());
    EXPECT_EQ(lasts->size(), 1u);
    EXPECT_TRUE(lasts->contains(last));
    EXPECT_TRUE(client.readable(other));
}
TEST_F(SharedMemoryTest, ReadFilterLastOnly) {SHM_ISOLATED(ReadFilterLastOnlyScenario(this->_name));}

static void JoinReturnWhenDataIsPresentScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 64, 4);
    utils::encapsulation::SharedMemory client(pid_t{1002});
    client.init(name);

    shm::Target target(1, pid_t{1002});
    shm::Id id = server.send(toBytes("join"), target, true, false, true);
    ASSERT_TRUE(waitFor([&] {return client.readable(id);}));
    client.join();
    client.join(id);
    client.join(id, true);
    SUCCEED();
}
TEST_F(SharedMemoryTest, JoinReturnWhenDataIsPresent) {SHM_ISOLATED(JoinReturnWhenDataIsPresentScenario(this->_name));}

static void OversizeScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 4, 2);
    shm::Target target(1, pid_t{1002});
    try {
        (void)server.send(toBytes("too long payload"), target, false, false, true);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::OutOfBounds);
    }
}
TEST_F(SharedMemoryTest, Oversize) {SHM_ISOLATED(OversizeScenario(this->_name));}

static void AnswerWithReceivedIdScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 64, 4);
    utils::encapsulation::SharedMemory client(pid_t{1002});
    client.init(name);

    // server -> client (request)
    shm::Target request(1, pid_t{1002});
    shm::Id id = server.send(toBytes("ping"), request, false, false, true);
    ASSERT_TRUE(waitFor([&] {return client.readable(id);}));
    (void)client.read(id);

    // client -> server (answer with the same id)
    shm::Target answer(1, pid_t{1001});
    ASSERT_NO_THROW(client.send(toBytes("pong"), id, answer, false, true, true));
    ASSERT_TRUE(waitFor([&] {return server.readable(id);}));
    auto data = server.read(id);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(toString(data->front()), "pong");
}
TEST_F(SharedMemoryTest, AnswerWithReceivedId) {SHM_ISOLATED(AnswerWithReceivedIdScenario(this->_name));}

static void AnswerWithUnknownIdThrowsScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true>(name, 64, 4);
    shm::Target target(1, pid_t{1002});
    try {
        server.send(toBytes("x"), shm::Id{99, pid_t{4242}}, target, false, false, true);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::InvalidId);
    }
}
TEST_F(SharedMemoryTest, AnswerWithUnknownIdThrows) {SHM_ISOLATED(AnswerWithUnknownIdThrowsScenario(this->_name));}

/* layout policies */
template<shm::LayoutPolicy policy>
static void layoutScenario(const std::string& name)
{
    utils::encapsulation::SharedMemory server(pid_t{1001});
    server.template init<true, policy>(name, 32, 5);
    utils::encapsulation::SharedMemory client(pid_t{1002});
    client.template init<false, policy>(name);

    std::vector<shm::Id> ids;
    std::vector<std::unique_ptr<shm::Target>> targets;
    for (int i = 0; i < 5; ++i) {
        targets.push_back(std::make_unique<shm::Target>(1, pid_t{1002}));
        ids.push_back(server.send(toBytes("msg" + std::to_string(i)), *targets.back(), false, false, true));
    }
    ASSERT_TRUE(waitFor([&] {
        for (const shm::Id& id: ids) if (!client.readable(id)) return false;
        return true;
    }));
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(toString(client.read(ids[i])->front()), "msg" + std::to_string(i));
}

TEST_F(SharedMemoryTest, LayoutCompact) {SHM_ISOLATED(layoutScenario<shm::LayoutPolicy::Compact>(this->_name));}
TEST_F(SharedMemoryTest, LayoutCompactSemiAligned) {SHM_ISOLATED(layoutScenario<shm::LayoutPolicy::CompactSemiAligned>(this->_name));}
TEST_F(SharedMemoryTest, LayoutInterleaved) {SHM_ISOLATED(layoutScenario<shm::LayoutPolicy::Interleaved>(this->_name));}
TEST_F(SharedMemoryTest, LayoutInterleavedAligned) {SHM_ISOLATED(layoutScenario<shm::LayoutPolicy::InterleavedAligned>(this->_name));}
