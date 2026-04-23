#include <gtest/gtest.h>
#include <DynXX/CXX/Crypto.hxx>
#include <DynXX/CXX/Coding.hxx>

class DynXXCryptoHashTestSuite : public ::testing::Test {};

TEST_F(DynXXCryptoHashTestSuite, Md5) {
    EXPECT_EQ(dynxxCryptoHashMd5(dynxxCodingStr2bytes("abc")).size(), 16U);
}

TEST_F(DynXXCryptoHashTestSuite, Sha1) {
    EXPECT_EQ(dynxxCryptoHashSha1(dynxxCodingStr2bytes("abc")).size(), 20U);
}

TEST_F(DynXXCryptoHashTestSuite, Sha256) {
    EXPECT_EQ(dynxxCryptoHashSha256(dynxxCodingStr2bytes("abc")).size(), 32U);
}

TEST_F(DynXXCryptoHashTestSuite, Md5_EmptyInput) {
    EXPECT_TRUE(dynxxCryptoHashMd5({}).empty());
}

TEST_F(DynXXCryptoHashTestSuite, Sha1_EmptyInput) {
    EXPECT_TRUE(dynxxCryptoHashSha1({}).empty());
}

TEST_F(DynXXCryptoHashTestSuite, Sha256_EmptyInput) {
    EXPECT_TRUE(dynxxCryptoHashSha256({}).empty());
}



