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
##  @file Cli.cpp

File Description:
##  Unit tests of the Cli (commands, parser, logic, flags, input editing, history, middlewares)
\**************************************************************/

#include "utils.hpp"
#include "tools/TempDir.hpp"
#include <gtest/gtest.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <functional>
#include <optional>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using utils::cli::Flag;

// Feed the cli with a script then Ctrl+D (exit)
static std::function<bool(char&)> script(const std::string& s)
{
    std::shared_ptr<std::size_t> pos = std::make_shared<std::size_t>(0);
    return [s, pos](char& c) {
        c = (*pos < s.size()) ? s[(*pos)++] : '\x04';
        return true;
    };
}

class CliTest: public ::testing::Test {
    protected:
        tests::tools::TempDir _home;
        std::unique_ptr<tests::tools::ScopedEnv> _env;
        std::unique_ptr<utils::cli::Cli> _cli;
        std::vector<std::string> _calls; // raw/parsed commands executed

        void SetUp(void) override
        {
            this->_env = std::make_unique<tests::tools::ScopedEnv>("HOME", this->_home.path().string());
            this->makeCli();
        };
        void makeCli(void)
        {
            this->_cli = std::make_unique<utils::cli::Cli>();
            this->_cli->setGetCHook(script("")); // never read the real stdin
            this->_cli->setFlags(Flag::NO_TTY | Flag::CATCH | Flag::TRIM);
            this->_cli->setCommand("record", [this](const utils::cli::Cli&, const std::string& input) {this->_calls.push_back(input);});
            this->_cli->setCommand("a", [this](const utils::cli::Cli&, const std::string&) {this->_calls.push_back("a");});
            this->_cli->setCommand("b", [this](const utils::cli::Cli&, const std::string&) {this->_calls.push_back("b");});
            this->_cli->setCommand("fail", [this](const utils::cli::Cli&, const std::string&) {this->_calls.push_back("fail"); throw std::runtime_error("boom");});
        };
        void TearDown(void) override
        {
            this->_cli.reset();
            this->_env.reset();
        };

        // Run the cli on the given inputs, return the captured stdout
        std::string run(const std::vector<std::string>& inputs, const std::string& typed = "")
        {
            this->_cli->setGetCHook(script(typed));
            testing::internal::CaptureStdout();
            testing::internal::CaptureStderr();
            try {
                (void)this->_cli->start(inputs);
            } catch (...) {
                (void)testing::internal::GetCapturedStderr();
                (void)testing::internal::GetCapturedStdout();
                throw;
            }
            std::string err = testing::internal::GetCapturedStderr();
            return testing::internal::GetCapturedStdout() + err;
        };
};

/* start */
TEST_F(CliTest, RequireTTY) {
    this->_cli->setFlags(Flag::CATCH);
    testing::internal::CaptureStdout(); // stdout is not a tty anymore
    try {
        (void)this->_cli->start();
        (void)testing::internal::GetCapturedStdout();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        (void)testing::internal::GetCapturedStdout();
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::CliTTY);
    }
    EXPECT_FALSE(this->_cli->start(1, true).has_value()); // failsafe
}

TEST_F(CliTest, StopOnCtrlD) {
    (void)this->run({});
    EXPECT_FALSE(this->_cli->isRunning());
    EXPECT_TRUE(this->_cli->wasStopped());
    EXPECT_FALSE(this->_cli->wasKilled());
}

TEST_F(CliTest, KilledCantRestart) {
    this->_cli->kill();
    try {
        (void)this->_cli->start();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Killed);
    }
    EXPECT_TRUE(this->_cli->wasKilled());
}

TEST_F(CliTest, Interrupt) {
    this->_cli->interrupt();
    (void)this->run({"a"}); // start reset the interrupt status
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a"}));
}

/* raw commands */
TEST_F(CliTest, RawCommand) {
    (void)this->run({"record hello world"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"record hello world"}));
    EXPECT_EQ(this->_cli->getCode(), 0);
}

