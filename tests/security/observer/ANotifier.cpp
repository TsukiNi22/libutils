/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 04/10/2026 by @author Tsukini

File Name:
##  @file ANotifier.cpp

File Description:
##  Unit tests of the ANotifier (link, unlink, clear & the overload hooks)
\**************************************************************/

#include "utils.hpp"
#include "utils/security/observer/ANotifier.hpp"
#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <vector>

// Notifier recording the calls of its overload hooks
class RecordNotifier: public utils::security::observer::ANotifier {
    private:
        bool _overload = false;

        void link_(const std::uint64_t id, std::string_view instance, _unused const bool safe_mode) override {this->calls.push_back("link " + std::to_string(id) + " " + std::string(instance));};
        void unlink_(const std::uint64_t id, _unused const bool safe_mode) override                          {this->calls.push_back("unlink " + std::to_string(id));};
        void clear_(_unused const bool safe_mode) override                                                   {this->calls.push_back("clear");};
        _nodiscard bool hasLinkOverload(void) const override   {return this->_overload;};
        _nodiscard bool hasUnlinkOverload(void) const override {return this->_overload;};
        _nodiscard bool hasClearOverload(void) const override  {return this->_overload;};

    public:
        std::vector<std::string> calls;

        void trigger(void) override {};
        std::size_t size(void) {return this->_links.size();};

        explicit RecordNotifier(const bool overload = false): _overload{overload} {};
};

TEST(ANotifier, LinkUnlink) {
    RecordNotifier notifier;
    notifier.link(1, "A", true);
    notifier.link(2, "B", true);
    EXPECT_EQ(notifier.size(), 2u);
    notifier.unlink(1, true);
    EXPECT_EQ(notifier.size(), 1u);
    EXPECT_TRUE(notifier.calls.empty()); // no overload
}

TEST(ANotifier, LinkZeroThrows) {
    RecordNotifier notifier;
    try {
        notifier.link(0, "A", true);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::InvalidId);
    }
    EXPECT_THROW(notifier.unlink(0, true), utils::exception::IException);
}

TEST(ANotifier, RelinkWarn) {
    RecordNotifier notifier;
    notifier.link(3, "A", true);
    testing::internal::CaptureStderr();
    notifier.link(3, "B", true);
    EXPECT_FALSE(testing::internal::GetCapturedStderr().empty());
    EXPECT_EQ(notifier.size(), 1u);
}

TEST(ANotifier, UnlinkUnknownWarn) {
    RecordNotifier notifier;
    testing::internal::CaptureStderr();
    notifier.unlink(42, false);
    EXPECT_FALSE(testing::internal::GetCapturedStderr().empty());
}

TEST(ANotifier, ClearFreeTheIds) {
    utils::system::IdHandler<std::uint64_t>& handler = utils::security::observer::instances::id_handler();
    RecordNotifier notifier;
    std::uint64_t a = handler.allocate(), b = handler.allocate();
    notifier.link(a, "A", true);
    notifier.link(b, "B", true);
    notifier.clear(true);
    EXPECT_EQ(notifier.size(), 0u);
    EXPECT_THROW(handler.free(a), utils::exception::IException); // already freed by clear
}

TEST(ANotifier, OverloadHooksCalled) {
    utils::system::IdHandler<std::uint64_t>& handler = utils::security::observer::instances::id_handler();
    RecordNotifier notifier(true);
    std::uint64_t id = handler.allocate();
    notifier.link(id, "X", true);
    notifier.unlink(id, true);
    notifier.clear(false);
    EXPECT_EQ(notifier.calls, (std::vector<std::string>{"link " + std::to_string(id) + " X", "unlink " + std::to_string(id), "clear"}));
    handler.free(id);
}
