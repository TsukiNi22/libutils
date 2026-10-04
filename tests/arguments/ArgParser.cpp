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
##  @file ArgParser.cpp

File Description:
##  Unit tests of the ArgParser (flags, options, usages, environment, help) & its default hooks
\**************************************************************/

#include "utils.hpp"
#include "tools/TempDir.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>

using Arguments = std::vector<std::tuple<std::string, bool, std::vector<std::string>>>;

// Basic parser:
// -n / -num / --number <value>  (int32, env UTILS_TEST_NUMBER)
// -v / --verbose
// -a / --all
// usage "main": number (mandatory), verbose, all
class ArgParserTest: public ::testing::Test {
    protected:
        utils::arguments::ArgParser _parser{"bin", "A test binary"};

        void SetUp(void) override
        {
            ::unsetenv("UTILS_TEST_NUMBER");
            this->_parser.setFlag("number", {"n", "num", "number", "UTILS_TEST_NUMBER"}, {{"value", true, utils::arguments::defaultInt32ParsingHook}}, "A number");
            this->_parser.setFlag("verbose", {"v", "", "verbose", ""}, {}, "Verbose mode");
            this->_parser.setFlag("all", {"a", "", "all", ""}, {}, "All mode");
            this->_parser.setUsage("main", "main", false, {{"number", true}, {"verbose", false}, {"all", false}}, "Main usage");
        };
        void TearDown(void) override {::unsetenv("UTILS_TEST_NUMBER");};

        static bool has(const utils::arguments::ParsedUsage& usage, const std::string& id, const std::vector<std::string>& values = {})
        {
            for (const auto &[aid, option, avalues]: usage.arguments)
                if (aid == id && avalues == values) return true;
            return false;
        };

        // Expect the parsing to throw with the given code
        void expectCode(const std::vector<std::string>& argv, utils::exception::InternalCode code, const bool failsafe = false)
        {
            try {
                (void)this->_parser.parse(argv, failsafe);
                ADD_FAILURE() << "Expected an exception";
            } catch (const utils::exception::IException& e) {
                EXPECT_EQ(e.getCode(), code) << e.what() << ": " << e.info();
            }
        };
};

/* setup */
TEST_F(ArgParserTest, Getters) {
    EXPECT_EQ(this->_parser.getBinary(), "bin");
    EXPECT_EQ(this->_parser.getDescription(), "A test binary");
    EXPECT_EQ(this->_parser.getFlags().size(), 3u);
    EXPECT_EQ(this->_parser.getUsages().size(), 1u);
    EXPECT_TRUE(this->_parser.getOptions().empty());
    this->_parser.setBinary("other");
    this->_parser.setDescription("desc");
    EXPECT_EQ(this->_parser.getBinary(), "other");
    EXPECT_EQ(this->_parser.getDescription(), "desc");
}

TEST_F(ArgParserTest, OverrideProtection) {
    EXPECT_THROW(this->_parser.setFlag("verbose", {"V", "", "", ""}, {}), utils::exception::IException);
    EXPECT_THROW(this->_parser.setOption("verbose", "name"), utils::exception::IException); // same id as a flag
    EXPECT_THROW(this->_parser.setUsage("main", "main", false, {}), utils::exception::IException);
    EXPECT_NO_THROW(this->_parser.setFlag<true>("verbose", {"V", "", "", ""}, {}));
    EXPECT_EQ(std::get<0>(this->_parser.getFlags().at("verbose").flag), "V");
}

TEST_F(ArgParserTest, UnlimitedFlagWithoutOption) {
    try {
        this->_parser.setFlag("files", {"f", "", "", ""}, {}, "[None]", true);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::FlagOption);
    }
}

TEST_F(ArgParserTest, Remove) {
    this->_parser.removeFlag("all");
    EXPECT_FALSE(this->_parser.getFlags().contains("all"));
    this->_parser.removeUsages({"main"});
    EXPECT_TRUE(this->_parser.getUsages().empty());

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(this->_parser.removeFlag("unknown"));
    EXPECT_NO_THROW(this->_parser.removeOption("unknown"));
    EXPECT_NO_THROW(this->_parser.removeUsage("unknown"));
    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_NE(err.find("unknown"), std::string::npos);
}