TEST_F(CliTest, MultipleInputs) {
    (void)this->run({"a", "b", "a"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a", "b", "a"}));
}

TEST_F(CliTest, BuiltinHelp) {
    std::string out = this->run({"help"});
    EXPECT_NE(out.find("Display commands help"), std::string::npos);
}

TEST_F(CliTest, BuiltinExit) {
    for (const std::string& command: {"exit", "quit", "bye"}) {
        this->makeCli(); // the inputs not consumed stay in the queue of the cli
        this->_calls.clear();
        (void)this->run({command, "a"});
        EXPECT_TRUE(this->_calls.empty()) << command;
        EXPECT_TRUE(this->_cli->wasStopped());
    }
}

TEST_F(CliTest, BuiltinCode) {
    std::string out = this->run({"unknown", "?"});
    EXPECT_NE(out.find("128"), std::string::npos);
    EXPECT_NE(out.find("Unknown command"), std::string::npos);
}

TEST_F(CliTest, UnknownCommand) {
    std::string out = this->run({"unknown"});
    EXPECT_EQ(this->_cli->getCode(), 128);
    EXPECT_NE(out.find("Unknown command"), std::string::npos);
}

TEST_F(CliTest, CommandNotImplemented) {
    this->_cli->setCommand("empty", std::function<void(const utils::cli::Cli&, const std::string&)>{});
    (void)this->run({"empty"});
    EXPECT_EQ(this->_cli->getCode(), 129);
}

TEST_F(CliTest, Hint) {
    this->_cli->addFlags(Flag::HINT);
    std::string out = this->run({"recrod"});
    EXPECT_NE(out.find("Did you mean 'record'?"), std::string::npos);
    EXPECT_EQ(this->_cli->getCode(), 128);
}

TEST_F(CliTest, CallbackExceptionCaught) {
    std::string out = this->run({"fail", "a"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"fail", "a"}));
    EXPECT_NE(out.find("boom"), std::string::npos);
}

TEST_F(CliTest, CallbackExceptionCode) {
    (void)this->run({"fail"});
    EXPECT_EQ(this->_cli->getCode(), 130);
}

TEST_F(CliTest, CallbackExceptionNotCaught) {
    this->_cli->subFlags(Flag::CATCH);
    try {
        (void)this->run({"fail", "a"});
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::CliExecution);
        EXPECT_NE(std::string(e.info()).find("boom"), std::string::npos);
    }
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"fail"}));
    EXPECT_FALSE(this->_cli->isRunning());
}

/* empty input */
TEST_F(CliTest, EmptyInputError) {
    std::string out = this->run({""});
    EXPECT_EQ(this->_cli->getCode(), 127);
    EXPECT_NE(out.find("Empty input"), std::string::npos);
}

TEST_F(CliTest, EmptyInputIgnored) {
    this->_cli->addFlags(Flag::EMPTY_INPUT);
    (void)this->run({"", "a"});
    EXPECT_EQ(this->_cli->getCode(), 0);
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a"}));
}

TEST_F(CliTest, TrimInput) {
    (void)this->run({"   record   spaced   "});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"record   spaced"}));
}

