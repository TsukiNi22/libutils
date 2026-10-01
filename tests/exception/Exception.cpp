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
##  @file Exception.cpp

File Description:
##  Unit tests of the exceptions (None/Error/Warning/Custom/Fatal, restriction, type operators)
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <string>

/* Type operators */
TEST(ExceptionType, BitwiseOperators) {
    using utils::exception::Type;
    EXPECT_EQ(Type::Error | Type::Fatal, static_cast<Type>(0b0110));
    EXPECT_EQ((Type::Error | Type::Fatal) & Type::Fatal, Type::Fatal);
    EXPECT_EQ(Type::Error ^ Type::Error, static_cast<Type>(0));
    EXPECT_EQ(static_cast<std::uint8_t>(~Type::None), static_cast<std::uint8_t>(0b11111110));

    Type t = Type::Error;
    t |= Type::Warning;
    EXPECT_EQ(t, static_cast<Type>(0b1100));
    t &= Type::Warning;
    EXPECT_EQ(t, Type::Warning);
    t ^= Type::Warning;
    EXPECT_EQ(t, static_cast<Type>(0));
}

/* ErrorException */
TEST(ErrorException, CodeTypeAndMessage) {
    utils::exception::ErrorException e(utils::exception::InternalCode::InvalidArgument, "custom info");
    EXPECT_EQ(e.getType(), utils::exception::Type::Error);
    EXPECT_EQ(e.getCode(), utils::exception::InternalCode::InvalidArgument);
    EXPECT_STREQ(e.what(), "Invalid argument given");
    EXPECT_STREQ(e.info(), "custom info");
    EXPECT_FALSE(e.isNone());
    EXPECT_FALSE(e.isFatal());
}

TEST(ErrorException, DefaultInfo) {
    utils::exception::ErrorException e(utils::exception::InternalCode::InvalidArgument);
    EXPECT_STREQ(e.info(), "[None]");
}

TEST(ErrorException, DefaultInfoFromConfig) {
    // VectorInvalidIndex has a default info in the json config
    utils::exception::ErrorException e(utils::exception::InternalCode::VectorInvalidIndex);
    EXPECT_STREQ(e.info(), "Can't retrieve the value, the VectorX dosen't have this index");
}

TEST(ErrorException, UndefinedByDefault) {
    utils::exception::ErrorException e;
    EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Undefined);
    EXPECT_STREQ(e.what(), "An undefined error has occured");
}

TEST(ErrorException, SourceLocation) {
    const std::size_t line = __LINE__ + 1;
    utils::exception::ErrorException e(utils::exception::InternalCode::InvalidArgument);
    EXPECT_EQ(e.loc().line(), line);
    EXPECT_NE(std::string(e.loc().file_name()).find("Exception.cpp"), std::string::npos);
}

TEST(ErrorException, CatchableAsStdException) {
    try {
        throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "oob");
    } catch (const std::exception& e) {
        EXPECT_STREQ(e.what(), "Bounds have been oversteapaded");
        return;
    }
    FAIL() << "Expected a std::exception";
}

TEST(ErrorException, CatchableAsIException) {
    try {
        throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "oob");
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::OutOfBounds);
        EXPECT_STREQ(e.info(), "oob");
        return;
    }
    FAIL() << "Expected a utils::exception::IException";
}

TEST(ErrorException, Formated) {
    utils::exception::ErrorException e(utils::exception::InternalCode::InvalidArgument, "custom info");
    std::string s = e.formated();
    EXPECT_NE(s.find("[Error"), std::string::npos);
    EXPECT_NE(s.find("Invalid argument given"), std::string::npos);
    EXPECT_NE(s.find("custom info"), std::string::npos);
    EXPECT_NE(s.find(std::to_string(e.loc().line())), std::string::npos);
}

/* WarningException */
TEST(WarningException, CodeTypeAndMessage) {
    utils::exception::WarningException e(utils::exception::InternalCode::UnknownId, "42");
    EXPECT_EQ(e.getType(), utils::exception::Type::Warning);
    EXPECT_EQ(e.getCode(), utils::exception::InternalCode::UnknownId);
    EXPECT_STREQ(e.info(), "42");
    EXPECT_NE(e.formated().find("[Warning"), std::string::npos);
}

/* NoneException */
TEST(NoneException, ExitCode) {
    utils::exception::NoneException e(utils::exception::InternalCode::Exit);
    EXPECT_EQ(e.getType(), utils::exception::Type::None);
    EXPECT_TRUE(e.isNone());
    EXPECT_FALSE(e.isFatal());
    EXPECT_STREQ(e.info(), "Exit"); // default info from config
    EXPECT_NE(e.formated().find("[None"), std::string::npos);
}

/* CustomException */
TEST(CustomException, GivenType) {
    utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::InternalCode::Empty, "info");
    EXPECT_EQ(e.getType(), utils::exception::Type::Warning);
    EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Empty);
    EXPECT_STREQ(e.info(), "info");
}

TEST(CustomException, InfoOnly) {
    utils::exception::CustomException e(utils::exception::Type::Error, std::string("only info"));
    EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Undefined);
    EXPECT_STREQ(e.info(), "only info");
}

/* Restriction */
TEST(ExceptionRestrictionDeathTest, NoneRestrictedCodeAsError) {
    // 'Exit' only allow the type None
    EXPECT_DEATH({utils::exception::ErrorException e(utils::exception::InternalCode::Exit);}, "ABORTED");
}

TEST(ExceptionRestrictionDeathTest, ErrorRestrictedCodeAsWarning) {
    // 'Poll' only allow the type Fatal & Error
    EXPECT_DEATH({utils::exception::WarningException e(utils::exception::InternalCode::Poll);}, "ABORTED");
}

TEST(ExceptionRestriction, NoRestrictionAllowsAll) {
    // 'Empty' has no restriction
    EXPECT_NO_FATAL_FAILURE({
        utils::exception::ErrorException e1(utils::exception::InternalCode::Empty);
        utils::exception::WarningException e2(utils::exception::InternalCode::Empty);
        utils::exception::NoneException e3(utils::exception::InternalCode::Empty);
    });
}

/* FatalException */
TEST(FatalExceptionDeathTest, AbortOnConstruction) {
    EXPECT_DEATH({utils::exception::FatalException e(utils::exception::InternalCode::InvalidArgument, "fatal info");}, "fatal info");
}

TEST(FatalExceptionDeathTest, AbortFromOtherException) {
    utils::exception::ErrorException error(utils::exception::InternalCode::InvalidArgument, "from error");
    EXPECT_DEATH({utils::exception::FatalException e(error);}, "from error");
}