TEST_F(ArgParserTest, Reset) {
    this->_parser.resetFlags();
    this->_parser.resetUsages();
    this->_parser.resetOptions();
    EXPECT_TRUE(this->_parser.getFlags().empty());
    EXPECT_TRUE(this->_parser.getUsages().empty());
}

/* parsing: errors */
TEST_F(ArgParserTest, EmptyArgv) {
    this->expectCode({}, utils::exception::InternalCode::ArgumentsNumber);
}

TEST_F(ArgParserTest, NoUsage) {
    this->_parser.resetUsages();
    this->expectCode({"bin"}, utils::exception::InternalCode::NoCompliantUsage);
}

TEST_F(ArgParserTest, MissingMandatoryFlag) {
    this->expectCode({"bin", "-v"}, utils::exception::InternalCode::NoCompliantUsage);
}

TEST_F(ArgParserTest, MissingFlagValue) {
    this->expectCode({"bin", "-n"}, utils::exception::InternalCode::FlagOptionsNumber);
}

TEST_F(ArgParserTest, InvalidFlagValue) {
    this->expectCode({"bin", "-n", "abc"}, utils::exception::InternalCode::FlagOption);
}

TEST_F(ArgParserTest, UnknownShortFlag) {
    this->expectCode({"bin", "-n", "1", "-z"}, utils::exception::InternalCode::UnknownFlag);
}

TEST_F(ArgParserTest, UnknownLongFlag) {
    this->expectCode({"bin", "--zzz"}, utils::exception::InternalCode::UnknownFlag);
}

TEST_F(ArgParserTest, EqualOnFlagWithoutOption) {
    this->expectCode({"bin", "-n", "1", "--verbose=1"}, utils::exception::InternalCode::FlagOption);
}

TEST_F(ArgParserTest, CombinedShortWithOption) {
    this->expectCode({"bin", "-vn", "1"}, utils::exception::InternalCode::FlagCombinaison);
}

TEST_F(ArgParserTest, FailsafeDoesNotThrowOnUnknownFlag) {
    testing::internal::CaptureStderr();
    this->expectCode({"bin", "-n", "1", "-z"}, utils::exception::InternalCode::NoCompliantUsage, true);
    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_NE(err.find("Unknown flag"), std::string::npos);
}

/* parsing: flags */
TEST_F(ArgParserTest, ShortFlag) {
    utils::arguments::ParsedUsages usages = this->_parser.parse({"bin", "-n", "42"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].id, "main");
    EXPECT_EQ(usages[0].arguments, (Arguments{{"number", false, {"42"}}}));
}

TEST_F(ArgParserTest, FlagNameAndLong) {
    EXPECT_TRUE(has(this->_parser.parse({"bin", "-num", "1"})[0], "number", {"1"}));
    EXPECT_TRUE(has(this->_parser.parse({"bin", "--number", "2"})[0], "number", {"2"}));
    EXPECT_TRUE(has(this->_parser.parse({"bin", "--number=3"})[0], "number", {"3"}));
    EXPECT_TRUE(has(this->_parser.parse({"bin", "-n=4"})[0], "number", {"4"}));
}

TEST_F(ArgParserTest, NegativeValue) {
    EXPECT_TRUE(has(this->_parser.parse({"bin", "--number=-5"})[0], "number", {"-5"}));
}

TEST_F(ArgParserTest, OptionalFlags) {
    utils::arguments::ParsedUsages usages = this->_parser.parse({"bin", "-v", "-n", "1", "--all"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].arguments.size(), 3u);
    EXPECT_TRUE(has(usages[0], "verbose"));
    EXPECT_TRUE(has(usages[0], "all"));
    EXPECT_TRUE(has(usages[0], "number", {"1"}));
}

TEST_F(ArgParserTest, CombinedShortFlags) {
    utils::arguments::ParsedUsages usages = this->_parser.parse({"bin", "-va", "-n", "1"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_TRUE(has(usages[0], "verbose"));
    EXPECT_TRUE(has(usages[0], "all"));
}

TEST_F(ArgParserTest, DuplicatedFlag) {
    testing::internal::CaptureStderr();
    utils::arguments::ParsedUsages usages = this->_parser.parse({"bin", "-v", "-n", "1", "-v"});
    std::string err = testing::internal::GetCapturedStderr();
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].arguments.size(), 2u);
    EXPECT_NE(err.find("redefined"), std::string::npos);
}

