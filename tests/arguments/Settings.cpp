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
##  @file Settings.cpp

File Description:
##  Unit tests of the Settings & Setting (storage, typed access, automatic cast)
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <chrono>
#include <string_view>
#include <string>
#include <vector>

using utils::arguments::CastType;

/* -------------------------------- Setting -------------------------------- */
TEST(Setting, GetAndIs) {
    utils::arguments::Setting setting(42);
    EXPECT_TRUE(setting.is<int>());
    EXPECT_FALSE(setting.is<long>());
    EXPECT_EQ(setting.get<int>(), 42);
}

TEST(Setting, ImplicitConversion) {
    utils::arguments::Setting setting(7);
    int value = setting;
    EXPECT_EQ(value, 7);
}

TEST(Setting, BadCast) {
    utils::arguments::Setting setting(42);
    try {
        (void)setting.get<std::string>();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::BadCast);
        EXPECT_NE(std::string(e.info()).find("int"), std::string::npos);
    }
    EXPECT_THROW((void)static_cast<double>(setting), utils::exception::IException);
}

TEST(Setting, EditInPlace) {
    utils::arguments::Setting setting(std::vector<int>{1, 2});
    setting.get<std::vector<int>>().push_back(3);
    EXPECT_EQ(setting.get<std::vector<int>>().size(), 3u);
}

TEST(Setting, AssignChangeType) {
    utils::arguments::Setting setting(1);
    setting.assign(std::string("now a string"));
    EXPECT_TRUE(setting.is<std::string>());
    EXPECT_EQ(setting.get<std::string>(), "now a string");
}

TEST(Setting, Move) {
    utils::arguments::Setting a(3.5);
    utils::arguments::Setting b(std::move(a));
    EXPECT_DOUBLE_EQ(b.get<double>(), 3.5);
}

/* ------------------------------- Settings -------------------------------- */
TEST(Settings, AddAndGet) {
    utils::arguments::Settings settings;
    settings.add("port", 8080);
    settings.add(std::string("name"), std::string("server"));
    EXPECT_TRUE(settings.contains("port"));
    EXPECT_TRUE(settings.contains(std::string_view("name")));
    EXPECT_FALSE(settings.contains("missing"));
    EXPECT_EQ(settings.get<int>("port"), 8080);
    EXPECT_EQ(settings.get<std::string>("name"), "server");
}

TEST(Settings, AddExistingThrows) {
    utils::arguments::Settings settings;
    settings.add("port", 1);
    try {
        settings.add("port", 2);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Override);
    }
    EXPECT_EQ(settings.get<int>("port"), 1);
}

TEST(Settings, SetOverride) {
    utils::arguments::Settings settings;
    settings.set("port", 1);
    settings.set("port", 2);
    EXPECT_EQ(settings.get<int>("port"), 2);
    settings.set("port", std::string("now a string"));
    EXPECT_EQ(settings.get<std::string>("port"), "now a string");
}

TEST(Settings, SetWithoutForce) {
    utils::arguments::Settings settings;
    settings.set<false>("port", 1);
    EXPECT_THROW(settings.set<false>("port", 2), utils::exception::IException);
}

TEST(Settings, UnknownId) {
    utils::arguments::Settings settings;
    try {
        (void)settings.at("missing");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::UnknownId);
        EXPECT_STREQ(e.info(), "missing");
    }
    EXPECT_THROW((void)settings.get<int>("missing"), utils::exception::IException);
    EXPECT_THROW((void)settings["missing"], utils::exception::IException);
}

TEST(Settings, WrongType) {
    utils::arguments::Settings settings;
    settings.add("port", 8080);
    try {
        (void)settings.get<std::string>("port");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::BadCast);
    }
}

TEST(Settings, SubscriptAccess) {
    utils::arguments::Settings settings;
    settings.add("ratio", 0.5);
    double ratio = settings["ratio"];
    EXPECT_DOUBLE_EQ(ratio, 0.5);
    const utils::arguments::Settings& constSettings = settings;
    EXPECT_TRUE(constSettings["ratio"].is<double>());
    EXPECT_TRUE(constSettings.get("ratio").is<double>());
}

