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
##  @file Verbose.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <functional>
#include <optional>
#include <string>
#include <atomic>
#include <thread>
#include <vector>

struct VerboseModeCase {
    std::string name;
    std::optional<utils::verbose::Verbose> mode;
    std::function<void(const std::string&)> trigger;
    std::function<std::string(const std::string&)> expected;
};

std::ostream& operator<<(std::ostream& os, const VerboseModeCase& c) {return os << c.name;};

using VerboseTestParam = std::tuple<VerboseModeCase, std::string>;

// Reset the global verbose mode around each test (the global state leaked between tests)
class VerboseFixture: public ::testing::TestWithParam<VerboseTestParam>
{
    protected:
        void SetUp(void) override {utils::verbose::verbose = utils::verbose::Verbose::Basic;}; // default value of the lib
        void TearDown(void) override {utils::verbose::verbose = utils::verbose::Verbose::Basic;};
};
class VerboseTest: public VerboseFixture {};
class VerboseRedirectTest: public VerboseFixture {};

const std::vector<VerboseModeCase>& verboseModeCases() {
    static const std::vector<VerboseModeCase> cases = {
        {
            "DefaultMode",
            std::nullopt,
            [](const std::string& input) {
                onBasicVerbose(input);
                onAdvancedVerbose(input);
                onDebugVerbose(input);
            },
            [](const std::string& input) {return input + "\n";}
        },
        {
            "DefaultModeFn",
            std::nullopt,
            [](const std::string& input) {
                onBasicVerboseFn(std::cout << input << std::endl;);
                onAdvancedVerboseFn(std::cout << input << std::endl;);
                onDebugVerboseFn(std::cout << input << std::endl;);
            },
            [](const std::string& input) {return input + "\n";}
        },
        {
            "NoneMode",
            utils::verbose::Verbose::None,
            [](const std::string& input) {
                onBasicVerbose(input);
                onAdvancedVerbose(input);
                onDebugVerbose(input);
            },
            [](const std::string&) {return std::string{};}
        },
        {
            "BasicMode",
            utils::verbose::Verbose::Basic,
            [](const std::string& input) {
                onBasicVerbose(input);
                onAdvancedVerbose(input);
                onDebugVerbose(input);
            },
            [](const std::string& input) {return input + "\n";}
        },
        {
            "AdvancedMode",
            utils::verbose::Verbose::Advanced,
            [](const std::string& input) {
                onAdvancedVerbose(input);
                onDebugVerbose(input);
            },
            [](const std::string& input) {return input + "\n";}
        },
        {
            "DebugMode",
            utils::verbose::Verbose::Debug,
            [](const std::string& input) {
                onDebugVerbose(input);
            },
            [](const std::string& input) {return "debug: " + input + "\n";}
        },
        {
            "NoneModeFn",
            utils::verbose::Verbose::None,
            [](const std::string& input) {
                onBasicVerboseFn(std::cout << input << std::endl;);
                onAdvancedVerboseFn(std::cout << input << std::endl;);
                onDebugVerboseFn(std::cout << input << std::endl;);
            },
            [](const std::string&) {return std::string{};}
        },
        {
            "BasicModeFn",
            utils::verbose::Verbose::Basic,
            [](const std::string& input) {
                onBasicVerboseFn(std::cout << input << std::endl;);
                onAdvancedVerboseFn(std::cout << input << std::endl;);
                onDebugVerboseFn(std::cout << input << std::endl;);
            },
            [](const std::string& input) {return input + "\n";}
        },
        {
            "AdvancedModeFn",
            utils::verbose::Verbose::Advanced,
            [](const std::string& input) {
                onAdvancedVerboseFn(std::cout << input << std::endl;);
                onDebugVerboseFn(std::cout << input << std::endl;);
            },
            [](const std::string& input) {return input + "\n";}
        },
        {
            "DebugModeFn",
            utils::verbose::Verbose::Debug,
            [](const std::string& input) {
                onDebugVerboseFn(std::cout << input << std::endl;);
            },
            [](const std::string& input) {return input + "\n";}
        },
        {
            "CustomNoneMode",
            utils::verbose::Verbose::None,
            [](const std::string& input) {
                onVerbose(utils::verbose::Verbose::None, input);
            },
            [](const std::string& input) {return input + "\n";}
        },
        {
            "CustomNoneModeFn",
            utils::verbose::Verbose::None,
            [](const std::string& input) {
                onVerboseFn(utils::verbose::Verbose::None, std::cout << input << std::endl;);
            },
            [](const std::string& input) {return input + "\n";}
        },
    };
    return cases;
}