TEST_F(ArgParserTest, OptionalFlagOption) {
    this->_parser.removeUsage("main"); // only test this usage (see ArgParserUsages for the multi usage)
    this->_parser.setFlag("level", {"l", "", "level", ""}, {{"lvl", false, utils::arguments::defaultInt32ParsingHook}});
    this->_parser.setUsage("lvl", "lvl", false, {{"level", true}});
    utils::arguments::ParsedUsages usages = this->_parser.parse({"bin", "-l"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_TRUE(has(usages[0], "level", {}));
}

TEST_F(ArgParserTest, UnlimitedFlag) {
    this->_parser.removeUsage("main"); // only test this usage (see ArgParserUsages for the multi usage)
    this->_parser.setFlag("files", {"f", "", "files", ""}, {{"file", true, utils::arguments::defaultTrueParsingHook}}, "Files", true);
    this->_parser.setUsage("files", "files", false, {{"files", true}, {"verbose", false}});
    utils::arguments::ParsedUsages usages = this->_parser.parse({"bin", "-f", "a", "b", "c", "-v"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].id, "files");
    EXPECT_TRUE(has(usages[0], "files", {"a", "b", "c"}));
    EXPECT_TRUE(has(usages[0], "verbose"));
}

TEST_F(ArgParserTest, UnlimitedFlagStopOnInvalidValue) {
    this->_parser.removeUsage("main"); // only test this usage (see ArgParserUsages for the multi usage)
    this->_parser.setFlag("ints", {"i", "", "", ""}, {{"int", true, utils::arguments::defaultSizetParsingHook}}, "Ints", true);
    this->_parser.setOption("word", "word", "A word");
    this->_parser.setUsage("ints", "ints", false, {{"ints", true}, {"word", false}});
    utils::arguments::ParsedUsages usages = this->_parser.parse({"bin", "-i", "1", "2", "word"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_TRUE(has(usages[0], "ints", {"1", "2"}));
    EXPECT_TRUE(has(usages[0], "word", {"word"}));
}

/* parsing: environment */
TEST_F(ArgParserTest, EnvironmentValue) {
    tests::tools::ScopedEnv env("UTILS_TEST_NUMBER", "7");
    utils::arguments::ParsedUsages usages = this->_parser.parse({"bin"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_TRUE(has(usages[0], "number", {"7"}));
}

TEST_F(ArgParserTest, ArgumentOverEnvironment) {
    tests::tools::ScopedEnv env("UTILS_TEST_NUMBER", "7");
    utils::arguments::ParsedUsages usages = this->_parser.parse({"bin", "-n", "8"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].arguments, (Arguments{{"number", false, {"8"}}}));
}

TEST_F(ArgParserTest, InvalidEnvironmentValueIgnored) {
    tests::tools::ScopedEnv env("UTILS_TEST_NUMBER", "not a number");
    this->expectCode({"bin"}, utils::exception::InternalCode::NoCompliantUsage);
}

/* parsing: options */
class ArgParserOptionTest: public ::testing::Test {
    protected:
        utils::arguments::ArgParser _parser{"bin"};

        void SetUp(void) override
        {
            this->_parser.setOption("cmd", "run", "The run command"); // exact
            this->_parser.setOption("file", "file", [](const std::string& s) -> std::optional<std::string> {
                if (s.ends_with(".txt")) return std::nullopt;
                return "not a .txt file";
            }, "A text file");
            this->_parser.setUsage("run", "run", true, {{"cmd", true}, {"file", true}});
        };
};

TEST_F(ArgParserOptionTest, ExactAndChecked) {
    utils::arguments::ParsedUsages usages = this->_parser.parse({"bin", "run", "a.txt"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].arguments, (Arguments{{"cmd", true, {"run"}}, {"file", true, {"a.txt"}}}));
}

TEST_F(ArgParserOptionTest, WrongExactOption) {
    try {
        (void)this->_parser.parse({"bin", "walk", "a.txt"});
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::OptionIngored);
    }
}

TEST_F(ArgParserOptionTest, OrderMatters) {
    EXPECT_THROW((void)this->_parser.parse({"bin", "a.txt", "run"}), utils::exception::IException);
}

TEST_F(ArgParserOptionTest, MissingOption) {
    try {
        (void)this->_parser.parse({"bin", "run"});
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::NoCompliantUsage);
    }
}

TEST_F(ArgParserOptionTest, ArgcArgv) {
    const char* argv[] = {"bin", "run", "b.txt"};
    utils::arguments::ParsedUsages usages = this->_parser.parse(3, argv);
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].id, "run");
}

/* parsing: multiple usages */
TEST(ArgParserUsages, SelectTheCompliantUsage) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("verbose", {"v", "", "verbose", ""}, {});
    parser.setFlag("all", {"a", "", "all", ""}, {});
    parser.setUsage("A", "A", false, {{"verbose", false}});
    parser.setUsage("B", "B", false, {{"verbose", false}, {"all", false}});

    utils::arguments::ParsedUsages usages = parser.parse({"bin", "-v", "-a"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].id, "B");
}

TEST(ArgParserUsages, SortedByMatchedArguments) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("verbose", {"v", "", "verbose", ""}, {});
    parser.setFlag("all", {"a", "", "all", ""}, {});
    parser.setUsage("A", "A", false, {{"verbose", false}});
    parser.setUsage("B", "B", false, {{"verbose", false}, {"all", false}});

    utils::arguments::ParsedUsages usages = parser.parse({"bin", "-v"});
    ASSERT_EQ(usages.size(), 2u);
    EXPECT_EQ(usages[0].arguments.size(), 1u);
    EXPECT_EQ(usages[1].arguments.size(), 1u);
}