TEST(Settings, EditInPlace) {
    utils::arguments::Settings settings;
    settings.add("count", 1);
    settings.get<int>("count") = 5;
    EXPECT_EQ(settings.get<int>("count"), 5);
    settings["count"].get<int>() += 1;
    EXPECT_EQ(settings.get<int>("count"), 6);
}

TEST(Settings, Remove) {
    utils::arguments::Settings settings;
    settings.add("a", 1);
    settings.remove("a");
    EXPECT_FALSE(settings.contains("a"));
    try {
        settings.remove("a");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::UnknownId);
    }
    EXPECT_NO_THROW(settings.remove<true>("a")); // failsafe
}

TEST(Settings, Clear) {
    utils::arguments::Settings settings;
    settings.add("a", 1);
    settings.add("b", 2);
    settings.clear();
    EXPECT_FALSE(settings.contains("a"));
    EXPECT_FALSE(settings.contains("b"));
}

TEST(Settings, Move) {
    utils::arguments::Settings a;
    a.add("a", 1);
    utils::arguments::Settings b(std::move(a));
    EXPECT_EQ(b.get<int>("a"), 1);
}

/* explicit cast */
TEST(SettingsCast, Integers) {
    utils::arguments::Settings settings;
    settings.cast<CastType::Int8>("i8", "-100");
    settings.cast<CastType::Int16>("i16", "-30000");
    settings.cast<CastType::Int32>("i32", "-2000000000");
    settings.cast<CastType::Int64>("i64", "-9000000000");
    settings.cast<CastType::UInt8>("u8", "255");
    settings.cast<CastType::UInt16>("u16", "65535");
    settings.cast<CastType::UInt32>("u32", "4000000000");
    settings.cast<CastType::UInt64>("u64", "18000000000000000000");
    EXPECT_EQ(settings.get<std::int8_t>("i8"), -100);
    EXPECT_EQ(settings.get<std::int16_t>("i16"), -30000);
    EXPECT_EQ(settings.get<std::int32_t>("i32"), -2000000000);
    EXPECT_EQ(settings.get<std::int64_t>("i64"), -9000000000ll);
    EXPECT_EQ(settings.get<std::uint8_t>("u8"), 255);
    EXPECT_EQ(settings.get<std::uint16_t>("u16"), 65535);
    EXPECT_EQ(settings.get<std::uint32_t>("u32"), 4000000000u);
    EXPECT_EQ(settings.get<std::uint64_t>("u64"), 18000000000000000000ull);
}

TEST(SettingsCast, IntegerOverflow) {
    utils::arguments::Settings settings;
    EXPECT_THROW(settings.cast<CastType::Int8>("a", "200"), utils::exception::IException);
    EXPECT_THROW(settings.cast<CastType::Int16>("a", "40000"), utils::exception::IException);
    EXPECT_THROW(settings.cast<CastType::Int32>("a", "3000000000"), utils::exception::IException);
    EXPECT_THROW(settings.cast<CastType::UInt8>("a", "256"), utils::exception::IException);
    EXPECT_THROW(settings.cast<CastType::UInt16>("a", "70000"), utils::exception::IException);
    EXPECT_THROW(settings.cast<CastType::UInt32>("a", "5000000000"), utils::exception::IException);
    EXPECT_THROW(settings.cast<CastType::UInt8>("a", "-1"), utils::exception::IException);
    EXPECT_THROW(settings.cast<CastType::Int32>("a", "12abc"), utils::exception::IException);
    EXPECT_THROW(settings.cast<CastType::Int32>("a", ""), utils::exception::IException);
    EXPECT_FALSE(settings.contains("a"));
}

TEST(SettingsCast, ByteAndBool) {
    utils::arguments::Settings settings;
    settings.cast<CastType::Byte>("byte", "200");
    settings.cast<CastType::Bool>("t", "TRUE");
    settings.cast<CastType::Bool>("f", "0");
    EXPECT_EQ(settings.get<std::byte>("byte"), std::byte{200});
    EXPECT_TRUE(settings.get<bool>("t"));
    EXPECT_FALSE(settings.get<bool>("f"));
    EXPECT_THROW(settings.cast<CastType::Byte>("x", "256"), utils::exception::IException);
    EXPECT_THROW(settings.cast<CastType::Bool>("x", "maybe"), utils::exception::IException);
}

