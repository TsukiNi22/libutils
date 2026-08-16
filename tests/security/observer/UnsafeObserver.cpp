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
##  @file UnsafeObserver.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils.hpp"
#include "utils/security/observer/UnsafeObserver.hpp"
#include "utils/security/observer/INotifier.hpp"
#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include <string>
#include <regex>

struct UnsafeObserverLeakCase {
    std::string name;
    std::function<void()> generateLeak;
    std::string expectedOutput;
};
std::ostream& operator<<(std::ostream& os, const UnsafeObserverLeakCase& c) {return os << c.name;}

class UnsafeObserverTest : public ::testing::TestWithParam<UnsafeObserverLeakCase> {};

TEST_P(UnsafeObserverTest, DetectsLeak) {
    const UnsafeObserverLeakCase& testCase = GetParam();

    ASSERT_GT(utils::security::observer::instances::Notifiers.size(), 0) << "See test: MemoryLeakNotifer::GlobalNotifiersArrayEmplacement";
    testing::internal::CaptureStderr();

    // generate the things to observe
    testCase.generateLeak();

    // Trigger & Reset MemoryLeakNotifer instances
    std::unique_ptr<utils::security::observer::INotifier>& notifier = utils::security::observer::instances::Notifiers[0];
    notifier->trigger();
    notifier->clear(true);

    std::string output = testing::internal::GetCapturedStderr();
    static const std::regex originRegex(R"(\(origin: [^)]*\))");
    ASSERT_EQ(std::regex_replace(output, originRegex, "(origin: X)"), testCase.expectedOutput);
}

class SimpleClass: private utils::security::observer::UnsafeObserver<"SimpleClass"> {};
class SubClass: private utils::security::observer::UnsafeObserver<"SubClass">
{
    private:
        SimpleClass _class1;
        SimpleClass _class2;
};

INSTANTIATE_TEST_SUITE_P(LeakCases, UnsafeObserverTest,
    ::testing::Values(
        UnsafeObserverLeakCase{
            "SimpleClass",
            [] {
                void* ptr = new SimpleClass();
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  1 - SimpleClass\n"
        },
        UnsafeObserverLeakCase{
            "SimpleClassAttribution",
            [] {
                SimpleClass* ptr = new SimpleClass();
                SimpleClass simp;
                *ptr = simp;
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  1 - SimpleClass\n"
        },
        UnsafeObserverLeakCase{
            "SimpleClassMove",
            [] {
                SimpleClass* ptr = new SimpleClass();
                SimpleClass simp;
                *ptr = std::move(simp);
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  2 - SimpleClass\n"
        },
        UnsafeObserverLeakCase{
            "SimpleClassCreateFrom",
            [] {
                SimpleClass simp;
                SimpleClass* ptr = new SimpleClass(simp);
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  2 - SimpleClass\n"
        },
        UnsafeObserverLeakCase{
            "SimpleClassCreateMove",
            [] {
                SimpleClass simp;
                SimpleClass* ptr = new SimpleClass(std::move(simp));
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  1 - SimpleClass\n"
        },
        UnsafeObserverLeakCase{
            "SubClass",
            [] {
                void* ptr = new SubClass();
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  3 - SimpleClass\n"
            "  2 - SimpleClass\n"
            "  1 - SubClass\n"
        },
        UnsafeObserverLeakCase{
            "SubClassAttribution",
            [] {
                SubClass* ptr = new SubClass();
                SubClass sub;
                *ptr = sub;
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  3 - SimpleClass\n"
            "  2 - SimpleClass\n"
            "  1 - SubClass\n"
        },
        UnsafeObserverLeakCase{
            "SubClassMove",
            [] {
                SubClass* ptr = new SubClass();
                SubClass sub;
                *ptr = std::move(sub);
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  6 - SimpleClass\n"
            "  5 - SimpleClass\n"
            "  4 - SubClass\n"
        },
        UnsafeObserverLeakCase{
            "SubClassCreateFrom",
            [] {
                SubClass sub;
                SubClass* ptr = new SubClass(sub);
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  6 - SimpleClass\n"
            "  5 - SimpleClass\n"
            "  4 - SubClass\n"
        },
        UnsafeObserverLeakCase{
            "SubClassCreateMove",
            [] {
                SubClass sub;
                SubClass* ptr = new SubClass(std::move(sub));
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  3 - SimpleClass\n"
            "  2 - SimpleClass\n"
            "  1 - SubClass\n"
        }
    ),
    [](const ::testing::TestParamInfo<UnsafeObserverLeakCase>& info) {return info.param.name;}
);