TEST(ArgParserUsages, DefaultUsageWithoutArgument) {
    utils::arguments::ArgParser parser("bin");
    parser.setDefaultUsage();
    utils::arguments::ParsedUsages usages = parser.parse({"bin"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].id, "default");
}

TEST(ArgParserUsages, DefaultUsageAllowAllFlags) {
    // "default -> allow all flag (like no usage defined)"
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("verbose", {"v", "", "verbose", ""}, {});
    parser.setDefaultUsage();
    utils::arguments::ParsedUsages usages;
    ASSERT_NO_THROW(usages = parser.parse({"bin", "-v"}));
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].arguments.size(), 1u);
}

TEST(ArgParserUsages, DefaultUsageAllowEveryFlagForm) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("a", {"a", "", "all", ""}, {});
    parser.setFlag("b", {"b", "bee", "", ""}, {});
    parser.setFlag("n", {"n", "", "number", ""}, {{"value", true, utils::arguments::defaultInt32ParsingHook}});
    parser.setFlag("o", {"o", "", "opt", ""}, {{"value", false, utils::arguments::defaultInt32ParsingHook}});
    parser.setFlag("l", {"l", "", "list", ""}, {{"value", true, utils::arguments::defaultTrueParsingHook}}, "[None]", true);
    parser.setOption("file", "file");
    parser.setDefaultUsage();
    const std::vector<std::vector<std::string>> argvs = {
        {"bin", "-a"}, {"bin", "-ab"}, {"bin", "-bee"}, {"bin", "--all"},
        {"bin", "-n", "1"}, {"bin", "--number=-2"}, {"bin", "-o"}, {"bin", "--opt", "3"},
        {"bin", "-l", "x", "y", "z"}, {"bin", "file", "-a", "-n", "1"},
        {"bin", "-n", "1", "-a", "-b", "-l", "x", "y"}
    };
    for (const std::vector<std::string>& argv: argvs) {
        utils::arguments::ParsedUsages usages;
        EXPECT_NO_THROW(usages = parser.parse(argv)) << argv[1];
        ASSERT_EQ(usages.size(), 1u) << argv[1];
        EXPECT_FALSE(usages[0].arguments.empty()) << argv[1];
    }
}

TEST(ArgParserUsages, OrderedFlags) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("first", {"f", "", "", ""}, {});
    parser.setFlag("second", {"s", "", "", ""}, {});
    parser.setUsage("ordered", "ordered", true, {{"first", true}, {"second", true}});
    EXPECT_NO_THROW((void)parser.parse({"bin", "-f", "-s"}));
    EXPECT_THROW((void)parser.parse({"bin", "-s", "-f"}), utils::exception::IException);
}

/* help */
TEST_F(ArgParserTest, HelpHookAndExit) {
    bool called = false;
    this->_parser.setHelpHook([&](const utils::arguments::ArgParser& parser) {called = true; EXPECT_EQ(parser.getBinary(), "bin");});
    for (const std::string& flag: {"-h", "-help", "--help"}) {
        called = false;
        try {
            (void)this->_parser.parse({"bin", flag});
            ADD_FAILURE() << "Expected an exit exception";
        } catch (const utils::exception::IException& e) {
            EXPECT_TRUE(e.isNone());
            EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Exit);
        }
        EXPECT_TRUE(called) << flag;
    }
}

