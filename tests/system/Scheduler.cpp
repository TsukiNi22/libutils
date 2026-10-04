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


// Wait until the condition is true or the timeout is reached
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

/* ------------------------------- Scheduler ------------------------------- */
TEST(Scheduler, ExecuteAfterDelay) {
    std::atomic<bool> done = false;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point executed;
    utils::system::Scheduler scheduler;

    (void)scheduler.schedule(std::chrono::milliseconds{50}, [&](void) {executed = std::chrono::steady_clock::now(); done = true;});
    EXPECT_FALSE(done);
    ASSERT_TRUE(waitFor([&](void) {return done.load();}));
    EXPECT_GE(executed - start, std::chrono::milliseconds{50});
}

TEST(Scheduler, DistinctIds) {
    utils::system::Scheduler scheduler;
    std::size_t a = scheduler.schedule(std::chrono::milliseconds{1000}, [](void) {});
    std::size_t b = scheduler.schedule(std::chrono::milliseconds{1000}, [](void) {});
    EXPECT_NE(a, b);
    EXPECT_NE(a, 0u);
    scheduler.cancel();
}

TEST(Scheduler, CancelOne) {
    std::atomic<bool> first = false, second = false;
    utils::system::Scheduler scheduler;
    std::size_t id = scheduler.schedule(std::chrono::milliseconds{100}, [&](void) {first = true;});
    (void)scheduler.schedule(std::chrono::milliseconds{100}, [&](void) {second = true;});

    EXPECT_NO_THROW(scheduler.cancel(id));
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
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
    for (int i = 0; i < 5; ++i) (void)scheduler.schedule(std::chrono::milliseconds{100}, [&](void) {++count;});
    scheduler.cancel();
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_EQ(count, 0);
}

TEST(Scheduler, DestructionCancelPendingTasks) {
    // Run in a sub-process: a crash during the destruction must not kill the whole test binary
    EXPECT_EXIT({
        std::atomic<bool> done = false;
        {
            utils::system::Scheduler scheduler;
            (void)scheduler.schedule(std::chrono::milliseconds{100}, [&](void) {done = true;});
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{200});
        std::exit(done ? 1 : 0);
    }, ::testing::ExitedWithCode(0), "");
}

TEST(Scheduler, ManyTasks) {
    std::atomic<int> count = 0;
    utils::system::Scheduler scheduler;
    for (int i = 0; i < 20; ++i) (void)scheduler.schedule(std::chrono::milliseconds(i), [&](void) {++count;});
    EXPECT_TRUE(waitFor([&](void) {return count == 20;}));
}

/* ----------------------------- LoadBalancer ------------------------------ */
using Balancer = utils::system::LoadBalancer<utils::type::Worker>;

TEST(LoadBalancer, Settings) {
    Balancer balancer(2, std::chrono::milliseconds{50});
    EXPECT_EQ(balancer.getLimit(), 2u);
    EXPECT_EQ(balancer.getLifespan(), std::chrono::milliseconds{50});
    balancer.setLimit(5);
    balancer.setLifespan(std::chrono::milliseconds{0});
    EXPECT_EQ(balancer.getLimit(), 5u);
    EXPECT_EQ(balancer.getLifespan(), std::chrono::milliseconds{0});
}

TEST(LoadBalancer, SpawnAndKill) {
    Balancer balancer(2);
    balancer.spawn(2);
    EXPECT_EQ(balancer.size(), 2u);
    try {
        balancer.spawn(1); // limit reached
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::OutOfBounds);
    }
    balancer.kill(1);
    EXPECT_EQ(balancer.size(), 1u);
    EXPECT_THROW(balancer.kill(5), utils::exception::IException);
    balancer.kill();
    EXPECT_EQ(balancer.size(), 0u);
}

TEST(LoadBalancer, NoLimit) {
    Balancer balancer(0); // 0 = infinite
    EXPECT_NO_THROW(balancer.spawn(50));
    EXPECT_EQ(balancer.size(), 50u);
}

TEST(LoadBalancer, GetWorker) {
    Balancer balancer(2);
    balancer.spawn(2);
    std::future<utils::type::Worker&> a = balancer.getWorker();
    std::future<utils::type::Worker&> b = balancer.getWorker();
    ASSERT_EQ(a.wait_for(std::chrono::seconds{1}), std::future_status::ready);
    ASSERT_EQ(b.wait_for(std::chrono::seconds{1}), std::future_status::ready);
    utils::type::Worker& wa = a.get();
    utils::type::Worker& wb = b.get();
    EXPECT_TRUE(wa.isWorking());
    EXPECT_TRUE(wb.isWorking());
    EXPECT_NE(&wa, &wb); // two different workers
    wa.setWorkingStatus(false);
    wb.setWorkingStatus(false);
}

TEST(LoadBalancer, GetWorkerWaitForAFreeOne) {
    Balancer balancer(1);
    balancer.spawn(1);
    utils::type::Worker& worker = balancer.getWorker().get();

    std::future<utils::type::Worker&> next = balancer.getWorker();
    EXPECT_EQ(next.wait_for(std::chrono::milliseconds{50}), std::future_status::timeout); // the only worker is working
    worker.setWorkingStatus(false);
    ASSERT_EQ(next.wait_for(std::chrono::seconds{1}), std::future_status::ready);
    EXPECT_EQ(&next.get(), &worker);
    worker.setWorkingStatus(false);
}

