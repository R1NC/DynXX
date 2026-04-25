#include <gtest/gtest.h>
#include <DynXX/CXX/Crypto.hxx>
#include <DynXX/CXX/Coding.hxx>

class DynXXCryptoAesTestSuite : public ::testing::Test {};

namespace {
    struct AesInvalidParamCase {
        Bytes in;
        Bytes key;
    };

    struct AesGcmInvalidParamCase {
        Bytes in;
        Bytes key;
        Bytes iv;
        Bytes aad;
        size_t tagBits;
    };

    struct AesGcmValidCase {
        size_t keyLen;
        size_t dataLen;
        size_t aadLen;
    };

    Bytes makeBytes(size_t len) {
        Bytes out(len);
        for (size_t i = 0; i < len; ++i) {
            out[i] = static_cast<byte>('a' + (i % 26));
        }
        return out;
    }
}

TEST_F(DynXXCryptoAesTestSuite, Encrypt) {
    const auto in = dynxxCodingStr2bytes("hello-dynxx");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    const auto out = dynxxCryptoAesEncrypt(in, key);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out, in);
}

TEST_F(DynXXCryptoAesTestSuite, Decrypt) {
    const auto in = dynxxCodingStr2bytes("hello-dynxx");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    const auto encrypted = dynxxCryptoAesEncrypt(in, key);
    ASSERT_FALSE(encrypted.empty());
    EXPECT_EQ(dynxxCryptoAesDecrypt(encrypted, key), in);
}

TEST_F(DynXXCryptoAesTestSuite, GcmEncrypt) {
    const auto in = dynxxCodingStr2bytes("hello-gcm");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    const auto iv = dynxxCryptoRand(12);
    const auto out = dynxxCryptoAesGcmEncrypt(in, key, iv, 128);
    EXPECT_GT(out.size(), in.size());
}

TEST_F(DynXXCryptoAesTestSuite, GcmDecrypt) {
    const auto in = dynxxCodingStr2bytes("hello-gcm");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    const auto iv = dynxxCryptoRand(12);
    const auto encrypted = dynxxCryptoAesGcmEncrypt(in, key, iv, 128);
    ASSERT_FALSE(encrypted.empty());
    EXPECT_EQ(dynxxCryptoAesGcmDecrypt(encrypted, key, iv, 128), in);
}

TEST_F(DynXXCryptoAesTestSuite, RoundTrip) {
    const auto in = dynxxCodingStr2bytes("aes-roundtrip");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    EXPECT_EQ(dynxxCryptoAesDecrypt(dynxxCryptoAesEncrypt(in, key), key), in);
}

TEST_F(DynXXCryptoAesTestSuite, GcmRoundTrip) {
    const auto in = dynxxCodingStr2bytes("aes-gcm-roundtrip");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    const auto iv = dynxxCryptoRand(12);
    EXPECT_EQ(dynxxCryptoAesGcmDecrypt(dynxxCryptoAesGcmEncrypt(in, key, iv, 128), key, iv, 128), in);
}

TEST_F(DynXXCryptoAesTestSuite, RoundTripVariousInputLengths) {
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    for (const auto len : {1U, 15U, 16U, 17U, 31U, 32U, 33U, 64U}) {
        const auto in = makeBytes(len);
        const auto encrypted = dynxxCryptoAesEncrypt(in, key);
        ASSERT_FALSE(encrypted.empty());
        EXPECT_EQ(dynxxCryptoAesDecrypt(encrypted, key), in);
    }
}

TEST_F(DynXXCryptoAesTestSuite, RoundTripValidKeyLengthMatrix) {
    const auto in = makeBytes(33);
    for (const auto keyLen : {16U, 24U, 32U}) {
        const auto key = makeBytes(keyLen);
        const auto encrypted = dynxxCryptoAesEncrypt(in, key);
        ASSERT_FALSE(encrypted.empty());
        EXPECT_EQ(dynxxCryptoAesDecrypt(encrypted, key), in);
    }
}

