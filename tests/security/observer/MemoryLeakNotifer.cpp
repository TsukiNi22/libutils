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
##  @file MemoryLeasNotifier.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils.hpp"
#include "utils/security/observer/UnsafeObserver.hpp"
#include "utils/security/observer/MemoryLeakNotifier.hpp"
#include "utils/security/observer/INotifier.hpp"
#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include <string>
#include <regex>

TEST(MemoryLeakNotifer, GlobalNotifiersArrayEmplacement) {
    // Check the number of instances
    ASSERT_GT(utils::security::observer::instances::Notifiers.size(), 0);

    // Check if the first pointer is the MemoryLeakNotifier
    std::unique_ptr<utils::security::observer::INotifier>& notifier = utils::security::observer::instances::Notifiers[0];
    ASSERT_NE(dynamic_cast<utils::security::observer::MemoryLeakNotifier*>(notifier.get()), nullptr);
}

struct MemoryLeasNotifierLeakCase {
    std::string name;
    std::function<void()> generateLeak;
    std::string expectedOutput;
};
std::ostream& operator<<(std::ostream& os, const MemoryLeasNotifierLeakCase& c) {return os << c.name;}

class MemoryLeasNotifierTest : public ::testing::TestWithParam<MemoryLeasNotifierLeakCase> {};

TEST_P(MemoryLeasNotifierTest, DetectsLeak) {
    const MemoryLeasNotifierLeakCase& testCase = GetParam();

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
class SubClass: private utils::security::observer::Observer<"SubClass">
{
    private:
        SimpleClass _class1;
        SimpleClass _class2;
};

INSTANTIATE_TEST_SUITE_P(LeakCases, MemoryLeasNotifierTest,
    ::testing::Values(
        MemoryLeasNotifierLeakCase{
            "SimpleClass",
            [] {
                void* ptr = new utils::arguments::Settings();
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  1 - Settings\n"
        },
        MemoryLeasNotifierLeakCase{
            "SubClass",
            [] {
                void* ptr = new utils::network::Client();
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  2 - ISocket\n"
            "  1 - Client\n"
        },
        MemoryLeasNotifierLeakCase{
            "UnsafeSimpleClass",
            [] {
                void* ptr = new SimpleClass();
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  1 - SimpleClass\n"
        },
        MemoryLeasNotifierLeakCase{
            "UnsafeSubClass",
            [] {
                void* ptr = new SubClass();
                (void)ptr;
            },
            "[WARNING] Memory leak detected (origin: X)\n"
            "-- At least one instance wasn't properly closed --\n"
            "  3 - SimpleClass\n"
            "  2 - SimpleClass\n"
            "  1 - SubClass\n"
        }
    ),
    [](const ::testing::TestParamInfo<MemoryLeasNotifierLeakCase>& info) {return info.param.name;}
);
