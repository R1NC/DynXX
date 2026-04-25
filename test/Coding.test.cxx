#include <gtest/gtest.h>
#include <DynXX/CXX/Coding.hxx>

class DynXXCodingTestSuite : public ::testing::Test {};

TEST_F(DynXXCodingTestSuite, CaseUpper) {
    EXPECT_EQ(dynxxCodingCaseUpper("Abc_123 xYz"), "ABC_123 XYZ");
}

TEST_F(DynXXCodingTestSuite, CaseLower) {
    EXPECT_EQ(dynxxCodingCaseLower("AbC_123 XyZ"), "abc_123 xyz");
}

TEST_F(DynXXCodingTestSuite, HexBytes2Str) {
    const auto bytes = dynxxCodingStr2bytes("Ab");
    EXPECT_EQ(dynxxCodingHexBytes2str(bytes), "4162");
    EXPECT_TRUE(dynxxCodingHexBytes2str({}).empty());
}

TEST_F(DynXXCodingTestSuite, HexStr2Bytes) {
    const auto bytes = dynxxCodingHexStr2bytes("41 62");
    EXPECT_EQ(dynxxCodingBytes2str(bytes), "Ab");
}

TEST_F(DynXXCodingTestSuite, HexStr2BytesOddLength) {
    const auto bytes = dynxxCodingHexStr2bytes("abc");
    ASSERT_EQ(bytes.size(), 2U);
    EXPECT_EQ(bytes[0], static_cast<byte>(0x0A));
    EXPECT_EQ(bytes[1], static_cast<byte>(0xBC));
}

TEST_F(DynXXCodingTestSuite, Str2Bytes) {
    const auto bytes = dynxxCodingStr2bytes("coding-str2bytes");
    EXPECT_EQ(bytes.size(), 16U);
    EXPECT_EQ(bytes.front(), static_cast<byte>('c'));
}

TEST_F(DynXXCodingTestSuite, Bytes2Str) {
    const auto bytes = dynxxCodingStr2bytes("coding-bytes2str");
    EXPECT_EQ(dynxxCodingBytes2str(bytes), "coding-bytes2str");
}

TEST_F(DynXXCodingTestSuite, StrBytesRoundTripWithControlChars) {
    const std::string in("a\0b\tc\n\"\\", 8);
    const auto bytes = dynxxCodingStr2bytes(in);
    ASSERT_EQ(bytes.size(), in.size());
    EXPECT_EQ(dynxxCodingBytes2str(bytes), in);
}

TEST_F(DynXXCodingTestSuite, StrTrim) {
    EXPECT_EQ(dynxxCodingStrTrim(" \t\n  hello-dynxx \r\n "), "hello-dynxx");
    EXPECT_TRUE(dynxxCodingStrTrim(" \t\r\n ").empty());
}

TEST_F(DynXXCodingTestSuite, StrBytesRoundTrip) {
    const std::string in = "str-bytes-roundtrip";
    EXPECT_EQ(dynxxCodingBytes2str(dynxxCodingStr2bytes(in)), in);
}

TEST_F(DynXXCodingTestSuite, HexRoundTrip) {
    const auto in = dynxxCodingStr2bytes("hex-roundtrip");
    EXPECT_EQ(dynxxCodingHexStr2bytes(dynxxCodingHexBytes2str(in)), in);
}

TEST_F(DynXXCodingTestSuite, CaseRoundTrip) {
    const std::string in = "aBc123_XyZ";
    EXPECT_EQ(dynxxCodingCaseLower(dynxxCodingCaseUpper(in)), dynxxCodingCaseLower(in));
}

TEST_F(DynXXCodingTestSuite, StrEscapeQuotes) {
    EXPECT_EQ(dynxxCodingStrEscapeQuotes(R"(a"b"c)"), R"(a\"b\"c)");
    EXPECT_EQ(dynxxCodingStrEscapeQuotes("no-quotes"), "no-quotes");
}

TEST_F(DynXXCodingTestSuite, StrEscapeQuotesWithNestedJsonString) {
    const auto nestedJson = R"({"outer":{"k":"v"},"arr":[1,2],"text":"a\"b"})";
    const auto escaped = dynxxCodingStrEscapeQuotes(nestedJson);
    EXPECT_NE(escaped.find(R"(\"outer\")"), std::string::npos);
    EXPECT_NE(escaped.find(R"(\"arr\")"), std::string::npos);
    EXPECT_NE(escaped.find(R"(\"text\")"), std::string::npos);
}

TEST_F(DynXXCodingTestSuite, HexInvalidInputStr2BytesShouldReturnEmpty) {
    for (const auto &input : {std::string(""), std::string(" "), std::string("g"), std::string("z!")}) {
        EXPECT_TRUE(dynxxCodingHexStr2bytes(input).empty());
    }
}


