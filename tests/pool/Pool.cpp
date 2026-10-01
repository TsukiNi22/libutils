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
##  @file Pool.cpp

File Description:
##  Unit tests of the pool part (Cluster & Middlewares)
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>

/* -------------------------------- Cluster -------------------------------- */
TEST(Cluster, SpawnOne) {
    utils::pool::Cluster<std::string> cluster;
    EXPECT_EQ(cluster.size(), 0u);
    cluster.spawn("a");
    cluster.spawn(std::string("b"));
    EXPECT_EQ(cluster.size(), 2u);
}

TEST(Cluster, SpawnMany) {
    utils::pool::Cluster<std::string> cluster;
    cluster.spawn(std::size_t{3}, "x");
    EXPECT_EQ(cluster.size(), 3u);
    std::vector<std::string> values;
    cluster.apply([&](std::string& s) {values.push_back(s);});
    EXPECT_EQ(values, (std::vector<std::string>{"x", "x", "x"}));
}

TEST(Cluster, Apply) {
    utils::pool::Cluster<int> cluster;
    cluster.spawn(std::size_t{4}, 1);
    cluster.apply([](int& v) {v *= 10;});
    int sum = 0;
    cluster.apply([&](int& v) {sum += v;});
    EXPECT_EQ(sum, 40);
}

TEST(Cluster, Kill) {
    utils::pool::Cluster<std::string> cluster;
    cluster.spawn("a");
    cluster.spawn("b");
    cluster.spawn("c");
    cluster.kill(2);
    EXPECT_EQ(cluster.size(), 1u);
    std::string remaining;
    cluster.apply([&](std::string& s) {remaining = s;});
    EXPECT_EQ(remaining, "a"); // the n last are killed
    cluster.kill();
    EXPECT_EQ(cluster.size(), 0u);
}

TEST(Cluster, KillTooMany) {
    utils::pool::Cluster<int> cluster;
    cluster.spawn(1);
    try {
        cluster.kill(2);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::OutOfBounds);
    }
    EXPECT_EQ(cluster.size(), 1u);
}

TEST(Cluster, Move) {
    utils::pool::Cluster<int> a;
    a.spawn(std::size_t{2}, 5);
    utils::pool::Cluster<int> b(std::move(a));
    EXPECT_EQ(b.size(), 2u);
}

/* ------------------------------ Middlewares ------------------------------ */
TEST(Middlewares, CallOrderTT) {
    utils::pool::Middlewares<int, std::string> m;
    std::vector<std::string> calls;
    utils::pool::Middleware<int> b1 = [&](int v) {calls.push_back("b1:" + std::to_string(v));};
    utils::pool::Middleware<int> b2 = [&](int v) {calls.push_back("b2:" + std::to_string(v));};
    utils::pool::Middleware<std::string> a1 = [&](std::string s) {calls.push_back("a1:" + s);};
    m.addBefore(b1);
    m.addBefore(b2);
    m.addAfter(a1);

    m.callBefore(7);
    m.callAfter("done");
    EXPECT_EQ(calls, (std::vector<std::string>{"b1:7", "b2:7", "a1:done"}));
}

TEST(Middlewares, AddVector) {
    utils::pool::Middlewares<int, int> m;
    int sum = 0;
    m.addBefore(std::vector<utils::pool::Middleware<int>>{[&](int v) {sum += v;}, [&](int v) {sum += 2 * v;}});
    m.addAfter(std::vector<utils::pool::Middleware<int>>{[&](int v) {sum -= v;}});
    m.callBefore(1);
    EXPECT_EQ(sum, 3);
    m.callAfter(3);
    EXPECT_EQ(sum, 0);
}

TEST(Middlewares, ExceptionWrapped) {
    utils::pool::Middlewares<int, int> m;
    utils::pool::Middleware<int> fail = [](int) {throw std::runtime_error("boom");};
    m.addBefore(fail);
    try {
        m.callBefore(1);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::MiddlewareCall);
        EXPECT_STREQ(e.info(), "boom");
    }
}

TEST(Middlewares, Clear) {
    utils::pool::Middlewares<void, void> m;
    int count = 0;
    utils::pool::Middleware<void> inc = [&]() {++count;};
    m.addBefore(inc);
    m.addAfter(inc);
    m.callBefore();
    m.callAfter();
    EXPECT_EQ(count, 2);
    m.clear();
    m.callBefore();
    m.callAfter();
    EXPECT_EQ(count, 2);
}

TEST(Middlewares, VoidVariants) {
    int count = 0;
    utils::pool::Middleware<void> inc = [&]() {++count;};
    utils::pool::Middleware<int> add = [&](int v) {count += v;};

    utils::pool::Middlewares<int, void> tv;
    tv.addBefore(add);
    tv.addAfter(inc);
    tv.callBefore(10);
    tv.callAfter();
    EXPECT_EQ(count, 11);

    utils::pool::Middlewares<void, int> vt;
    vt.addBefore(inc);
    vt.addAfter(add);
    vt.callBefore();
    vt.callAfter(100);
    EXPECT_EQ(count, 112);
}

TEST(Middlewares, VoidExceptionWrapped) {
    utils::pool::Middlewares<void, void> m;
    utils::pool::Middleware<void> fail = []() {throw std::runtime_error("void boom");};
    m.addAfter(fail);
    EXPECT_NO_THROW(m.callBefore());
    EXPECT_THROW(m.callAfter(), utils::exception::IException);
}

TEST(Middlewares, CopyAndMove) {
    int count = 0;
    utils::pool::Middleware<int> add = [&](int v) {count += v;};
    utils::pool::Middlewares<int, int> a;
    a.addBefore(add);

    utils::pool::Middlewares<int, int> b(a);
    b.callBefore(1);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(a.before.size(), 1u); // the copy keep the original

    utils::pool::Middlewares<int, int> c(std::move(b));
    c.callBefore(1);
    EXPECT_EQ(count, 2);

    utils::pool::Middlewares<int, int> d;
    d = a;
    d.callBefore(5);
    EXPECT_EQ(count, 7);

    utils::pool::Middlewares<int, int> e;
    e = std::move(d);
    e.callBefore(1);
    EXPECT_EQ(count, 8);
}

TEST(Middlewares, ReferenceArguments) {
    utils::pool::Middlewares<const std::string&, const std::string&> m;
    std::string seen;
    utils::pool::Middleware<const std::string&> b = [&](const std::string& s) {seen = s;};
    m.addBefore(b);
    m.callBefore("input");
    EXPECT_EQ(seen, "input");
}
