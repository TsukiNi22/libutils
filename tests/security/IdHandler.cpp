/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 16/08/2026 by @author Tsukini

File Name:
##  @file IdHandler.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <limits>

template <typename T>
class IdHandlerTest : public ::testing::Test {
    protected: utils::security::IdHandler<T> handler;
};

using IdTypes = ::testing::Types<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>;
TYPED_TEST_SUITE(IdHandlerTest, IdTypes);

TYPED_TEST(IdHandlerTest, AllocationBeforeRangeLimitsNoAbort) {
    using T = TypeParam;
    constexpr std::size_t range = std::min<std::size_t>(std::numeric_limits<T>::max(), 10000);
    T id;

    for (std::size_t i = 0; i < range; ++i) this->handler.allocate(id);
    SUCCEED();
}

/* only try on */
TEST(IdHandler, AbortOnOverflow) {
    using T = std::uint8_t;
    utils::security::IdHandler<T> handler;
    T id;

    for (T i = 0; i < std::numeric_limits<T>::max(); ++i) handler.allocate(id);
    EXPECT_DEATH(handler.allocate(id), ".*");
}
