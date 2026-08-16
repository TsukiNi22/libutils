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
##  @file MemoryLeakNotifer.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils.hpp"
#include "utils/security/observer/UnsafeObserver.hpp"
#include "utils/security/observer/MemoryLeakNotifier.hpp"
#include <gtest/gtest.h>
#include <memory>

TEST(MemoryLeakNotifer, GlobalNotifiersArrayEmplacement) {
    // Check the number of instances
    ASSERT_GT(utils::security::observer::instances::Notifiers.size(), 0);

    // Check if the first pointer is the MemoryLeakNotifier
    std::unique_ptr<utils::security::observer::INotifier>& notifier = utils::security::observer::instances::Notifiers[0];
    ASSERT_NE(dynamic_cast<utils::security::observer::MemoryLeakNotifier*>(notifier.get()), nullptr);
}

TEST(MemoryLeakNotifer, MemoryLeakDetectedSimpleClass) {
    ASSERT_GT(utils::security::observer::instances::Notifiers.size(), 0) << "See test: MemoryLeakNotifer::GlobalNotifiersArrayEmplacement";
    testing::internal::CaptureStderr();

    // Generate a memory leak on observer
    {
        // Create a pointer on observed class
        void *ptr = new utils::arguments::Settings();
        (void)ptr; // cancel unused warning
    }

    // Trigger & Reset MemoryLeakNotifer instances
    std::unique_ptr<utils::security::observer::INotifier>& notifier = utils::security::observer::instances::Notifiers[0];
    notifier->trigger();
    notifier->clear(false); // Free the allocated ids

    // Check 'notification'
    std::string output = testing::internal::GetCapturedStderr();
    std::string expected =
    "[WARNING] Memory leak detected (origin: ./unit_tests)\n"
    "-- At least one instance wasn't properly closed --\n"
    "  1 - Settings\n";
    ASSERT_EQ(output, expected);
}

TEST(MemoryLeakNotifer, MemoryLeakDetectedSubClass) {
    ASSERT_GT(utils::security::observer::instances::Notifiers.size(), 0) << "See test: MemoryLeakNotifer::GlobalNotifiersArrayEmplacement";
    testing::internal::CaptureStderr();

    // Generate a memory leak on observer
    {
        // Create a pointer on observed class
        void *ptr = new utils::network::Client();
        (void)ptr; // cancel unused warning
    }

    // Trigger & Reset MemoryLeakNotifer instances
    std::unique_ptr<utils::security::observer::INotifier>& notifier = utils::security::observer::instances::Notifiers[0];
    notifier->trigger();
    notifier->clear(false); // Free the allocated ids

    // Check 'notification'
    std::string output = testing::internal::GetCapturedStderr();
    std::string expected =
    "[WARNING] Memory leak detected (origin: ./unit_tests)\n"
    "-- At least one instance wasn't properly closed --\n"
    "  2 - ISocket\n"
    "  1 - Client\n";
    ASSERT_EQ(output, expected);
}

class UnsafeObservedClass: private utils::security::observer::UnsafeObserver<"Testing"> {};

TEST(MemoryLeakNotifer, MemoryLeakDetecteUnsafeClass) {
    ASSERT_GT(utils::security::observer::instances::Notifiers.size(), 0) << "See test: MemoryLeakNotifer::GlobalNotifiersArrayEmplacement";
    testing::internal::CaptureStderr();

    // Generate a memory leak on unrestricted observer (UnsafeObserver)
    {
        // Create a pointer on observed class
        void *ptr = new UnsafeObservedClass();
        (void)ptr; // cancel unused warning
    }

    // Trigger & Reset MemoryLeakNotifer instances
    std::unique_ptr<utils::security::observer::INotifier>& notifier = utils::security::observer::instances::Notifiers[0];
    notifier->trigger();
    notifier->clear(false); // Free the allocated ids

    // Check 'notification'
    std::string output = testing::internal::GetCapturedStderr();
    std::string expected =
    "[WARNING] Memory leak detected (origin: ./unit_tests)\n"
    "-- At least one instance wasn't properly closed --\n"
    "  1 - Testing\n";
    ASSERT_EQ(output, expected);
}

class UnsafeObservedSubClass: private utils::security::observer::UnsafeObserver<"SubTesting"> {private: UnsafeObservedClass _class;};

TEST(MemoryLeakNotifer, MemoryLeakDetecteUnsafeSubClass) {
    ASSERT_GT(utils::security::observer::instances::Notifiers.size(), 0) << "See test: MemoryLeakNotifer::GlobalNotifiersArrayEmplacement";
    testing::internal::CaptureStderr();

    // Generate a memory leak on unrestricted observer (UnsafeObserver)
    {
        // Create a pointer on observed class
        void *ptr = new UnsafeObservedSubClass();
        (void)ptr; // cancel unused warning
    }

    // Trigger & Reset MemoryLeakNotifer instances
    std::unique_ptr<utils::security::observer::INotifier>& notifier = utils::security::observer::instances::Notifiers[0];
    notifier->trigger();
    notifier->clear(false); // Free the allocated ids

    // Check 'notification'
    std::string output = testing::internal::GetCapturedStderr();
    std::string expected =
    "[WARNING] Memory leak detected (origin: ./unit_tests)\n"
    "-- At least one instance wasn't properly closed --\n"
    "  2 - Testing\n"
    "  1 - SubTesting\n";
    ASSERT_EQ(output, expected);
}