TEST(SettingsCast, Floating) {
    utils::arguments::Settings settings;
    settings.cast<CastType::Float32>("f32", "1.5");
    settings.cast<CastType::Float64>("f64", "-2.25e10");
    settings.cast<CastType::Float128>("f128", "1e400");
    EXPECT_FLOAT_EQ(static_cast<float>(settings.get<utils::arguments::float32_t>("f32")), 1.5f);
    EXPECT_DOUBLE_EQ(static_cast<double>(settings.get<utils::arguments::float64_t>("f64")), -2.25e10);
    EXPECT_GT(settings.get<utils::arguments::float128_t>("f128"), static_cast<utils::arguments::float128_t>(1e300));
    EXPECT_THROW(settings.cast<CastType::Float64>("x", "1.5x"), utils::exception::IException);
}

TEST(SettingsCast, Chars) {
    utils::arguments::Settings settings;
    settings.cast<CastType::Char8>("c8", "a");
    settings.cast<CastType::Char16>("c16", "0x263A");
    settings.cast<CastType::Char32>("c32", "128512");
    settings.cast<CastType::WChar>("wc", "\xC3\xA9"); // é
    EXPECT_EQ(settings.get<char8_t>("c8"), u8'a');
    EXPECT_EQ(settings.get<char16_t>("c16"), u'☺');
    EXPECT_EQ(settings.get<char32_t>("c32"), U'\U0001F600');
    EXPECT_EQ(settings.get<wchar_t>("wc"), L'é');
    EXPECT_THROW(settings.cast<CastType::Char8>("x", "ab"), utils::exception::IException);
    EXPECT_THROW(settings.cast<CastType::Char32>("x", "0x110000"), utils::exception::IException);
}

TEST(SettingsCast, Strings) {
    utils::arguments::Settings settings;
    settings.cast<CastType::U8String>("u8", "h\xC3\xA9");
    settings.cast<CastType::U16String>("u16", "\xF0\x9F\x98\x80"); // 😀 (surrogate pair)
    settings.cast<CastType::U32String>("u32", "a\xC3\xA9");
    settings.cast<CastType::WString>("ws", "ok");
    EXPECT_EQ(settings.get<std::u8string>("u8"), u8"hé");
    EXPECT_EQ(settings.get<std::u16string>("u16"), u"\U0001F600");
    EXPECT_EQ(settings.get<std::u32string>("u32"), U"aé");
    EXPECT_EQ(settings.get<std::wstring>("ws"), L"ok");
    EXPECT_THROW(settings.cast<CastType::U8String>("x", "\xC3"), utils::exception::IException); // truncated
    EXPECT_THROW(settings.cast<CastType::U32String>("x", "\xFF"), utils::exception::IException); // invalid
}

TEST(SettingsCast, Path) {
    utils::arguments::Settings settings;
    settings.cast<CastType::Path>("p", "/tmp/file.txt");
    EXPECT_EQ(settings.get<std::filesystem::path>("p"), std::filesystem::path("/tmp/file.txt"));
    EXPECT_THROW(settings.cast<CastType::Path>("x", ""), utils::exception::IException);
}

/* automatic cast */
struct AutoCastCase {
    std::string input;
    CastType expected;
};
std::ostream& operator<<(std::ostream& os, const AutoCastCase& c) {return os << '"' << c.input << '"';}

class SettingsAutoCast: public ::testing::TestWithParam<AutoCastCase> {};

TEST_P(SettingsAutoCast, DetectedType) {
    utils::arguments::Settings settings;
    EXPECT_EQ(settings.autoCast("id", GetParam().input), GetParam().expected);
    EXPECT_TRUE(settings.contains("id"));
}

