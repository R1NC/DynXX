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

TEST_F(DynXXCryptoAesTestSuite, RoundTrip_VariousInputLengths) {
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    for (const auto len : {1U, 15U, 16U, 17U, 31U, 32U, 33U, 64U}) {
        const auto in = makeBytes(len);
        const auto encrypted = dynxxCryptoAesEncrypt(in, key);
        ASSERT_FALSE(encrypted.empty());
        EXPECT_EQ(dynxxCryptoAesDecrypt(encrypted, key), in);
    }
}

TEST_F(DynXXCryptoAesTestSuite, RoundTrip_ValidKeyLengthMatrix) {
    const auto in = makeBytes(33);
    for (const auto keyLen : {16U, 24U, 32U}) {
        const auto key = makeBytes(keyLen);
        const auto encrypted = dynxxCryptoAesEncrypt(in, key);
        ASSERT_FALSE(encrypted.empty());
        EXPECT_EQ(dynxxCryptoAesDecrypt(encrypted, key), in);
    }
}

TEST_F(DynXXCryptoAesTestSuite, InvalidParams_EncryptDecryptShouldReturnEmpty) {
    for (const auto &param : {
        AesInvalidParamCase{dynxxCodingStr2bytes("hello"), dynxxCodingStr2bytes("12345678")},
        AesInvalidParamCase{dynxxCodingStr2bytes("hello"), dynxxCodingStr2bytes("1234567890abcdef1234567890abcdef1234")},
        AesInvalidParamCase{{}, dynxxCodingStr2bytes("1234567890abcdef")}
    }) {
        EXPECT_TRUE(dynxxCryptoAesEncrypt(param.in, param.key).empty());
        EXPECT_TRUE(dynxxCryptoAesDecrypt(param.in, param.key).empty());
    }
}

TEST_F(DynXXCryptoAesTestSuite, GcmInvalidParams_EncryptDecryptShouldReturnEmpty) {
    for (const auto &param : {
        AesGcmInvalidParamCase{dynxxCodingStr2bytes("hello"), dynxxCodingStr2bytes("12345678"), dynxxCryptoRand(12), {}, 128},
        AesGcmInvalidParamCase{dynxxCodingStr2bytes("hello"), dynxxCodingStr2bytes("1234567890123456789012345678901234567890"), dynxxCryptoRand(12), {}, 128},
        AesGcmInvalidParamCase{dynxxCodingStr2bytes("hello"), dynxxCodingStr2bytes("1234567890abcdef"), {}, {}, 128},
        AesGcmInvalidParamCase{dynxxCodingStr2bytes("hello"), dynxxCodingStr2bytes("1234567890abcdef"), dynxxCryptoRand(12), dynxxCodingStr2bytes("1234567890abcdefg"), 128},
        AesGcmInvalidParamCase{dynxxCodingStr2bytes("hello"), dynxxCodingStr2bytes("1234567890abcdef"), dynxxCryptoRand(12), {}, 88},
        AesGcmInvalidParamCase{dynxxCodingStr2bytes("hello"), dynxxCodingStr2bytes("1234567890abcdef"), dynxxCryptoRand(12), {}, 136},
        AesGcmInvalidParamCase{{}, dynxxCodingStr2bytes("1234567890abcdef"), dynxxCryptoRand(12), {}, 128}
    }) {
        EXPECT_TRUE(dynxxCryptoAesGcmEncrypt(param.in, param.key, param.iv, param.tagBits, param.aad).empty());
        EXPECT_TRUE(dynxxCryptoAesGcmDecrypt(param.in, param.key, param.iv, param.tagBits, param.aad).empty());
    }
}

TEST_F(DynXXCryptoAesTestSuite, GcmValidParams_EncryptDecryptRoundTrip) {
    for (const auto &param : {
        AesGcmValidCase{16, 1, 0},
        AesGcmValidCase{16, 16, 16},
        AesGcmValidCase{24, 17, 8},
        AesGcmValidCase{24, 33, 16},
        AesGcmValidCase{32, 31, 4},
        AesGcmValidCase{32, 64, 16}
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