/* logic */
TEST_F(CliTest, LogicSequence) {
    this->_cli->addFlags(Flag::LOGIC);
    (void)this->run({"a ; b"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a", "b"}));
}

TEST_F(CliTest, LogicAnd) {
    this->_cli->addFlags(Flag::LOGIC);
    (void)this->run({"a && b"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a", "b"}));
    this->_calls.clear();
    (void)this->run({"fail && b"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"fail"}));
}

TEST_F(CliTest, LogicOr) {
    this->_cli->addFlags(Flag::LOGIC);
    (void)this->run({"fail || b"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"fail", "b"}));
    this->_calls.clear();
    (void)this->run({"a || b"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a"}));
}

TEST_F(CliTest, LogicChained) {
    this->_cli->addFlags(Flag::LOGIC);
    (void)this->run({"fail && a ; b"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"fail", "b"}));
}

TEST_F(CliTest, NoLogicKeepSeparators) {
    (void)this->run({"record a && b"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"record a && b"}));
}

/* parsed commands */
TEST_F(CliTest, ParsedCommand) {
    std::vector<std::string> received;
    this->_cli->addFlags(Flag::PARSED);
    this->_cli->setCommand("add", std::make_tuple(
        std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>([&](const utils::cli::Cli&, const std::vector<std::string>& args) {received = args;}),
        std::int16_t{2}, std::int16_t{2}
    ));
    (void)this->run({"add 1 2"});
    EXPECT_EQ(this->_cli->getCode(), 0);
    EXPECT_EQ(received, (std::vector<std::string>{"add", "1", "2"}));
}

TEST_F(CliTest, ParsedCommandArgumentsNumber) {
    this->_cli->addFlags(Flag::PARSED);
    this->_cli->setCommand("add", std::make_tuple(
        std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>([](const utils::cli::Cli&, const std::vector<std::string>&) {}),
        std::int16_t{2}, std::int16_t{2}
    ));
    (void)this->run({"add 1"});
    EXPECT_EQ(this->_cli->getCode(), 124); // not enough
    (void)this->run({"add 1 2 3"});
    EXPECT_EQ(this->_cli->getCode(), 125); // too many
}

TEST_F(CliTest, ParsedCommandUnlimitedArguments) {
    std::vector<std::string> received;
    this->_cli->addFlags(Flag::PARSED);
    this->_cli->setCommand("any", std::make_tuple(
        std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>([&](const utils::cli::Cli&, const std::vector<std::string>& args) {received = args;}),
        std::int16_t{-1}, std::int16_t{-1}
    ));
    (void)this->run({"any 1 2 3 4"});
    EXPECT_EQ(this->_cli->getCode(), 0);
    EXPECT_EQ(received.size(), 5u);
}

TEST_F(CliTest, ParsedBuiltins) {
    this->_cli->addFlags(Flag::PARSED);
    std::string out = this->run({"help"});
    EXPECT_EQ(this->_cli->getCode(), 0);
    EXPECT_NE(out.find("Display commands help"), std::string::npos);
}

TEST_F(CliTest, ParsedIgnoreRawCommands) {
    this->_cli->addFlags(Flag::PARSED);
    (void)this->run({"a"}); // 'a' is only a raw command
    EXPECT_TRUE(this->_calls.empty());
    EXPECT_EQ(this->_cli->getCode(), 128);
}

/* default parser hook */
TEST(CliParserHook, RawMode) {
    utils::cli::ParsedData data = utils::cli::defaultParserHook("cmd arg1  arg2", true, false, false);
    ASSERT_EQ(data.size(), 1u);
    EXPECT_EQ(data[0], (std::vector<std::string>{"cmd", "cmd arg1  arg2", ""}));
}

TEST(CliParserHook, LogicSplit) {
    utils::cli::ParsedData data = utils::cli::defaultParserHook("a 1 && b 2 || c ; d", true, true, false);
    ASSERT_EQ(data.size(), 4u);
    EXPECT_EQ(data[0].front(), "a");
    EXPECT_EQ(data[0].back(), "&&");
    EXPECT_EQ(data[1].front(), "b");
    EXPECT_EQ(data[1].back(), "||");
    EXPECT_EQ(data[2].back(), ";");
    EXPECT_EQ(data[3].front(), "d");
    EXPECT_EQ(data[3].back(), "");
}

TEST(CliParserHook, ParsedMode) {
    utils::cli::ParsedData data = utils::cli::defaultParserHook("cmd a\tb", true, false, true);
    ASSERT_EQ(data.size(), 1u);
    EXPECT_EQ(data[0].front(), "cmd");
    EXPECT_EQ(data[0].back(), "");
    EXPECT_NE(std::find(data[0].begin(), data[0].end(), "a"), data[0].end());
    EXPECT_NE(std::find(data[0].begin(), data[0].end(), "b"), data[0].end());
}

TEST(CliParserHook, Empty) {
    EXPECT_TRUE(utils::cli::defaultParserHook("", true, true, true).empty());
    EXPECT_TRUE(utils::cli::defaultParserHook("   ", true, true, true).empty());
}

/* custom hooks */
TEST_F(CliTest, CustomParserHook) {
    this->_cli->setParserHook([](const std::string& input, bool, bool, bool) {
        return utils::cli::ParsedData{{"record", "custom:" + input, ""}};
    });
    (void)this->run({"anything"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"custom:anything"}));
}

TEST_F(CliTest, ParserHookException) {
    this->_cli->setParserHook([](const std::string&, bool, bool, bool) -> utils::cli::ParsedData {throw std::runtime_error("parser");});
    (void)this->run({"a"});
    EXPECT_EQ(this->_cli->getCode(), 2);
}

/* typed input (getc hook) */
TEST_F(CliTest, TypedInput) {
    (void)this->run({}, "record typed\n");
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"record typed"}));
}

TEST_F(CliTest, TypedBackspace) {
    (void)this->run({}, "record ax\x7f" "b\bc\n");
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"record ac"}));
}