TEST_F(ArgParserTest, HelpHookException) {
    this->_parser.setHelpHook([](const utils::arguments::ArgParser&) {throw std::runtime_error("hook failure");});
    try {
        this->_parser.help();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::ArgParserHook);
        EXPECT_STREQ(e.info(), "hook failure");
    }
}

TEST_F(ArgParserTest, HelpDisabled) {
    this->_parser.disableHelp();
    this->expectCode({"bin", "-n", "1", "-h"}, utils::exception::InternalCode::UnknownFlag);
}

TEST_F(ArgParserTest, DefaultHelpHookOutput) {
    this->_parser.setOption("file", "file", "A file option");
    testing::internal::CaptureStdout();
    this->_parser.help();
    std::string out = testing::internal::GetCapturedStdout();
    for (const std::string& s: {"PROJECT", "A test binary", "USAGE", "./bin", "OPTIONS", "A file option", "FLAGS",
                                "--help", "--number", "A number", "Verbose mode", "ENVIRONMENT", "UTILS_TEST_NUMBER"})
        EXPECT_NE(out.find(s), std::string::npos) << s;
}

/* ------------------------------ default hooks ---------------------------- */
TEST(ArgParserHooks, Bool) {
    for (const std::string& s: {"0", "1", "true", "false"})
        EXPECT_FALSE(utils::arguments::defaultBoolParsingHook(s).has_value()) << s;
    for (const std::string& s: {"", "2", "yes", "TRUEE"})
        EXPECT_TRUE(utils::arguments::defaultBoolParsingHook(s).has_value()) << s;
}

TEST(ArgParserHooks, Int32) {
    for (const std::string& s: {"0", "42", "2147483647", "-1", "-2147483648"})
        EXPECT_FALSE(utils::arguments::defaultInt32ParsingHook(s).has_value()) << s;
    for (const std::string& s: {"", "abc", "1.5", "2147483648", "-2147483649", "12a"})
        EXPECT_TRUE(utils::arguments::defaultInt32ParsingHook(s).has_value()) << s;
}

TEST(ArgParserHooks, Sizet) {
    for (const std::string& s: {"0", "18446744073709551615"})
        EXPECT_FALSE(utils::arguments::defaultSizetParsingHook(s).has_value()) << s;
    for (const std::string& s: {"", "-1", "18446744073709551616", "1e3"})
        EXPECT_TRUE(utils::arguments::defaultSizetParsingHook(s).has_value()) << s;
}

TEST(ArgParserHooks, Double) {
    for (const std::string& s: {"0", "1.5", "-2e10", ".5"})
        EXPECT_FALSE(utils::arguments::defaultDoubleParsingHook(s).has_value()) << s;
    for (const std::string& s: {"", "abc", "1.5x"})
        EXPECT_TRUE(utils::arguments::defaultDoubleParsingHook(s).has_value()) << s;
}

TEST(ArgParserHooks, True) {
    EXPECT_FALSE(utils::arguments::defaultTrueParsingHook("anything").has_value());
}

TEST(ArgParserHooks, File) {
    tests::tools::TempDir dir;
    std::ofstream(dir / "full") << "content";
    std::ofstream(dir / "empty");
    EXPECT_FALSE(utils::arguments::defaultFileParsingHook((dir / "full").string()).has_value());
    EXPECT_TRUE(utils::arguments::defaultFileParsingHook((dir / "empty").string()).has_value());
    EXPECT_TRUE(utils::arguments::defaultFileParsingHook((dir / "missing").string()).has_value());
    EXPECT_TRUE(utils::arguments::defaultFileParsingHook(dir.path().string()).has_value());
}

TEST(ArgParserHooks, Directory) {
    tests::tools::TempDir dir;
    std::ofstream(dir / "file") << "content";
    EXPECT_FALSE(utils::arguments::defaultDirectoryParsingHook(dir.path().string()).has_value());
    EXPECT_TRUE(utils::arguments::defaultDirectoryParsingHook((dir / "file").string()).has_value());
    EXPECT_TRUE(utils::arguments::defaultDirectoryParsingHook((dir / "missing").string()).has_value());
}

