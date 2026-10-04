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
##  @file Base64Codec.cpp

File Description:
##  Unit tests of the Base64Codec
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <string>

struct Base64Case {
    std::string decoded;
    std::string encoded;
};
std::ostream& operator<<(std::ostream& os, const Base64Case& c) {return os << '"' << c.decoded << '"';}

class Base64CodecTest: public ::testing::TestWithParam<Base64Case> {
    protected:
        utils::smanip::codec::Base64Codec _codec;
};

TEST_P(Base64CodecTest, Encode) {
    EXPECT_EQ(this->_codec.encode(GetParam().decoded), GetParam().encoded);
}

TEST_P(Base64CodecTest, Decode) {
    EXPECT_EQ(this->_codec.decode(GetParam().encoded), GetParam().decoded);
}

TEST_P(Base64CodecTest, RoundTrip) {
    EXPECT_EQ(this->_codec.decode(this->_codec.encode(GetParam().decoded)), GetParam().decoded);
}

// RFC 4648 test vectors
INSTANTIATE_TEST_SUITE_P(RFC4648, Base64CodecTest,
    ::testing::Values(
        Base64Case{"", ""},
        Base64Case{"f", "Zg=="},
        Base64Case{"fo", "Zm8="},
        Base64Case{"foo", "Zm9v"},
        Base64Case{"foob", "Zm9vYg=="},
        Base64Case{"fooba", "Zm9vYmE="},
        Base64Case{"foobar", "Zm9vYmFy"}
    )
);

TEST(Base64Codec, BinaryRoundTrip) {
    utils::smanip::codec::Base64Codec codec;
    std::string binary;
    for (int i = 0; i < 256; ++i) binary += static_cast<char>(i);
    binary += std::string("\0\0\0", 3);
    EXPECT_EQ(codec.decode(codec.encode(binary)), binary);
}

TEST(Base64Codec, EncodedHasNoReservedFramingBytes) {
    // The EETPParser rely on ETB (0x17) & EOT (0x04) never being in the encoded output
    utils::smanip::codec::Base64Codec codec;
    std::string binary;
    for (int i = 0; i < 256; ++i) binary += static_cast<char>(i);
    std::string encoded = codec.encode(binary);
    EXPECT_EQ(encoded.find('\x17'), std::string::npos);
    EXPECT_EQ(encoded.find('\x04'), std::string::npos);
}

TEST(Base64Codec, InvalidDecode) {
    utils::smanip::codec::Base64Codec codec;
    try {
        (void)codec.decode("!!!*");
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Codec);
    }
}

TEST(Base64Codec, Polymorphism) {
    std::unique_ptr<utils::smanip::codec::ICodec> codec = std::make_unique<utils::smanip::codec::Base64Codec>();
    EXPECT_EQ(codec->encode("foo"), "Zm9v");
    EXPECT_EQ(codec->decode("Zm9v"), "foo");
}

TEST(Base64Codec, DecodeTrailingWhitespace) {
    utils::smanip::codec::Base64Codec codec;
    EXPECT_EQ(codec.decode("QQ==\n"), "A");
    EXPECT_EQ(codec.decode("Zm8= \r\n"), "fo");
    EXPECT_EQ(codec.decode("Zm9v\n"), "foo");
}