TEST_F(CliTest, TypedCustomDelimitor) {
    this->_cli->setInputDelimitor('|');
    EXPECT_EQ(this->_cli->getInputDelimitor(), '|');
    (void)this->run({}, "a|b|");
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a", "b"}));
}

TEST_F(CliTest, TypedArrowLeft) {
    this->_cli->addFlags(Flag::ARROW);
    (void)this->run({}, "record ac\x1b[Db\n");
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"record abc"}));
}

TEST_F(CliTest, TypedArrowLeftRight) {
    this->_cli->addFlags(Flag::ARROW);
    (void)this->run({}, "record ab\x1b[D\x1b[Cc\n");
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"record abc"}));
}

TEST_F(CliTest, TypedHistoryUp) {
    this->_cli->addFlags(Flag::HISTORY);
    (void)this->run({}, "record first\n\x1b[A\n");
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"record first", "record first"}));
}

TEST_F(CliTest, TypedAutoCompletion) {
    this->_cli->addFlags(Flag::AUTO_COMPLETION);
    (void)this->run({}, "reco\t\n");
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"record"}));
}

TEST_F(CliTest, GetCHookException) {
    this->_cli->setGetCHook([](char&) -> bool {throw std::runtime_error("getc");});
    testing::internal::CaptureStderr();
    testing::internal::CaptureStdout();
    std::thread killer([&](void) {std::this_thread::sleep_for(std::chrono::milliseconds(10)); this->_cli->kill();});
    (void)this->_cli->start();
    killer.join();
    (void)testing::internal::GetCapturedStdout();
    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(this->_cli->getCode(), 2);
    EXPECT_NE(err.find("getc"), std::string::npos);
}

/* manual & thread */
TEST_F(CliTest, ManualCalls) {
    this->_cli->addFlags(Flag::MANUAL);
    testing::internal::CaptureStdout();
    (void)this->_cli->start(std::vector<std::string>{"a", "b"}, 1);
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a"}));
    (void)this->_cli->start(1);
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a", "b"}));
    (void)testing::internal::GetCapturedStdout();
}

TEST_F(CliTest, Thread) {
    this->_cli->addFlags(Flag::THREAD);
    testing::internal::CaptureStdout();
    std::optional<std::thread> thread = this->_cli->start(std::vector<std::string>{"a", "b"});
    ASSERT_TRUE(thread.has_value());
    thread->join();
    (void)testing::internal::GetCapturedStdout();
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a", "b"}));
    EXPECT_FALSE(this->_cli->isRunning());
}

TEST_F(CliTest, DetachedThreadJoin) {
    this->_cli->addFlags(Flag::THREAD | Flag::DETACHED);
    testing::internal::CaptureStdout();
    std::optional<std::thread> thread = this->_cli->start(std::vector<std::string>{"a"});
    EXPECT_FALSE(thread.has_value());
    this->_cli->join();
    (void)testing::internal::GetCapturedStdout();
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a"}));
}

TEST_F(CliTest, StartWhileRunning) {
    this->_cli->addFlags(Flag::THREAD);
    this->_cli->setGetCHook([](char&) {return false;}); // never any input
    testing::internal::CaptureStdout();
    std::optional<std::thread> thread = this->_cli->start();
    try {
        (void)this->_cli->start();
        ADD_FAILURE() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::AlreadyRunning);
    }
    this->_cli->interrupt();
    thread->join();
    (void)testing::internal::GetCapturedStdout();
    EXPECT_TRUE(this->_cli->wasInterrupted());
}

/* commands management */
TEST_F(CliTest, CommandOverride) {
    try {
        this->_cli->setCommand("a", [](const utils::cli::Cli&, const std::string&) {});
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Override);
    }
    EXPECT_NO_THROW(this->_cli->setCommand<true>("a", [this](const utils::cli::Cli&, const std::string&) {this->_calls.push_back("new a");}));
    (void)this->run({"a"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"new a"}));
}

TEST_F(CliTest, SetCommands) {
    this->_cli->setCommands(std::unordered_map<std::string, std::function<void(const utils::cli::Cli&, const std::string&)>>{
        {"c", [this](const utils::cli::Cli&, const std::string&) {this->_calls.push_back("c");}},
        {"d", [this](const utils::cli::Cli&, const std::string&) {this->_calls.push_back("d");}},
    });
    (void)this->run({"c", "d"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"c", "d"}));
}