const std::vector<VerboseModeCase>& verboseRedirectModeCases() {
    static const std::vector<VerboseModeCase> cases = {
        {
            "DefaultMode",
            std::nullopt,
            [](const std::string& input) {
                onBasicVerboseC(std::cerr, input);
                onAdvancedVerboseC(std::cerr, input);
                onDebugVerboseC(std::cerr, input);
            },
            [](const std::string& input) {return input + "\n";}
        },
        {
            "NoneMode",
            utils::verbose::Verbose::None,
            [](const std::string& input) {
                onBasicVerboseC(std::cerr, input);
                onAdvancedVerboseC(std::cerr, input);
                onDebugVerboseC(std::cerr, input);
            },
            [](const std::string&) {return std::string{};}
        },
        {
            "BasicMode",
            utils::verbose::Verbose::Basic,
            [](const std::string& input) {
                onBasicVerboseC(std::cerr, input);
                onAdvancedVerboseC(std::cerr, input);
                onDebugVerboseC(std::cerr, input);
            },
            [](const std::string& input) {return input + "\n";}
        },
        {
            "AdvancedMode",
            utils::verbose::Verbose::Advanced,
            [](const std::string& input) {
                onAdvancedVerboseC(std::cerr, input);
                onDebugVerboseC(std::cerr, input);
            },
            [](const std::string& input) {return input + "\n";}
        },
        {
            "DebugMode",
            utils::verbose::Verbose::Debug,
            [](const std::string& input) {
                onDebugVerboseC(std::cerr, input);
            },
            [](const std::string& input) {return "debug: " + input + "\n";}
        },
        {
            "CustomNoneMode",
            utils::verbose::Verbose::None,
            [](const std::string& input) {
                onVerboseC(std::cerr, utils::verbose::Verbose::None, input);
            },
            [](const std::string& input) {return input + "\n";}
        },
    };
    return cases;
}

TEST_P(VerboseTest, ProducesExpectedOutput) {
    const auto &[modeCase, input] = GetParam();

    testing::internal::CaptureStdout();
    if (modeCase.mode.has_value())
        utils::verbose::verbose = *modeCase.mode;
    modeCase.trigger(input);
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_EQ(output, modeCase.expected(input));
}

TEST_P(VerboseRedirectTest, ProducesExpectedOutputRedirect) {
    const auto &[modeCase, input] = GetParam();

    testing::internal::CaptureStderr();
    if (modeCase.mode.has_value())
        utils::verbose::verbose = *modeCase.mode;
    modeCase.trigger(input);
    std::string output = testing::internal::GetCapturedStderr();

    ASSERT_EQ(output, modeCase.expected(input));
}

INSTANTIATE_TEST_SUITE_P(InputCases, VerboseTest,
    ::testing::Combine(
        ::testing::ValuesIn(verboseModeCases()),
        ::testing::Values(
            "Testing",
            "S.O.S",
            "Please need help, fuck the unit_tests...",
            ""
        )
    ),
    [](const ::testing::TestParamInfo<VerboseTestParam>& info) {return std::get<0>(info.param).name + "_" + std::to_string(info.index);}
);

INSTANTIATE_TEST_SUITE_P(InputCases, VerboseRedirectTest,
    ::testing::Combine(
        ::testing::ValuesIn(verboseRedirectModeCases()),
        ::testing::Values(
            "Testing",
            "S.O.S",
            "Please need help, fuck the unit_tests...",
            ""
        )
    ),
    [](const ::testing::TestParamInfo<VerboseTestParam>& info) {return std::get<0>(info.param).name + "_" + std::to_string(info.index);}
);

TEST(VerboseNested, VerboseInsideFn) {
    utils::verbose::verbose = utils::verbose::Verbose::Basic;
    testing::internal::CaptureStdout();
    onBasicVerboseFn(onBasicVerbose("inner"););
    EXPECT_EQ(testing::internal::GetCapturedStdout(), "inner\n");
}

TEST(VerboseLocked, RunTheFunction) {
    int calls = 0;
    utils::verbose::locked([&calls](void) {++calls;});
    EXPECT_EQ(calls, 1);
}

TEST(VerboseLocked, SerializeConcurrentOutputs) {
    utils::verbose::verbose = utils::verbose::Verbose::Basic;
    std::atomic<int> inside = 0, maxInside = 0;
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&](void) {
            for (int i = 0; i < 200; ++i) {
                utils::verbose::locked([&](void) {
                    int now = ++inside;
                    if (now > maxInside) maxInside = now;
                    --inside;
                });
            }
        });
    }
    for (std::thread& thread: threads) thread.join();
    EXPECT_EQ(maxInside.load(), 1);
}

TEST(VerboseLevel, ConcurrentChangeAndRead) {
    std::atomic<bool> stop = false;
    std::thread writer([&](void) {
        while (!stop) {
            utils::verbose::verbose = utils::verbose::Verbose::None;
            utils::verbose::verbose = utils::verbose::Verbose::Basic;
        }
    });
    testing::internal::CaptureStdout();
    for (int i = 0; i < 1000; ++i) onDebugVerbose("never"); // level always under Debug
    stop = true;
    writer.join();
    EXPECT_EQ(testing::internal::GetCapturedStdout(), "");
    utils::verbose::verbose = utils::verbose::Verbose::Basic;
}
