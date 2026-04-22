#include <gtest/gtest.h>
#include <DynXX/CXX/Coding.hxx>

TEST(Coding, DynxxCodingCaseUpper) {
    EXPECT_EQ(dynxxCodingCaseUpper("Abc_123 xYz"), "ABC_123 XYZ");
}

TEST(Coding, DynxxCodingCaseLower) {
    EXPECT_EQ(dynxxCodingCaseLower("AbC_123 XyZ"), "abc_123 xyz");
}

TEST(Coding, DynxxCodingHexBytes2str) {
    const auto bytes = dynxxCodingStr2bytes("Ab");
    EXPECT_EQ(dynxxCodingHexBytes2str(bytes), "4162");
    EXPECT_TRUE(dynxxCodingHexBytes2str({}).empty());
}

TEST(Coding, DynxxCodingHexStr2bytes) {
    const auto bytes = dynxxCodingHexStr2bytes("41 62");
    EXPECT_EQ(dynxxCodingBytes2str(bytes), "Ab");
}

TEST(Coding, DynxxCodingHexStr2bytes_OddLength) {
    const auto bytes = dynxxCodingHexStr2bytes("abc");
    ASSERT_EQ(bytes.size(), 2U);
    EXPECT_EQ(bytes[0], static_cast<byte>(0x0A));
    EXPECT_EQ(bytes[1], static_cast<byte>(0xBC));
}

TEST(Coding, DynxxCodingStr2bytes) {
    const auto bytes = dynxxCodingStr2bytes("coding-str2bytes");
    EXPECT_EQ(bytes.size(), 16U);
    EXPECT_EQ(bytes.front(), static_cast<byte>('c'));
}

TEST(Coding, DynxxCodingBytes2str) {
    const auto bytes = dynxxCodingStr2bytes("coding-bytes2str");
    EXPECT_EQ(dynxxCodingBytes2str(bytes), "coding-bytes2str");
}

TEST(Coding, DynxxCodingStrBytesRoundTrip_WithControlChars) {
    const std::string in("a\0b\tc\n\"\\", 8);
    const auto bytes = dynxxCodingStr2bytes(in);
    ASSERT_EQ(bytes.size(), in.size());
    EXPECT_EQ(dynxxCodingBytes2str(bytes), in);
}

TEST(Coding, DynxxCodingStrTrim) {
    EXPECT_EQ(dynxxCodingStrTrim(" \t\n  hello-dynxx \r\n "), "hello-dynxx");
    EXPECT_TRUE(dynxxCodingStrTrim(" \t\r\n ").empty());
}

TEST(Coding, StrBytesRoundTrip) {
    const std::string in = "str-bytes-roundtrip";
    EXPECT_EQ(dynxxCodingBytes2str(dynxxCodingStr2bytes(in)), in);
}

TEST(Coding, HexRoundTrip) {
    const auto in = dynxxCodingStr2bytes("hex-roundtrip");
    EXPECT_EQ(dynxxCodingHexStr2bytes(dynxxCodingHexBytes2str(in)), in);
}

TEST(Coding, CaseRoundTrip) {
    const std::string in = "aBc123_XyZ";
    EXPECT_EQ(dynxxCodingCaseLower(dynxxCodingCaseUpper(in)), dynxxCodingCaseLower(in));
}

TEST(Coding, StrEscapeQuotes) {
    EXPECT_EQ(dynxxCodingStrEscapeQuotes(R"(a"b"c)"), R"(a\"b\"c)");
    EXPECT_EQ(dynxxCodingStrEscapeQuotes("no-quotes"), "no-quotes");
}

TEST(Coding, StrEscapeQuotes_WithNestedJsonString) {
    const auto nestedJson = R"({"outer":{"k":"v"},"arr":[1,2],"text":"a\"b"})";
    const auto escaped = dynxxCodingStrEscapeQuotes(nestedJson);
    EXPECT_NE(escaped.find(R"(\"outer\")"), std::string::npos);
    EXPECT_NE(escaped.find(R"(\"arr\")"), std::string::npos);
    EXPECT_NE(escaped.find(R"(\"text\")"), std::string::npos);
}

class CodingHexInvalidInputTest : public ::testing::TestWithParam<std::string> {};

TEST_P(CodingHexInvalidInputTest, Str2BytesShouldReturnEmpty) {
    EXPECT_TRUE(dynxxCodingHexStr2bytes(GetParam()).empty());
}

INSTANTIATE_TEST_SUITE_P(
    Coding,
    CodingHexInvalidInputTest,
    ::testing::Values("", " ", "g", "z!")
);