TEST_F(CliTest, DelCommand) {
    this->_cli->delCommand("a");
    this->_cli->delCommands({"b"});
    (void)this->run({"a", "b"});
    EXPECT_TRUE(this->_calls.empty());
    EXPECT_EQ(this->_cli->getCode(), 128);
}

TEST_F(CliTest, ClearAndResetCommands) {
    this->_cli->clearCommands();
    std::string out = this->run({"help"});
    EXPECT_EQ(out.find("Display commands help"), std::string::npos);
    this->_cli->resetCommands();
    out = this->run({"help", "a"});
    EXPECT_NE(out.find("Display commands help"), std::string::npos);
    EXPECT_TRUE(this->_calls.empty()); // custom commands removed
}

/* flags */
TEST_F(CliTest, FlagsEdition) {
    this->_cli->setFlags(Flag::TRIM);
    EXPECT_EQ(this->_cli->getFlags(), static_cast<std::uint32_t>(Flag::TRIM));
    this->_cli->addFlags(Flag::LOGIC | Flag::HINT);
    EXPECT_EQ(this->_cli->getFlags(), static_cast<std::uint32_t>(Flag::TRIM | Flag::LOGIC | Flag::HINT));
    this->_cli->subFlags(Flag::TRIM);
    EXPECT_EQ(this->_cli->getFlags(), static_cast<std::uint32_t>(Flag::LOGIC | Flag::HINT));
    this->_cli->resetFlags();
    EXPECT_EQ(this->_cli->getFlags(), utils::cli::flags::DEFAULT);
}

/* history */
TEST_F(CliTest, History) {
    (void)this->run({"a", "a", "b"});
    std::vector<std::string> history = this->_cli->getHistory();
    EXPECT_EQ(history, (std::vector<std::string>{"a", "b"})); // consecutive duplicates are merged
}

TEST_F(CliTest, PersistentHistory) {
    this->_cli->addFlags(Flag::PERSISTENT);
    (void)this->run({"a", "b"});
    this->_cli.reset(); // save on destruction
    EXPECT_TRUE(std::filesystem::exists(this->_home / HISTORY_FILE));

    utils::cli::Cli other;
    EXPECT_EQ(other.getHistory(), (std::vector<std::string>{"a", "b"}));
}

TEST_F(CliTest, NotPersistentByDefault) {
    (void)this->run({"a"});
    this->_cli.reset();
    EXPECT_FALSE(std::filesystem::exists(this->_home / HISTORY_FILE));
}

/* middlewares */
TEST_F(CliTest, Middlewares) {
    std::vector<std::string> events;
    utils::pool::Middleware<void> cliStart = [&](void) {events.push_back("cli:start");};
    utils::pool::Middleware<void> cliEnd = [&](void) {events.push_back("cli:end");};
    utils::pool::Middleware<const std::string&> cmdBefore = [&](const std::string& c) {events.push_back("cmd:" + c);};
    utils::pool::Middleware<const std::string&> parser = [&](const std::string& i) {events.push_back("parse:" + i);};
    this->_cli->cliMiddlewares.addBefore(cliStart);
    this->_cli->cliMiddlewares.addAfter(cliEnd);
    this->_cli->commandMiddlewares.addBefore(cmdBefore);
    this->_cli->parserMiddlewares.addBefore(parser);
    (void)this->run({"a"});
    EXPECT_EQ(events, (std::vector<std::string>{"cli:start", "parse:a", "cmd:a", "cli:end"}));
}

TEST_F(CliTest, ErrorMiddlewares) {
    std::vector<int> codes;
    utils::pool::Middleware<std::uint8_t> error = [&](std::uint8_t code) {codes.push_back(code);};
    this->_cli->errorMiddlewares.addBefore(error);
    (void)this->run({""});
    EXPECT_EQ(codes, (std::vector<int>{127}));
}

TEST_F(CliTest, ResetMiddlewares) {
    int count = 0;
    utils::pool::Middleware<void> inc = [&](void) {++count;};
    this->_cli->cliMiddlewares.addBefore(inc);
    this->_cli->resetMiddlewares();
    (void)this->run({"a"});
    EXPECT_EQ(count, 0);
}

