#include <gtest/gtest.h>
#include <DynXX/CXX/Crypto.hxx>
#include <DynXX/CXX/Coding.hxx>

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

TEST(Crypto, DynxxCryptoBase64Encode) {
    const auto encoded = dynxxCryptoBase64Encode(dynxxCodingStr2bytes("abc"), true);
    EXPECT_EQ(dynxxCodingBytes2str(encoded), "YWJj");
}

TEST(Crypto, DynxxCryptoBase64Decode) {
    const auto decoded = dynxxCryptoBase64Decode(dynxxCodingStr2bytes("YWJj"), true);
    EXPECT_EQ(dynxxCodingBytes2str(decoded), "abc");
}

TEST(Crypto, DynxxCryptoBase64EncodeDecode_EmptyInput) {
    EXPECT_TRUE(dynxxCryptoBase64Encode({}, true).empty());
    EXPECT_TRUE(dynxxCryptoBase64Decode({}, true).empty());
    EXPECT_TRUE(dynxxCryptoBase64Encode({}, false).empty());
    EXPECT_TRUE(dynxxCryptoBase64Decode({}, false).empty());
}

TEST(Crypto, Base64RoundTrip) {
    const auto in = dynxxCodingStr2bytes("base64-roundtrip");
    EXPECT_EQ(dynxxCryptoBase64Decode(dynxxCryptoBase64Encode(in, true), true), in);
}

class CryptoBase64LengthTest : public ::testing::TestWithParam<Base64LengthCase> {};

TEST_P(CryptoBase64LengthTest, EncodeDecodeRoundTrip) {
    const auto &param = GetParam();
    const auto in = makeBytes(param.dataLen);
    const auto encoded = dynxxCryptoBase64Encode(in, param.noNewLines);
    ASSERT_FALSE(encoded.empty());
    EXPECT_EQ(dynxxCryptoBase64Decode(encoded, param.noNewLines), in);
}

TEST(Crypto, Base64Decode_NoNewLinesFlagMismatch_ShouldFail) {
    const auto in = makeBytes(32);
    const auto encodedWithNewLines = dynxxCryptoBase64Encode(in, false);
    ASSERT_FALSE(encodedWithNewLines.empty());
    EXPECT_TRUE(dynxxCryptoBase64Decode(encodedWithNewLines, true).empty());
    EXPECT_EQ(dynxxCryptoBase64Decode(encodedWithNewLines, false), in);
}

TEST(Crypto, Base64Decode_InvalidCharacters_ShouldFail) {
    auto encoded = dynxxCryptoBase64Encode(makeBytes(8), true);
    ASSERT_FALSE(encoded.empty());
    encoded[2] = static_cast<byte>('@');
    EXPECT_TRUE(dynxxCryptoBase64Decode(encoded, true).empty());
    EXPECT_TRUE(dynxxCryptoBase64Decode(encoded, false).empty());
}

TEST(Crypto, Base64Decode_InsertedCRLF_NoNewLinesFalse_ShouldPass) {
    const auto in = makeBytes(48);
    auto encoded = dynxxCryptoBase64Encode(in, true);
    ASSERT_FALSE(encoded.empty());
    encoded.insert(encoded.begin() + 4, static_cast<byte>('\r'));
    encoded.insert(encoded.begin() + 5, static_cast<byte>('\n'));
    EXPECT_EQ(dynxxCryptoBase64Decode(encoded, false), in);
    EXPECT_TRUE(dynxxCryptoBase64Decode(encoded, true).empty());
}

TEST(Crypto, Base64Decode_WhitespaceCharacters_ShouldFail) {
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

TEST(Crypto, Base64Decode_UrlSafeCharacters_ShouldFail) {
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

INSTANTIATE_TEST_SUITE_P(
    LengthMatrix,
    CryptoBase64LengthTest,
    ::testing::Values(
        Base64LengthCase{1, true},
        Base64LengthCase{2, true},
        Base64LengthCase{3, true},
        Base64LengthCase{4, true},
        Base64LengthCase{7, true},
        Base64LengthCase{15, false},
        Base64LengthCase{16, false},
        Base64LengthCase{31, false},
        Base64LengthCase{32, false}
    )
);
