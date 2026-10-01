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
##  @file Scheduler.cpp

File Description:
##  Unit tests of the Scheduler, the LoadBalancer & more IdHandler cases
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <atomic>
#include <future>
#include <vector>
#include <set>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

// Wait until the condition is true or the timeout is reached
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

/* ------------------------------- Scheduler ------------------------------- */
TEST(Scheduler, ExecuteAfterDelay) {
    std::atomic<bool> done = false;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point executed;
    utils::system::Scheduler scheduler;

    (void)scheduler.schedule(50ms, [&] {executed = std::chrono::steady_clock::now(); done = true;});
    EXPECT_FALSE(done);
    ASSERT_TRUE(waitFor([&] {return done.load();}));
    EXPECT_GE(executed - start, 50ms);
}

TEST(Scheduler, DistinctIds) {
    utils::system::Scheduler scheduler;
    std::size_t a = scheduler.schedule(1000ms, [] {});
    std::size_t b = scheduler.schedule(1000ms, [] {});
    EXPECT_NE(a, b);
    EXPECT_NE(a, 0u);
    scheduler.cancel();
}

TEST(Scheduler, CancelOne) {
    std::atomic<bool> first = false, second = false;
    utils::system::Scheduler scheduler;
    std::size_t id = scheduler.schedule(100ms, [&] {first = true;});
    (void)scheduler.schedule(100ms, [&] {second = true;});

    EXPECT_NO_THROW(scheduler.cancel(id));
    std::this_thread::sleep_for(200ms);
    EXPECT_FALSE(first);
    EXPECT_TRUE(second); // only the given task is canceled
}

TEST(Scheduler, CancelUnknown) {
    utils::system::Scheduler scheduler;
    try {
        scheduler.cancel(42);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::UnknownId);
    }
}

TEST(Scheduler, CancelAll) {
    std::atomic<int> count = 0;
    utils::system::Scheduler scheduler;
    for (int i = 0; i < 5; ++i) (void)scheduler.schedule(100ms, [&] {++count;});
    scheduler.cancel();
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(count, 0);
}

TEST(Scheduler, DestructionCancelPendingTasks) {
    // Run in a sub-process: a crash during the destruction must not kill the whole test binary
    EXPECT_EXIT({
        std::atomic<bool> done = false;
        {
            utils::system::Scheduler scheduler;
            (void)scheduler.schedule(100ms, [&] {done = true;});
        }
        std::this_thread::sleep_for(200ms);
        std::exit(done ? 1 : 0);
    }, ::testing::ExitedWithCode(0), "");
}

TEST(Scheduler, ManyTasks) {
    std::atomic<int> count = 0;
    utils::system::Scheduler scheduler;
    for (int i = 0; i < 20; ++i) (void)scheduler.schedule(std::chrono::milliseconds(i), [&] {++count;});
    EXPECT_TRUE(waitFor([&] {return count == 20;}));
}

/* ----------------------------- LoadBalancer ------------------------------ */
// The scenario is a template, it's only instantiated once the class can be instantiated
template<typename LB>
[[maybe_unused]] static void loadBalancerScenario(void)
{
    LB balancer(2, 50ms);
    EXPECT_EQ(balancer.getLimit(), 2u);
    EXPECT_EQ(balancer.getLifespan(), 50ms);

    balancer.spawn(2);
    EXPECT_THROW(balancer.spawn(1), utils::exception::IException); // limit reached

    auto future = balancer.getWorker();
    ASSERT_EQ(future.wait_for(1s), std::future_status::ready);
    EXPECT_TRUE(future.get().isWorking());

    balancer.kill(1);
    EXPECT_THROW(balancer.kill(5), utils::exception::IException);
}

TEST(LoadBalancer, Instantiation) {
    // utils::system::LoadBalancer<utils::type::Worker> can't be instantiated for now:
    // - findWorker return a std::optional<T&> (ill-formed)
    // - findWorker call worker.setStatus(true) that doesn't exist on utils::type::Worker (setWorkingStatus)
    // - LoadBalancer() & LoadBalancer(limit = 1, lifespan = 0) are ambiguous default constructors
    // Once fixed, replace this failure by: loadBalancerScenario<utils::system::LoadBalancer<utils::type::Worker>>();
    ADD_FAILURE() << "utils::system::LoadBalancer<T> can't be instantiated (see the test comment)";
}

/* ------------------------------- IdHandler ------------------------------- */
TEST(IdHandlerExtra, PreviewAndActual) {
    utils::system::IdHandler<std::uint32_t> handler;
    EXPECT_EQ(handler.preview(), 1u);
    EXPECT_EQ(handler.allocate(), 1u);
    EXPECT_EQ(handler.id(), 1u);
    EXPECT_EQ(handler.actual(), 1u);
    EXPECT_EQ(handler.preview(), 2u);
    handler.free(std::uint32_t{1});
    EXPECT_EQ(handler.preview(), 1u);
    EXPECT_EQ(handler.actual(), 1u);
}

TEST(IdHandlerExtra, DoubleFree) {
    utils::system::IdHandler<std::uint32_t> handler;
    std::uint32_t id = handler.allocate();
    handler.free(std::uint32_t{id});
    try {
        handler.free(std::uint32_t{id});
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::DoubleFree);
    }
}

TEST(IdHandlerExtra, ForcedUse) {
    utils::system::IdHandler<std::uint32_t> handler;
    handler.use(3);
    EXPECT_EQ(handler.allocate(), 1u);
    EXPECT_EQ(handler.allocate(), 2u);
    EXPECT_EQ(handler.allocate(), 4u); // 3 is already used
}

TEST(IdHandlerExtra, ForcedUseTwice) {
    utils::system::IdHandler<std::uint32_t> handler;
    handler.use(3);
    try {
        handler.use(3);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::DoubleUse);
    }
}

TEST(IdHandlerExtra, ForcedUseAlreadyAllocated) {
    utils::system::IdHandler<std::uint32_t> handler;
    (void)handler.allocate();
    (void)handler.allocate();
    EXPECT_THROW(handler.use(2), utils::exception::IException);
}

TEST(IdHandlerExtra, ForcedUseThenFreeIsReusable) {
    utils::system::IdHandler<std::uint32_t> handler;
    handler.use(5);
    handler.free(std::uint32_t{5});
    EXPECT_NO_THROW(handler.use(5)); // 5 was freed, it can be used again
}

TEST(IdHandlerExtra, FreeAll) {
    utils::system::IdHandler<std::uint32_t> handler;
    for (int i = 0; i < 10; ++i) (void)handler.allocate();
    handler.free();
    EXPECT_EQ(handler.id(), 0u);
    EXPECT_EQ(handler.allocate(), 1u);
}

TEST(IdHandlerExtra, ConcurrentAllocationAreUnique) {
    utils::system::IdHandler<std::uint64_t> handler;
    std::vector<std::vector<std::uint64_t>> ids(8);
    std::vector<std::thread> threads;
    for (std::size_t t = 0; t < ids.size(); ++t)
        threads.emplace_back([&, t] {for (int i = 0; i < 1000; ++i) ids[t].push_back(handler.allocate());});
    for (std::thread& thread: threads) thread.join();

    std::set<std::uint64_t> all;
    for (const auto& v: ids) all.insert(v.begin(), v.end());
    EXPECT_EQ(all.size(), 8000u);
    EXPECT_EQ(*all.begin(), 1u);
    EXPECT_EQ(*all.rbegin(), 8000u);
}
