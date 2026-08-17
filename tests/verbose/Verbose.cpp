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

struct VerboseModeCase {
    std::string name;
    std::optional<utils::verbose::Verbose> mode;
    std::function<void(const std::string&)> trigger;
    std::function<std::string(const std::string&)> expected;
};

std::ostream& operator<<(std::ostream& os, const VerboseModeCase& c) {return os << c.name;};

using VerboseTestParam = std::tuple<VerboseModeCase, std::string>;
class VerboseTest : public ::testing::TestWithParam<VerboseTestParam> {};
class VerboseRedirectTest : public ::testing::TestWithParam<VerboseTestParam> {};

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
    const auto& [modeCase, input] = GetParam();

    testing::internal::CaptureStdout();
    if (modeCase.mode.has_value())
        utils::verbose::verbose = *modeCase.mode;
    modeCase.trigger(input);
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_EQ(output, modeCase.expected(input));
}

TEST_P(VerboseRedirectTest, ProducesExpectedOutputRedirect) {
    const auto& [modeCase, input] = GetParam();

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