/* code */
TEST(CliCode, Strcode) {
    tests::tools::TempDir home;
    tests::tools::ScopedEnv env("HOME", home.path().string());
    utils::cli::Cli cli;
    EXPECT_EQ(cli.strcode(0), "OK");
    EXPECT_EQ(cli.strcode(124), "Not enough arguments");
    EXPECT_EQ(cli.strcode(125), "Too many arguments");
    EXPECT_EQ(cli.strcode(127), "Empty input");
    EXPECT_EQ(cli.strcode(128), "Unknown command");
    EXPECT_EQ(cli.strcode(129), "Command not implemented");
    EXPECT_EQ(cli.strcode(130), "Callback exception");
    EXPECT_EQ(cli.strcode(255), "Undefined error");
    EXPECT_EQ(cli.strcode(77), "No errors are associated with this code");
}

/* -------------------------------- edge cases -------------------------------- */
TEST_F(CliTest, LogicMixed) {
    this->_cli->addFlags(Flag::LOGIC);
    (void)this->run({"fail && a || b"}); // bash: b is run
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"fail", "b"}));
    this->_calls.clear();
    (void)this->run({"a || fail && b"}); // bash: a then b
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a", "b"}));
}

TEST_F(CliTest, CommandEditingCommands) {
    this->_cli->setCommand("register", [this](const utils::cli::Cli&, const std::string&) {
        this->_cli->setCommand<true>("new", [this](const utils::cli::Cli&, const std::string&) {this->_calls.push_back("new");});
    });
    (void)this->run({"register", "new"});
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"new"}));
}

TEST_F(CliTest, HookUsingTheCli) {
    std::size_t history = 0;
    std::shared_ptr<std::size_t> pos = std::make_shared<std::size_t>(0);
    std::string typed = "a\n";
    testing::internal::CaptureStdout();
    this->_cli->setGetCHook([&, pos, typed](char& c) {
        history = this->_cli->getHistory().size();
        c = (*pos < typed.size()) ? typed[(*pos)++] : '\x04';
        return true;
    });
    (void)this->_cli->start();
    (void)testing::internal::GetCapturedStdout();
    EXPECT_EQ(this->_calls, (std::vector<std::string>{"a"}));
    EXPECT_EQ(history, 1u);
}

TEST_F(CliTest, ThreadExceptionDoesNotTerminate) {
    this->_cli->setFlags(Flag::NO_TTY | Flag::THREAD);
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    std::optional<std::thread> thread = this->_cli->start(std::vector<std::string>{"fail"});
    ASSERT_TRUE(thread.has_value());
    thread->join();
    (void)testing::internal::GetCapturedStderr();
    (void)testing::internal::GetCapturedStdout();
    EXPECT_FALSE(this->_cli->isRunning());
}

TEST_F(CliTest, PromptMiddlewareErrorCode) {
    this->_cli->addFlags(Flag::PROMPT);
    this->_cli->promptMiddlewares.addBefore([](void) {throw std::runtime_error("mw");});
    (void)this->run({"a"});
    EXPECT_EQ(this->_cli->getCode(), 3);
}

TEST_F(CliTest, CommandMiddlewareErrorCode) {
    this->_cli->commandMiddlewares.addBefore([](const std::string&) {throw std::runtime_error("mw");});
    (void)this->run({"a"});
    EXPECT_EQ(this->_cli->getCode(), 3);
}

TEST_F(CliTest, CompletionWithoutCommand) {
    this->_cli->addFlags(Flag::AUTO_COMPLETION);
    this->_cli->clearCommands();
    std::vector<std::string> parsed;
    this->_cli->parserMiddlewares.addBefore([&](const std::string& input) {parsed.push_back(input);});
    (void)this->run({}, "x\t\n");
    EXPECT_EQ(parsed, (std::vector<std::string>{"x"}));
}

TEST_F(CliTest, DestructionStopRunningThread) {
    this->_cli->setFlags(Flag::NO_TTY | Flag::THREAD | Flag::DETACHED);
    this->_cli->setGetCHook([](char&) {return false;}); // never any input
    testing::internal::CaptureStdout();
    (void)this->_cli->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    this->_cli.reset();
    (void)testing::internal::GetCapturedStdout();
    SUCCEED();
}

TEST(CliGetCHook, EndOfFile) {
    // stdin at EOF
    int fds[2];
    ASSERT_EQ(::pipe(fds), 0);
    ::close(fds[1]);
    int saved = ::dup(STDIN_FILENO);
    ::dup2(fds[0], STDIN_FILENO);
    char c = 'x';
    bool res = utils::cli::defaultGetCHook(c);
    ::dup2(saved, STDIN_FILENO);
    ::close(saved);
    ::close(fds[0]);
    EXPECT_TRUE(res);
    EXPECT_EQ(c, 0);
}

