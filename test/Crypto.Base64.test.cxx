#include <gtest/gtest.h>
#include <DynXX/CXX/Crypto.hxx>
#include <DynXX/CXX/Coding.hxx>

class DynXXCryptoBase64TestSuite : public ::testing::Test {};

namespace {
    struct Base64LengthCase {
        size_t dataLen;
        bool noNewLines;
    };

    Bytes makeBytes(size_t len) {
        Bytes out(len);
        for (size_t i = 0; i < len; ++i) {
            out[i] = static_cast<byte>('a' + (i % 26));
        }
        return out;
    }
}

TEST_F(DynXXCryptoBase64TestSuite, Encode) {
    const auto encoded = dynxxCryptoBase64Encode(dynxxCodingStr2bytes("abc"), true);
    EXPECT_EQ(dynxxCodingBytes2str(encoded), "YWJj");
}

TEST_F(DynXXCryptoBase64TestSuite, Decode) {
    const auto decoded = dynxxCryptoBase64Decode(dynxxCodingStr2bytes("YWJj"), true);
    EXPECT_EQ(dynxxCodingBytes2str(decoded), "abc");
}

TEST_F(DynXXCryptoBase64TestSuite, EncodeDecodeEmptyInput) {
    EXPECT_TRUE(dynxxCryptoBase64Encode({}, true).empty());
    EXPECT_TRUE(dynxxCryptoBase64Decode({}, true).empty());
    EXPECT_TRUE(dynxxCryptoBase64Encode({}, false).empty());
    EXPECT_TRUE(dynxxCryptoBase64Decode({}, false).empty());
}

TEST_F(DynXXCryptoBase64TestSuite, RoundTrip) {
    const auto in = dynxxCodingStr2bytes("base64-roundtrip");
    EXPECT_EQ(dynxxCryptoBase64Decode(dynxxCryptoBase64Encode(in, true), true), in);
}

TEST_F(DynXXCryptoBase64TestSuite, EncodeDecodeRoundTripLengthMatrix) {
    for (const auto &param : {
        Base64LengthCase{.dataLen=1, .noNewLines=true},
        Base64LengthCase{.dataLen=2, .noNewLines=true},
        Base64LengthCase{.dataLen=3, .noNewLines=true},
        Base64LengthCase{.dataLen=4, .noNewLines=true},
        Base64LengthCase{.dataLen=7, .noNewLines=true},
        Base64LengthCase{.dataLen=15, .noNewLines=false},
        Base64LengthCase{.dataLen=16, .noNewLines=false},
        Base64LengthCase{.dataLen=31, .noNewLines=false},
        Base64LengthCase{.dataLen=32, .noNewLines=false}
    }) {
        const auto in = makeBytes(param.dataLen);
        const auto encoded = dynxxCryptoBase64Encode(in, param.noNewLines);
        ASSERT_FALSE(encoded.empty());
        EXPECT_EQ(dynxxCryptoBase64Decode(encoded, param.noNewLines), in);
    }
}

TEST_F(DynXXCryptoBase64TestSuite, DecodeNoNewLinesFlagMismatchShouldFail) {
    const auto in = makeBytes(32);
    const auto encodedWithNewLines = dynxxCryptoBase64Encode(in, false);
    ASSERT_FALSE(encodedWithNewLines.empty());
    EXPECT_TRUE(dynxxCryptoBase64Decode(encodedWithNewLines, true).empty());
    EXPECT_EQ(dynxxCryptoBase64Decode(encodedWithNewLines, false), in);
}

TEST_F(DynXXCryptoBase64TestSuite, DecodeInvalidCharactersShouldFail) {
    auto encoded = dynxxCryptoBase64Encode(makeBytes(8), true);
    ASSERT_FALSE(encoded.empty());
    encoded[2] = static_cast<byte>('@');
    EXPECT_TRUE(dynxxCryptoBase64Decode(encoded, true).empty());
    EXPECT_TRUE(dynxxCryptoBase64Decode(encoded, false).empty());
}

TEST_F(DynXXCryptoBase64TestSuite, DecodeInsertedCRLFNoNewLinesFalseShouldPass) {
    const auto in = makeBytes(48);
    auto encoded = dynxxCryptoBase64Encode(in, true);
    ASSERT_FALSE(encoded.empty());
    encoded.insert(encoded.begin() + 4, static_cast<byte>('\r'));
    encoded.insert(encoded.begin() + 5, static_cast<byte>('\n'));
    EXPECT_EQ(dynxxCryptoBase64Decode(encoded, false), in);
    EXPECT_TRUE(dynxxCryptoBase64Decode(encoded, true).empty());
}

TEST_F(DynXXCryptoBase64TestSuite, DecodeWhitespaceCharactersShouldFail) {
    const auto in = makeBytes(12);
    const auto encoded = dynxxCryptoBase64Encode(in, true);
    ASSERT_FALSE(encoded.empty());
    for (const auto c : {static_cast<byte>(' '), static_cast<byte>('\t')}) {
        auto mutated = encoded;
        mutated[2] = c;
        EXPECT_TRUE(dynxxCryptoBase64Decode(mutated, true).empty());
        EXPECT_TRUE(dynxxCryptoBase64Decode(mutated, false).empty());
    }
}

TEST_F(DynXXCryptoBase64TestSuite, DecodeUrlSafeCharactersShouldFail) {
    const auto in = makeBytes(12);
    const auto encoded = dynxxCryptoBase64Encode(in, true);
    ASSERT_FALSE(encoded.empty());
    for (const auto c : {static_cast<byte>('-'), static_cast<byte>('_')}) {
        auto mutated = encoded;
        mutated[2] = c;
        EXPECT_TRUE(dynxxCryptoBase64Decode(mutated, true).empty());
        EXPECT_TRUE(dynxxCryptoBase64Decode(mutated, false).empty());
    }
}