TEST(ArgParserHooks, Writable) {
    tests::tools::TempDir dir;
    std::ofstream(dir / "existing") << "content";
    EXPECT_TRUE(utils::arguments::defaultWritableParsingHook((dir / "existing").string()).has_value());

    EXPECT_FALSE(utils::arguments::defaultWritableParsingHook((dir / "new.txt").string()).has_value());
    EXPECT_FALSE(std::filesystem::exists(dir / "new.txt")); // the test file is removed

    EXPECT_FALSE(utils::arguments::defaultWritableParsingHook((dir / "sub/dir/").string()).has_value());
    EXPECT_FALSE(utils::arguments::defaultWritableParsingHook(dir.path().string()).has_value());
}

TEST(ArgParserHooks, WritableNoPermission) {
    if (::geteuid() == 0) GTEST_SKIP() << "root ignore permissions";
    tests::tools::TempDir dir;
    std::filesystem::create_directory(dir / "ro");
    std::filesystem::permissions(dir / "ro", std::filesystem::perms::owner_read | std::filesystem::perms::owner_exec);
    EXPECT_TRUE(utils::arguments::defaultWritableParsingHook((dir / "ro/file.txt").string()).has_value());
    std::filesystem::permissions(dir / "ro", std::filesystem::perms::owner_all);
}

/* -------------------------------- edge cases -------------------------------- */
TEST(ArgParserUsages, OptionBeforeFlagUnordered) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("F", {"f", "", "", ""}, {});
    parser.setOption("X", "x", utils::arguments::defaultTrueParsingHook);
    parser.setUsage("u", "u", false, {{"F", false}, {"X", true}});
    utils::arguments::ParsedUsages usages = parser.parse({"bin", "val", "-f"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].arguments.size(), 2u);
}

TEST(ArgParserUsages, MandatoryAfterOptionUnordered) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("F", {"f", "", "", ""}, {});
    parser.setOption("X", "x", utils::arguments::defaultTrueParsingHook);
    parser.setUsage("u", "u", false, {{"F", true}, {"X", true}});
    EXPECT_NO_THROW((void)parser.parse({"bin", "-f", "val"}));
    EXPECT_NO_THROW((void)parser.parse({"bin", "val", "-f"})); // the order doesn't matter in an unordered usage
}

TEST(ArgParserUsages, DefaultUsageOptionThenFlag) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("F", {"f", "", "", ""}, {});
    parser.setOption("X", "x", utils::arguments::defaultTrueParsingHook);
    parser.setDefaultUsage();
    utils::arguments::ParsedUsages usages = parser.parse({"bin", "val", "-f"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].arguments.size(), 2u);
}

TEST(ArgParserUsages, OrderedReversedFlags) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("A", {"a", "", "", ""}, {});
    parser.setFlag("B", {"b", "", "", ""}, {});
    parser.setUsage("u", "u", true, {{"A", false}, {"B", false}});
    try {
        (void)parser.parse({"bin", "-b", "-a"});
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::NoCompliantUsage);
    }
}

TEST(ArgParserUsages, OrderedOutOfOrderFlagInvalidateUsage) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("F", {"f", "", "", ""}, {{"v", true, utils::arguments::defaultTrueParsingHook}});
    parser.setOption("X", "x", utils::arguments::defaultTrueParsingHook);
    parser.setOption("Y", "y", utils::arguments::defaultTrueParsingHook);
    parser.setUsage("u", "u", true, {{"X", true}, {"F", false}, {"Y", false}});
    EXPECT_THROW((void)parser.parse({"bin", "-f", "fval", "xval"}), utils::exception::IException);
}

TEST(ArgParserUsages, UnlimitedFlagEmptyArgument) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("F", {"f", "", "", ""}, {{"v", true, utils::arguments::defaultTrueParsingHook}}, "", true);
    parser.setDefaultUsage();
    utils::arguments::ParsedUsages usages = parser.parse({"bin", "-f", "a", ""});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(std::get<2>(usages[0].arguments[0]), (std::vector<std::string>{"a", ""}));
}

