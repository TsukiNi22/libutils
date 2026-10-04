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
##  @file Freezable.cpp

File Description:
##  Unit tests of the Freezable, Worker & BidirectionalLookupTable types
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <type_traits>
#include <chrono>
#include <string>
#include <thread>

/* ------------------------------- Freezable ------------------------------- */
TEST(Freezable, InitialState) {
    utils::type::Freezable unfrozen(false), frozen(true);
    EXPECT_FALSE(unfrozen.isFrozen());
    EXPECT_TRUE(frozen.isFrozen());
}

TEST(Freezable, Freeze) {
    utils::type::Freezable f(false);
    f.freeze();
    EXPECT_TRUE(f.isFrozen());
}

TEST(Freezable, RequireFrozen) {
    utils::type::Freezable f(false);
    try {
        f.requireFrozen();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::ShouldBeFrozen);
    }
    EXPECT_NO_THROW(f.requireUnfrozen());
    f.freeze();
    EXPECT_NO_THROW(f.requireFrozen());
}

TEST(Freezable, RequireUnfrozen) {
    utils::type::Freezable f(true);
    try {
        f.requireUnfrozen();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Frozen);
    }
}

TEST(Freezable, CopyAndMoveKeepState) {
    utils::type::Freezable a(true);
    utils::type::Freezable b(a);
    EXPECT_TRUE(b.isFrozen());
    utils::type::Freezable c(std::move(b));
    EXPECT_TRUE(c.isFrozen());
    utils::type::Freezable d(false);
    d = a;
    EXPECT_TRUE(d.isFrozen());
}

/* --------------------------------- Worker -------------------------------- */
TEST(Worker, DefaultNotWorking) {
    utils::type::Worker w;
    EXPECT_FALSE(w.isWorking());
}

TEST(Worker, StatusAndTimestamp) {
    utils::type::Worker w;
    w.setWorkingStatus(true);
    EXPECT_TRUE(w.isWorking());

    std::chrono::steady_clock::time_point before = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    w.setWorkingStatus(false);
    EXPECT_FALSE(w.isWorking());
    EXPECT_GE(w.getStopedWorkingTimestamp(), before);
}

TEST(Worker, CopyAndMove) {
    utils::type::Worker a;
    a.setWorkingStatus(true);
    utils::type::Worker b(a);
    EXPECT_TRUE(b.isWorking());
    utils::type::Worker c(std::move(a));
    EXPECT_TRUE(c.isWorking());
    EXPECT_FALSE(a.isWorking());
}

/* ------------------------ BidirectionalLookupTable ----------------------- */
using BLT = utils::type::BidirectionalLookupTable<int, std::string>;
using BLTSame = utils::type::BidirectionalLookupTable<BLT_TYPE(std::string)>;

TEST(BidirectionalLookupTable, DefaultConstructible) {
    EXPECT_TRUE(std::is_default_constructible_v<BLT>);
    EXPECT_TRUE(std::is_default_constructible_v<BLTSame>);
}

// The scenarios are templates so they can be written even if the type can't be constructed for now
template<typename T>
static void bltLeftRightScenario(void)
{
    if constexpr (std::is_default_constructible_v<T>) {
        T table;
        EXPECT_FALSE(table.isFrozen());

        // edition (before freeze)
        table.addElement(1, std::string("one"));
        table.addElement(std::string("two"), 2);
        table.setElement(3, std::string("three"));
        EXPECT_EQ(table[1], "one");
        EXPECT_EQ(table[2], "two");
        EXPECT_EQ(table[std::string("one")], 1);
        EXPECT_EQ(table[std::string("three")], 3);

        // override is disabled by default
        try {
            table.setElement(1, std::string("uno"));
            FAIL() << "Expected an exception";
        } catch (const utils::exception::IException& e) {
            EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Override);
        }
        table.template setElement<true>(1, std::string("uno"));
        EXPECT_EQ(table[1], "uno");

        // removal
        table.removeElement(2);
        EXPECT_THROW((void)table[2], utils::exception::IException);
        EXPECT_THROW((void)table[std::string("two")], utils::exception::IException);
        table.removeElement(std::string("three"));
        EXPECT_THROW((void)table[3], utils::exception::IException);

        // unknown key
        try {
            (void)table[42];
            FAIL() << "Expected an exception";
        } catch (const utils::exception::IException& e) {
            EXPECT_EQ(e.getCode(), utils::exception::InternalCode::UnknownKey);
        }

        // freeze: no more edition
        table.freeze();
        EXPECT_EQ(table[1], "uno");
        EXPECT_THROW(table.addElement(5, std::string("five")), utils::exception::IException);
        EXPECT_THROW(table.clear(), utils::exception::IException);
    } else {
        ADD_FAILURE() << "BidirectionalLookupTable<L, R> can't be default constructed (Freezable has no default constructor)";
    }
}

template<typename T>
static void bltSameTypeScenario(void)
{
    if constexpr (std::is_default_constructible_v<T>) {
        T table;
        table.addElement(std::string("up"), std::string("down"));
        EXPECT_EQ(table[std::string("up")], "down");
        EXPECT_EQ(table[std::string("down")], "up");
        EXPECT_THROW(table.addElement(std::string("up"), std::string("left")), utils::exception::IException);
        table.removeElement(std::string("up"));
        EXPECT_THROW((void)table[std::string("up")], utils::exception::IException);
        table.freeze();
        EXPECT_THROW(table.addElement(std::string("a"), std::string("b")), utils::exception::IException);
    } else {
        ADD_FAILURE() << "BidirectionalLookupTable<T, T> can't be default constructed (Freezable has no default constructor)";
    }
}

TEST(BidirectionalLookupTable, LeftRight) {
    bltLeftRightScenario<BLT>();
}

TEST(BidirectionalLookupTable, SameType) {
    bltSameTypeScenario<BLTSame>();
}

TEST(BidirectionalLookupTable, SameTypeRemoveElements) {
    BLTSame table;
    table.addElement(std::string("up"), std::string("down"));
    table.addElement(std::string("left"), std::string("right"));
    table.addElement(std::string("in"), std::string("out"));
    table.removeElements({std::string("up"), std::string("right")}); // by any side of the pair
    EXPECT_THROW((void)table[std::string("down")], utils::exception::IException);
    EXPECT_THROW((void)table[std::string("left")], utils::exception::IException);
    EXPECT_EQ(table[std::string("in")], "out");

    testing::internal::CaptureStderr();
    table.removeElements({std::string("unknown")}); // warning only
    EXPECT_FALSE(testing::internal::GetCapturedStderr().empty());
    EXPECT_EQ(table[std::string("out")], "in");
}
