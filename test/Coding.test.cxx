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
}

TEST(Coding, DynxxCodingHexStr2bytes) {
    const auto bytes = dynxxCodingHexStr2bytes("41 62");
    EXPECT_EQ(dynxxCodingBytes2str(bytes), "Ab");
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