/* hooks */
// The prompt is only displayed on a tty: run the cli with stdout on a pseudo-terminal, return what was displayed
static std::string runOnPty(utils::cli::Cli& cli, const std::vector<std::string>& inputs)
{
    int master = ::posix_openpt(O_RDWR | O_NOCTTY);
    if (master == -1 || ::grantpt(master) != 0 || ::unlockpt(master) != 0) return "[pty error]";
    int slave = ::open(::ptsname(master), O_RDWR | O_NOCTTY);
    if (slave == -1) return "[pty error]";
    std::cout.flush();
    int saved = ::dup(STDOUT_FILENO);
    ::dup2(slave, STDOUT_FILENO);
    (void)cli.start(inputs);
    std::cout.flush();
    ::dup2(saved, STDOUT_FILENO);
    ::close(saved);
    ::close(slave);

    std::string out;
    char buf[256];
    ::fcntl(master, F_SETFL, O_NONBLOCK);
    for (ssize_t n = 0; (n = ::read(master, buf, sizeof(buf))) > 0;) out.append(buf, static_cast<std::size_t>(n));
    ::close(master);
    return out;
}

#define CLI_ISOLATED(...) EXPECT_EXIT({::alarm(10); __VA_ARGS__; std::exit(::testing::Test::HasFailure() ? 1 : 0);}, ::testing::ExitedWithCode(0), "")

TEST_F(CliTest, CustomPromptHook) {
    CLI_ISOLATED({
        std::vector<std::uint8_t> codes;
        this->_cli->setFlags(Flag::NO_TTY | Flag::CATCH | Flag::TRIM | Flag::PROMPT);
        this->_cli->setPromptHook([&codes](const utils::cli::Cli&, std::uint8_t code) {codes.push_back(code); std::cout << "$ " << std::flush;});
        std::string out = runOnPty(*this->_cli, {"record x"});
        EXPECT_NE(out.find("$ "), std::string::npos) << out;
        EXPECT_EQ(out.find("> "), std::string::npos) << out;
        EXPECT_FALSE(codes.empty());
    });
}

TEST_F(CliTest, ResetPromptHook) {
    CLI_ISOLATED({
        this->_cli->setFlags(Flag::NO_TTY | Flag::CATCH | Flag::TRIM | Flag::PROMPT);
        this->_cli->setPromptHook([](const utils::cli::Cli&, std::uint8_t) {std::cout << "$ " << std::flush;});
        this->_cli->resetPromptHook();
        std::string out = runOnPty(*this->_cli, {"record x"});
        EXPECT_NE(out.find("> "), std::string::npos) << out; // defaultPromptHook
        EXPECT_EQ(out.find("$ "), std::string::npos) << out;
    });
}

TEST_F(CliTest, DefaultPromptHook) {
    testing::internal::CaptureStdout();
    utils::cli::defaultPromptHook(*this->_cli, 0);
    EXPECT_EQ(testing::internal::GetCapturedStdout(), "> ");
}

TEST_F(CliTest, ResetParserHook) {
    std::size_t called = 0;
    this->_cli->setParserHook([&called](const std::string& input, bool trim, bool logic, bool parse) {
        ++called;
        return utils::cli::defaultParserHook(input, trim, logic, parse);
    });
    (void)this->run({"record a"});
    EXPECT_EQ(called, 1u);
    this->_cli->resetParserHook();
    (void)this->run({"record b"});
    EXPECT_EQ(called, 1u); // custom hook no more used
    ASSERT_EQ(this->_calls.size(), 2u);
}

TEST_F(CliTest, ResetHooks) {
    CLI_ISOLATED({
        bool customPrompt = false;
        this->_cli->setFlags(Flag::NO_TTY | Flag::CATCH | Flag::TRIM | Flag::PROMPT);
        this->_cli->setPromptHook([&customPrompt](const utils::cli::Cli&, std::uint8_t) {customPrompt = true;});
        this->_cli->resetHooks();
        this->_cli->setGetCHook(script("")); // the default one reads the real stdin
        std::string out = runOnPty(*this->_cli, {"record x"});
        EXPECT_FALSE(customPrompt);
        EXPECT_NE(out.find("> "), std::string::npos) << out;
    });
}
