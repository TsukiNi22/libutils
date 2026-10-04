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
##  @file Sos.cpp

File Description:
##  Unit tests of the s.o.s steganography algorithm & its tools
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <cstdint>
#include <random>
#include <string>
#include <vector>
#include <cmath>

using Bytes = utils::algorithms::sos::Bytes;

// Generate an audio like carrier (sine + noise) on 16 bits
static Bytes makeCarrier(std::size_t size, std::uint32_t seed = 42)
{
    std::mt19937 gen(seed);
    std::normal_distribution<double> noise(0.0, 800.0);
    Bytes carrier(size);
    for (std::size_t i = 0; i < size; ++i) {
        double value = 32768.0 + 18000.0 * std::sin(static_cast<double>(i) * 0.01) + noise(gen);
        carrier[i] = static_cast<std::uint16_t>(std::clamp(value, 0.0, 65535.0));
    }
    return carrier;
}

/* tools */
TEST(SosConvert, StringRoundTrip) {
    const std::string s = "Hello, steganography!";
    Bytes bytes = utils::algorithms::sos::tools::to_bytes(s);
    EXPECT_EQ(bytes.size(), (s.size() + 1) / 2);
    std::string back = utils::algorithms::sos::tools::bytes_to<std::string>(bytes);
    EXPECT_EQ(back.substr(0, s.size()), s);
}

TEST(SosConvert, IntVectorRoundTrip) {
    const std::vector<std::int32_t> values = {1, -2, 300000, -400000, 0};
    Bytes bytes = utils::algorithms::sos::tools::to_bytes(values);
    EXPECT_EQ(bytes.size(), values.size() * sizeof(std::int32_t) / sizeof(std::uint16_t));
    EXPECT_EQ(utils::algorithms::sos::tools::bytes_to<std::vector<std::int32_t>>(bytes), values);
}

TEST(SosConvert, InvalidByteCount) {
    Bytes bytes = {1, 2, 3}; // 6 bytes, not a multiple of 4
    EXPECT_THROW((void)utils::algorithms::sos::tools::bytes_to<std::vector<std::int32_t>>(bytes), std::invalid_argument);
}

TEST(SosConvert, Uint8ByteType) {
    const std::string s = "abc";
    std::vector<std::uint8_t> bytes = utils::algorithms::sos::tools::to_bytes<std::uint8_t>(s);
    EXPECT_EQ(bytes, (std::vector<std::uint8_t>{'a', 'b', 'c'}));
}

TEST(SosThreshold, Index) {
    Bytes bytes = {0, 255, 256, 30000, 65280, 65281, 65535};
    std::vector<std::uint_fast32_t> index;
    utils::algorithms::sos::tools::get_threshold_index(index, bytes);
    EXPECT_EQ(index, (std::vector<std::uint_fast32_t>{1, 2, 3, 4}));
}

TEST(SosThreshold, RemoveThresholdTooFewRange) {
    Bytes bytes(10000, 30000);
    EXPECT_THROW(utils::algorithms::sos::tools::remove_threshold(bytes), std::out_of_range);
}

TEST(SosThreshold, RemoveThresholdMovesBoundaries) {
    Bytes bytes = makeCarrier(100000);
    bytes[0] = THRESHOLD_MIN(std::uint16_t);
    bytes[1] = THRESHOLD_MAX(std::uint16_t);
    utils::algorithms::sos::tools::remove_threshold(bytes);
    EXPECT_NE(bytes[0], THRESHOLD_MIN(std::uint16_t));
    EXPECT_NE(bytes[1], THRESHOLD_MAX(std::uint16_t));
}

TEST(SosNoise, RmsTooSmall) {
    Bytes bytes(1000, 1);
    EXPECT_THROW(utils::algorithms::sos::tools::noise(bytes), std::out_of_range);
}

TEST(SosNoise, ChangesValuesSlightly) {
    Bytes carrier = makeCarrier(10000);
    Bytes noisy = carrier;
    utils::algorithms::sos::tools::noise(noisy);
    std::size_t changed = 0;
    for (std::size_t i = 0; i < carrier.size(); ++i) {
        changed += (carrier[i] != noisy[i]);
        EXPECT_LT(std::abs(static_cast<int>(carrier[i]) - static_cast<int>(noisy[i])), 2000);
    }
    EXPECT_GT(changed, carrier.size() / 2);
}