TEST(LoadBalancer, CanceledOnDestruction) {
    std::future<utils::type::Worker&> future;
    {
        Balancer balancer(1);
        future = balancer.getWorker(); // no worker: never found
    }
    ASSERT_EQ(future.wait_for(std::chrono::seconds{1}), std::future_status::ready);
    try {
        (void)future.get();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::PromiseCanceled);
    }
}

TEST(LoadBalancer, LifespanKillUnusedWorkers) {
    Balancer balancer(3, std::chrono::milliseconds{30});
    balancer.spawn(3);
    utils::type::Worker& busy = balancer.getWorker().get(); // stay working
    EXPECT_TRUE(waitFor([&](void) {return balancer.size() == 1;}));
    EXPECT_TRUE(busy.isWorking());
    busy.setWorkingStatus(false);
    EXPECT_TRUE(waitFor([&](void) {return balancer.size() == 0;}));
}

TEST(LoadBalancer, InfiniteLifespan) {
    Balancer balancer(2, std::chrono::milliseconds{0});
    balancer.spawn(2);
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_EQ(balancer.size(), 2u);
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
        threads.emplace_back([&, t](void) {for (int i = 0; i < 1000; ++i) ids[t].push_back(handler.allocate());});
    for (std::thread& thread: threads) thread.join();

    std::set<std::uint64_t> all;
    for (const std::vector<std::uint64_t>& v: ids) all.insert(v.begin(), v.end());
    EXPECT_EQ(all.size(), 8000u);
    EXPECT_EQ(*all.begin(), 1u);
    EXPECT_EQ(*all.rbegin(), 8000u);
}

/* -------------------------------- edge cases -------------------------------- */
TEST(IdHandlerExtra, FreeForcedIdAboveCounterNoDuplicate) {
    utils::system::IdHandler<std::uint32_t> handler;
    handler.use(5);
    handler.free(std::uint32_t{5});
    std::set<std::uint32_t> ids;
    for (int i = 0; i < 6; ++i) ids.insert(handler.allocate());
    EXPECT_EQ(ids.size(), 6u);
}

TEST(IdHandlerExtra, FreeNeverDistributed) {
    utils::system::IdHandler<std::uint32_t> handler;
    EXPECT_THROW(handler.free(std::uint32_t{100}), utils::exception::IException);
    EXPECT_THROW(handler.free(std::uint32_t{0}), utils::exception::IException);
}

TEST(IdHandlerExtra, PreviewSkipForced) {
    utils::system::IdHandler<std::uint32_t> handler;
    handler.use(1);
    EXPECT_EQ(handler.preview(), 2u);
    EXPECT_EQ(handler.allocate(), 2u);
}

TEST(IdHandlerExtra, FreeLiteral) {
    utils::system::IdHandler<std::size_t> handler;
    (void)handler.allocate();
    handler.free(1);
    EXPECT_EQ(handler.allocate(), 1u);
    handler.clear();
    EXPECT_EQ(handler.id(), 0u);
}

TEST(IdHandlerExtra, PreviewOverflowNotFatal) {
    utils::system::IdHandler<std::uint8_t> handler;
    for (int i = 0; i < 255; ++i) (void)handler.allocate();
    try {
        (void)handler.preview();
        ADD_FAILURE() << "preview should throw on overflow";
    } catch (const utils::exception::IException& e) {
        EXPECT_FALSE(e.isFatal());
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::IdOverflow);
    }
    EXPECT_DEATH((void)handler.allocate(), ".*");
}

TEST(IdHandlerDeathTest, OverflowWhileSkippingForced) {
    utils::system::IdHandler<std::uint8_t> handler;
    handler.use(255);
    for (int i = 0; i < 254; ++i) (void)handler.allocate();
    EXPECT_DEATH((void)handler.allocate(), ".*");
}

TEST(Scheduler, TaskCancelItself) {
    utils::system::Scheduler scheduler;
    std::atomic<std::size_t> id = 0;
    std::atomic<bool> done = false;
    id = scheduler.schedule(std::chrono::milliseconds{10}, [&](void) {scheduler.cancel(id); done = true;});
    EXPECT_TRUE(waitFor([&](void) {return done.load();}));
}

TEST(LoadBalancer, KillKeepWorkingWorkers) {
    Balancer balancer(2);
    balancer.spawn(2);
    utils::type::Worker& busy = balancer.getWorker().get();
    EXPECT_THROW(balancer.kill(2), utils::exception::IException); // only 1 worker not working
    balancer.kill();
    EXPECT_EQ(balancer.size(), 1u); // the working one is kept (its reference is used)
    EXPECT_TRUE(busy.isWorking());
    busy.setWorkingStatus(false);
    balancer.kill(1);
    EXPECT_EQ(balancer.size(), 0u);
}

TEST(LoadBalancer, KillForcedWorkingWorkers) {
    Balancer balancer(3);
    balancer.spawn(3);
    (void)balancer.getWorker().get();
    (void)balancer.getWorker().get();
    EXPECT_THROW(balancer.kill<true>(4), utils::exception::IException);
    balancer.kill<true>(2); // the not working one first, then a working one
    EXPECT_EQ(balancer.size(), 1u);
    balancer.kill<true>();
    EXPECT_EQ(balancer.size(), 0u);
}