TEST_F(DynXXCryptoAesTestSuite, InvalidParamsEncryptDecryptShouldReturnEmpty) {
    for (const auto &param : {
        AesInvalidParamCase{.in=dynxxCodingStr2bytes("hello"), .key=dynxxCodingStr2bytes("12345678")},
        AesInvalidParamCase{.in=dynxxCodingStr2bytes("hello"), .key=dynxxCodingStr2bytes("1234567890abcdef1234567890abcdef1234")},
        AesInvalidParamCase{.in={}, .key=dynxxCodingStr2bytes("1234567890abcdef")}
    }) {
        EXPECT_TRUE(dynxxCryptoAesEncrypt(param.in, param.key).empty());
        EXPECT_TRUE(dynxxCryptoAesDecrypt(param.in, param.key).empty());
    }
}

TEST_F(DynXXCryptoAesTestSuite, GcmInvalidParamsEncryptDecryptShouldReturnEmpty) {
    for (const auto &param : {
        AesGcmInvalidParamCase{.in=dynxxCodingStr2bytes("hello"), .key=dynxxCodingStr2bytes("12345678"), .iv=dynxxCryptoRand(12), .aad={}, .tagBits=128},
        AesGcmInvalidParamCase{.in=dynxxCodingStr2bytes("hello"), .key=dynxxCodingStr2bytes("1234567890123456789012345678901234567890"), .iv=dynxxCryptoRand(12), .aad={}, .tagBits=128},
        AesGcmInvalidParamCase{.in=dynxxCodingStr2bytes("hello"), .key=dynxxCodingStr2bytes("1234567890abcdef"), .iv={}, .aad={}, .tagBits=128},
        AesGcmInvalidParamCase{.in=dynxxCodingStr2bytes("hello"), .key=dynxxCodingStr2bytes("1234567890abcdef"), .iv=dynxxCryptoRand(12), .aad=dynxxCodingStr2bytes("1234567890abcdefg"), .tagBits=128},
        AesGcmInvalidParamCase{.in=dynxxCodingStr2bytes("hello"), .key=dynxxCodingStr2bytes("1234567890abcdef"), .iv=dynxxCryptoRand(12), .aad={}, .tagBits=88},
        AesGcmInvalidParamCase{.in=dynxxCodingStr2bytes("hello"), .key=dynxxCodingStr2bytes("1234567890abcdef"), .iv=dynxxCryptoRand(12), .aad={}, .tagBits=136},
        AesGcmInvalidParamCase{.in={}, .key=dynxxCodingStr2bytes("1234567890abcdef"), .iv=dynxxCryptoRand(12), .aad={}, .tagBits=128}
    }) {
        EXPECT_TRUE(dynxxCryptoAesGcmEncrypt(param.in, param.key, param.iv, param.tagBits, param.aad).empty());
        EXPECT_TRUE(dynxxCryptoAesGcmDecrypt(param.in, param.key, param.iv, param.tagBits, param.aad).empty());
    }
}

TEST_F(DynXXCryptoAesTestSuite, GcmValidParamsEncryptDecryptRoundTrip) {
    for (const auto &param : {
        AesGcmValidCase{.keyLen=16, .dataLen=1, .aadLen=0},
        AesGcmValidCase{.keyLen=16, .dataLen=16, .aadLen=16},
        AesGcmValidCase{.keyLen=24, .dataLen=17, .aadLen=8},
        AesGcmValidCase{.keyLen=24, .dataLen=33, .aadLen=16},
        AesGcmValidCase{.keyLen=32, .dataLen=31, .aadLen=4},
        AesGcmValidCase{.keyLen=32, .dataLen=64, .aadLen=16}
    }) {
        const auto in = makeBytes(param.dataLen);
        const auto key = makeBytes(param.keyLen);
        const auto iv = dynxxCryptoRand(12);
        const auto aad = makeBytes(param.aadLen);
        const auto encrypted = dynxxCryptoAesGcmEncrypt(in, key, iv, 128, aad);
        ASSERT_FALSE(encrypted.empty());
        EXPECT_EQ(dynxxCryptoAesGcmDecrypt(encrypted, key, iv, 128, aad), in);
    }
}


