#include <gtest/gtest.h>
#include <DynXX/CXX/Crypto.hxx>
#include <DynXX/CXX/Coding.hxx>

TEST(Crypto, DynxxCryptoHashMd5) {
    EXPECT_EQ(dynxxCryptoHashMd5(dynxxCodingStr2bytes("abc")).size(), 16U);
}

TEST(Crypto, DynxxCryptoHashSha1) {
    EXPECT_EQ(dynxxCryptoHashSha1(dynxxCodingStr2bytes("abc")).size(), 20U);
}

TEST(Crypto, DynxxCryptoHashSha256) {
    EXPECT_EQ(dynxxCryptoHashSha256(dynxxCodingStr2bytes("abc")).size(), 32U);
}

TEST(Crypto, DynxxCryptoHashMd5_EmptyInput) {
    EXPECT_TRUE(dynxxCryptoHashMd5({}).empty());
}

TEST(Crypto, DynxxCryptoHashSha1_EmptyInput) {
    EXPECT_TRUE(dynxxCryptoHashSha1({}).empty());
}

TEST(Crypto, DynxxCryptoHashSha256_EmptyInput) {
    EXPECT_TRUE(dynxxCryptoHashSha256({}).empty());
}