INSTANTIATE_TEST_SUITE_P(Cases, SettingsAutoCast,
    ::testing::Values(
        AutoCastCase{"true", CastType::Bool},
        AutoCastCase{"False", CastType::Bool},
        AutoCastCase{"t", CastType::Bool},
        AutoCastCase{"42", CastType::UInt32},
        AutoCastCase{"+42", CastType::UInt32},
        AutoCastCase{"5000000000", CastType::UInt64},
        AutoCastCase{"-42", CastType::Int32},
        AutoCastCase{"-5000000000", CastType::Int64},
        AutoCastCase{"1.5", CastType::Float64},
        AutoCastCase{"-.5", CastType::Float64},
        AutoCastCase{"3e8", CastType::Float64},
        AutoCastCase{"1e400", CastType::Float128},
        AutoCastCase{"./file", CastType::Path},
        AutoCastCase{"/usr/bin", CastType::Path},
        AutoCastCase{"~/conf", CastType::Path},
        AutoCastCase{"dir/file", CastType::Path},
        AutoCastCase{"x", CastType::Char8},
        AutoCastCase{"\xC3\xA9", CastType::Char32},
        AutoCastCase{"hello", CastType::None},
        AutoCastCase{"", CastType::None}
    )
);

TEST(SettingsAutoCastValue, StoredValues) {
    utils::arguments::Settings settings;
    settings.autoCast("b", "true");
    settings.autoCast("u", "42");
    settings.autoCast("i", "-42");
    settings.autoCast("f", "1.5");
    settings.autoCast("p", "./file");
    settings.autoCast("s", "hello");
    EXPECT_TRUE(settings.get<bool>("b"));
    EXPECT_EQ(settings.get<std::uint32_t>("u"), 42u);
    EXPECT_EQ(settings.get<std::int32_t>("i"), -42);
    EXPECT_DOUBLE_EQ(static_cast<double>(settings.get<utils::arguments::float64_t>("f")), 1.5);
    EXPECT_EQ(settings.get<std::filesystem::path>("p"), std::filesystem::path("./file"));
    EXPECT_EQ(settings.get<std::string>("s"), "hello");
}

TEST(SettingsAutoCastValue, NoOverrideByDefault) {
    utils::arguments::Settings settings;
    settings.autoCast("a", "1");
    EXPECT_THROW(settings.autoCast("a", "2"), utils::exception::IException);
    settings.autoCast<true>("a", "2");
    EXPECT_EQ(settings.get<std::uint32_t>("a"), 2u);
}

/* -------------------------------- edge cases -------------------------------- */
TEST(SettingsAutoCastValue, ExtremeFloats) {
    utils::arguments::Settings settings;
    EXPECT_EQ(settings.autoCast("a", "1e-400"), CastType::Float128);
    EXPECT_EQ(settings.autoCast("b", "1e-310"), CastType::Float128); // denormal
    EXPECT_EQ(settings.autoCast("c", "1e99999"), CastType::None); // not representable: kept as a string
    EXPECT_EQ(settings.get<std::string>("c"), "1e99999");
}

TEST(SettingsAutoCastValue, LongString) {
    utils::arguments::Settings settings;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    EXPECT_EQ(settings.autoCast("long", std::string(100000, 'a')), CastType::None);
    EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::seconds(5));
    EXPECT_EQ(settings.autoCast("p", std::string(50000, 'a') + "/b"), CastType::Path);
}