TEST(SosHash, Deterministic) {
    Bytes carrier = makeCarrier(5000);
    std::vector<std::uint_fast32_t> index;
    utils::algorithms::sos::tools::get_threshold_index(index, carrier);
    EXPECT_EQ(utils::algorithms::sos::tools::hash(index, carrier), utils::algorithms::sos::tools::hash(index, carrier));
}

/* algorithm */
TEST(Sos, EmbedExtract) {
    Bytes carrier = makeCarrier(200000);
    Bytes payload = utils::algorithms::sos::tools::to_bytes(std::string("secret message"));
    ASSERT_NO_THROW(utils::algorithms::sos::sos_embed(carrier, payload));
    EXPECT_EQ(utils::algorithms::sos::sos_extract(carrier), payload);
}

TEST(Sos, EmbedExtractWithKey) {
    Bytes carrier = makeCarrier(200000);
    Bytes payload = utils::algorithms::sos::tools::to_bytes(std::string("keyed message"));
    Bytes key = utils::algorithms::sos::tools::to_bytes(std::string("my-key"));
    ASSERT_NO_THROW(utils::algorithms::sos::sos_embed(carrier, payload, key));
    EXPECT_EQ(utils::algorithms::sos::sos_extract(carrier, key), payload);
}

TEST(Sos, WrongKeyDoesNotRevealPayload) {
    Bytes carrier = makeCarrier(200000);
    Bytes payload = utils::algorithms::sos::tools::to_bytes(std::string("keyed message"));
    Bytes key = utils::algorithms::sos::tools::to_bytes(std::string("my-key"));
    Bytes wrong = utils::algorithms::sos::tools::to_bytes(std::string("other"));
    utils::algorithms::sos::sos_embed(carrier, payload, key);
    try {
        EXPECT_NE(utils::algorithms::sos::sos_extract(carrier, wrong), payload);
    } catch (const std::exception&) {
        SUCCEED(); // invalid magic/size is the expected result most of the time
    }
}

TEST(Sos, ExtractWithoutMessage) {
    Bytes carrier = makeCarrier(200000);
    utils::algorithms::sos::tools::remove_threshold(carrier);
    EXPECT_THROW((void)utils::algorithms::sos::sos_extract(carrier), std::exception);
}

TEST(Sos, CarrierStaysClose) {
    Bytes carrier = makeCarrier(200000);
    Bytes original = carrier;
    utils::algorithms::sos::sos_embed(carrier, utils::algorithms::sos::tools::to_bytes(std::string("x")));
    for (std::size_t i = 0; i < carrier.size(); ++i)
        ASSERT_LE(std::abs(static_cast<int>(carrier[i]) - static_cast<int>(original[i])), 2) << i;
}

TEST(Sos, EmbedWithNoise) {
    Bytes carrier = makeCarrier(200000);
    Bytes payload = utils::algorithms::sos::tools::to_bytes(std::string("noisy"));
    ASSERT_NO_THROW(utils::algorithms::sos::sos_embed<utils::algorithms::sos::Option::Noise>(carrier, payload));
    EXPECT_EQ(utils::algorithms::sos::sos_extract(carrier), payload);
}

TEST(Sos, EmbedWithGlobalNoise) {
    Bytes carrier = makeCarrier(200000);
    Bytes payload = utils::algorithms::sos::tools::to_bytes(std::string("global noisy"));
    ASSERT_NO_THROW(utils::algorithms::sos::sos_embed<utils::algorithms::sos::Option::GlobalNoise>(carrier, payload));
    EXPECT_EQ(utils::algorithms::sos::sos_extract(carrier), payload);
}

TEST(Sos, CustomMagic) {
    Bytes carrier = makeCarrier(200000);
    Bytes payload = utils::algorithms::sos::tools::to_bytes(std::string("magic"));
    utils::algorithms::sos::sos_embed<utils::algorithms::sos::Option::None, 0x42>(carrier, payload);
    EXPECT_EQ((utils::algorithms::sos::sos_extract<0x42>(carrier)), payload);
    EXPECT_THROW((void)utils::algorithms::sos::sos_extract(carrier), std::exception);
}

TEST(Sos, CarrierTooSmall) {
    Bytes carrier = makeCarrier(300);
    Bytes payload(100, 0xABCD);
    EXPECT_THROW(utils::algorithms::sos::sos_embed(carrier, payload), std::exception);
}

TEST(Sos, EmptyPayload) {
    Bytes carrier = makeCarrier(200000);
    utils::algorithms::sos::sos_embed(carrier, Bytes{});
    EXPECT_TRUE(utils::algorithms::sos::sos_extract(carrier).empty());
}
