/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 26/08/2026 by @author Tsukini

File Name:
##  @file IdHandler.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils.hpp"
#include "tools/StepSynchronizer.hpp"
#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <limits>

template <typename T>
class IdHandlerTest : public ::testing::Test {
    protected: utils::system::IdHandler<T> handler;
};

using IdTypes = ::testing::Types<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>;
TYPED_TEST_SUITE(IdHandlerTest, IdTypes);

TYPED_TEST(IdHandlerTest, AllocationBeforeRangeLimitsNoAbort) {
    using T = TypeParam;
    constexpr T range = std::min<std::size_t>(std::numeric_limits<T>::max(), 1000);
    T id;

    for (T i = 0; i < range; ++i) {
        this->handler.allocate(id);
        EXPECT_EQ(id, i + 1);
    }
    for (T i = 0; i < range; ++i) {
        id = i + 1;
        this->handler.free(id);
        EXPECT_EQ(id, 0);
    }
}

TEST(IdHandler, ReallocationAfterFree) {
    using T = std::uint8_t;
    utils::system::IdHandler<T> handler;
    T id;

    // Allocate id 1 & 2, free 1 and 1 should be the next given followed by 3
    for (T i = 1; i <= 2; ++i) {
        handler.allocate(id);
        EXPECT_EQ(id, i);
    }

    // Free id 1
    id = 1;
    handler.free(id);
    EXPECT_EQ(id, 0);

    // Should allocate 1
    handler.allocate(id);
    EXPECT_EQ(id, 1);

    // Should allocate 3
    handler.allocate(id);
    EXPECT_EQ(id, 3);
}

TEST(IdHandler, ReallocationAfterFreeMultiThread) {
    using T = std::uint8_t;
    utils::system::IdHandler<T> handler;
    tests::tools::StepSynchronizer sync;

    /* Order:
     * step 0 -> 1: Thread A allocate (wanted: 1)
     * step 1 -> 2: Thread B allocate (wanted: 2)
     * step 2 -> 3: Thread A free(1)
     * step 3 -> 4: Thread B allocate (wanted: 1, reused)
     * step 4 -> 5: Thread A allocate (wanted: 3)
    */

    std::thread threadA([&] {
        T id{};

        sync.waitForStep(0);
        handler.allocate(id);
        EXPECT_EQ(id, 1);
        sync.advanceTo(1);

        sync.waitForStep(2);
        id = 1;
        handler.free(id);
        EXPECT_EQ(id, 0);
        sync.advanceTo(3);

        sync.waitForStep(4);
        handler.allocate(id);
        EXPECT_EQ(id, 3);
        sync.advanceTo(5);
    });

    std::thread threadB([&] {
        T id{};

        sync.waitForStep(1);
        handler.allocate(id);
        EXPECT_EQ(id, 2);
        sync.advanceTo(2);

        sync.waitForStep(3);
        handler.allocate(id);
        EXPECT_EQ(id, 1);
        sync.advanceTo(4);
    });

    // Start the sequence
    sync.advanceTo(0);

    // Wait for the end
    threadA.join();
    threadB.join();
}

/* only try on uint8 due to other type high limits */
TEST(IdHandler, AbortOnOverflow) {
    using T = std::uint8_t;
    utils::system::IdHandler<T> handler;
    T id;

    for (T i = 0; i < std::numeric_limits<T>::max(); ++i) handler.allocate(id);
    EXPECT_DEATH(handler.allocate(id), ".*");
}