/* -------------------------------- forced cast -------------------------------- */
TEST(SettingsCastForce, OverrideEveryType) {
    utils::arguments::Settings settings;
    settings.add("v", std::string("old"));
    settings.cast<CastType::Byte, true>("v", "7");
    EXPECT_EQ(settings.get<std::byte>("v"), std::byte{7});
    settings.cast<CastType::Bool, true>("v", "t");
    EXPECT_TRUE(settings.get<bool>("v"));
    settings.cast<CastType::Int8, true>("v", "-8");
    EXPECT_EQ(settings.get<std::int8_t>("v"), -8);
    settings.cast<CastType::Int16, true>("v", "-16");
    EXPECT_EQ(settings.get<std::int16_t>("v"), -16);
    settings.cast<CastType::Int32, true>("v", "-32");
    EXPECT_EQ(settings.get<std::int32_t>("v"), -32);
    settings.cast<CastType::Int64, true>("v", "-64");
    EXPECT_EQ(settings.get<std::int64_t>("v"), -64);
    settings.cast<CastType::UInt8, true>("v", "+8");
    EXPECT_EQ(settings.get<std::uint8_t>("v"), 8u);
    settings.cast<CastType::UInt16, true>("v", "16");
    EXPECT_EQ(settings.get<std::uint16_t>("v"), 16u);
    settings.cast<CastType::UInt32, true>("v", "32");
    EXPECT_EQ(settings.get<std::uint32_t>("v"), 32u);
    settings.cast<CastType::UInt64, true>("v", "64");
    EXPECT_EQ(settings.get<std::uint64_t>("v"), 64u);
    settings.cast<CastType::Float32, true>("v", "0.5");
    EXPECT_FLOAT_EQ(static_cast<float>(settings.get<utils::arguments::float32_t>("v")), 0.5f);
    settings.cast<CastType::Float64, true>("v", "0.25");
    EXPECT_DOUBLE_EQ(static_cast<double>(settings.get<utils::arguments::float64_t>("v")), 0.25);
    settings.cast<CastType::Float128, true>("v", "2");
    EXPECT_EQ(settings.get<utils::arguments::float128_t>("v"), static_cast<utils::arguments::float128_t>(2));
    settings.cast<CastType::Char8, true>("v", "z");
    EXPECT_EQ(settings.get<char8_t>("v"), u8'z');
    settings.cast<CastType::Char16, true>("v", "65");
    EXPECT_EQ(settings.get<char16_t>("v"), u'A');
    settings.cast<CastType::Char32, true>("v", "66");
    EXPECT_EQ(settings.get<char32_t>("v"), U'B');
    settings.cast<CastType::U8String, true>("v", "u8");
    EXPECT_EQ(settings.get<std::u8string>("v"), u8"u8");
    settings.cast<CastType::U16String, true>("v", "u16");
    EXPECT_EQ(settings.get<std::u16string>("v"), u"u16");
    settings.cast<CastType::U32String, true>("v", "u32");
    EXPECT_EQ(settings.get<std::u32string>("v"), U"u32");
    settings.cast<CastType::WChar, true>("v", "w");
    EXPECT_EQ(settings.get<wchar_t>("v"), L'w');
    settings.cast<CastType::WString, true>("v", "ws");
    EXPECT_EQ(settings.get<std::wstring>("v"), L"ws");
    settings.cast<CastType::Path, true>("v", "/p");
    EXPECT_EQ(settings.get<std::filesystem::path>("v"), std::filesystem::path("/p"));
}

TEST(SettingsCastForce, NoForceKeepExisting) {
    utils::arguments::Settings settings;
    settings.add("v", 1);
    try {
        settings.cast<CastType::Int32>("v", "2");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Override);
    }
    EXPECT_EQ(settings.get<int>("v"), 1);
}

TEST(SettingsAutoCastForce, OverrideDetectedTypes) {
    utils::arguments::Settings settings;
    settings.add("v", 0);
    EXPECT_EQ(settings.autoCast<true>("v", "text"), CastType::None);
    EXPECT_EQ(settings.get<std::string>("v"), "text");
    EXPECT_EQ(settings.autoCast<true>("v", "true"), CastType::Bool);
    EXPECT_EQ(settings.autoCast<true>("v", "-5"), CastType::Int32);
    EXPECT_EQ(settings.autoCast<true>("v", "-9000000000"), CastType::Int64);
    EXPECT_EQ(settings.autoCast<true>("v", "5000000000"), CastType::UInt64);
    EXPECT_EQ(settings.autoCast<true>("v", "1.5"), CastType::Float64);
    EXPECT_EQ(settings.autoCast<true>("v", "./file"), CastType::Path);
    EXPECT_EQ(settings.get<std::filesystem::path>("v"), std::filesystem::path("./file"));
}

/* ------------------------------ non-ASCII input ------------------------------ */
TEST(SettingsCast, NonAsciiDigitsRejected) {
    utils::arguments::Settings settings;
    for (const std::string& input: std::vector<std::string>{"\xC3\xA9", "1\xC3\xA9", "\xEF\xBC\x91"}) { // é, 1é, fullwidth 1
        try {
            settings.cast<CastType::UInt16>("x", input);
            FAIL() << "Expected an exception for " << input;
        } catch (const utils::exception::IException& e) {
            EXPECT_EQ(e.getCode(), utils::exception::InternalCode::BadCast);
        }
        EXPECT_THROW(settings.cast<CastType::Byte>("x", input), utils::exception::IException);
        EXPECT_THROW(settings.cast<CastType::Bool>("x", input), utils::exception::IException);
    }
    EXPECT_FALSE(settings.contains("x"));
}