TEST(ArgParserUsages, EmptyShortNameNeverMatch) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("a", {"a", "", "all", ""}, {});
    parser.setFlag("verb", {"", "verbose", "verbose", ""}, {});
    parser.setFlag("b", {"", "", "bee", ""}, {});
    parser.setDefaultUsage();
    utils::arguments::ParsedUsages usages = parser.parse({"bin", "-a"});
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].arguments.size(), 1u);
}

TEST(ArgParserUsages, DuplicatedFlagConsumeItsOption) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("n", {"n", "", "", ""}, {{"v", true, utils::arguments::defaultInt32ParsingHook}});
    parser.setDefaultUsage();
    testing::internal::CaptureStderr();
    utils::arguments::ParsedUsages usages;
    EXPECT_NO_THROW(usages = parser.parse({"bin", "-n", "1", "-n", "2"}));
    (void)testing::internal::GetCapturedStderr();
    ASSERT_EQ(usages.size(), 1u);
    EXPECT_EQ(usages[0].arguments, (Arguments{{"n", false, {"1"}}}));
}

TEST(ArgParserUsages, EnvironmentHookError) {
    utils::arguments::ArgParser parser("bin");
    parser.setFlag("F", {"f", "", "", "UTILS_TEST_FILE"}, {{"file", true, utils::arguments::defaultFileParsingHook}});
    parser.setUsage("u", "u", false, {{"F", true}});
    tests::tools::ScopedEnv env("UTILS_TEST_FILE", std::string(5000, 'a'));
    EXPECT_THROW((void)parser.parse({"bin"}), utils::exception::IException);
}

TEST(ArgParserHooks, LongPath) {
    EXPECT_TRUE(utils::arguments::defaultFileParsingHook(std::string(5000, 'a')).has_value());
    EXPECT_TRUE(utils::arguments::defaultDirectoryParsingHook(std::string(5000, 'a')).has_value());
}

TEST(ArgParserHooks, WritableSpecialPaths) {
    tests::tools::TempDir dir;
    EXPECT_TRUE(utils::arguments::defaultWritableParsingHook("").has_value());

    // fifo
    ASSERT_EQ(::mkfifo((dir / "fifo").c_str(), 0600), 0);
    EXPECT_TRUE(utils::arguments::defaultWritableParsingHook((dir / "fifo").string()).has_value());
    EXPECT_TRUE(std::filesystem::exists(std::filesystem::symlink_status(dir / "fifo")));

    // dangling symlink
    std::filesystem::create_symlink(dir / "target", dir / "link");
    EXPECT_TRUE(utils::arguments::defaultWritableParsingHook((dir / "link").string()).has_value());
    EXPECT_TRUE(std::filesystem::is_symlink(std::filesystem::symlink_status(dir / "link")));
    EXPECT_FALSE(std::filesystem::exists(dir / "target"));
}

TEST_F(ArgParserTest, ResetHelpHook) {
    bool called = false;
    this->_parser.setHelpHook([&called](const utils::arguments::ArgParser&) {called = true;});
    this->_parser.resetHelpHook();
    testing::internal::CaptureStdout();
    this->_parser.help();
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_FALSE(called);
    EXPECT_NE(out.find("USAGE"), std::string::npos); // defaultHelpHook
}

TEST_F(ArgParserTest, DefaultHelpHookContent) {
    this->_parser.setOption("word", "word", "A word");
    testing::internal::CaptureStdout();
    utils::arguments::defaultHelpHook(this->_parser);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(out.find("./bin"), std::string::npos);
    EXPECT_NE(out.find("--number"), std::string::npos);
    EXPECT_NE(out.find("A number"), std::string::npos);
    EXPECT_NE(out.find("A test binary"), std::string::npos);
}

/* remove */
TEST_F(ArgParserTest, RemoveFlagsAndOptions) {
    this->_parser.setOption("word", "word", "A word");
    this->_parser.setOption("other", "other", "Another");
    this->_parser.removeFlags({"verbose", "all"});
    this->_parser.removeOptions({"word", "other"});
    EXPECT_FALSE(this->_parser.getFlags().contains("verbose"));
    EXPECT_FALSE(this->_parser.getFlags().contains("all"));
    EXPECT_TRUE(this->_parser.getFlags().contains("number"));
    EXPECT_TRUE(this->_parser.getOptions().empty());

    testing::internal::CaptureStderr();
    this->_parser.removeFlags({"verbose"}); // unknown: warning only
    EXPECT_FALSE(testing::internal::GetCapturedStderr().empty());
}
