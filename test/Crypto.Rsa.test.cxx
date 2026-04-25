#include <gtest/gtest.h>
#include <DynXX/CXX/Crypto.hxx>
#include <DynXX/CXX/Coding.hxx>

class DynXXCryptoRsaTestSuite : public ::testing::Test {};

namespace {
    constexpr auto kRsaPublicKeyPem = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4F4vWeeGYo78xoZeync3
Hdls3JqqyyaXYORaQwkSPlVJZBiG3BsapkfuPLzZ1KZGoJswAPiJ1bBoE/Ovrg/6
rBFwTycOj+B87dcFArSHJ3ddOw60OC9HRn/G5xiRac8WwoBSvCfA48IxMlj6A+cX
u/PSt+QsUOAXunWdK5XpnSYqYUmSziogp9/wpXZY4MmOb7dY45mYLi8C3PCo62bn
3umMCGy6QGCtoDd6EYIjBn5saRFb/FtKnuq2noy6ix7ERjpz5SPf6mdEcHYu0XR2
EVAqWVt3Qov3KKxOptdcvl0ga9grh/sAS8dCdUnuGRXcil6hsNdb3jZoglNXMVng
WQIDAQAB
-----END PUBLIC KEY-----
)";

    constexpr auto kRsaPrivateKeyPem = R"(-----BEGIN PRIVATE KEY-----
MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQDgXi9Z54ZijvzG
hl7Kdzcd2WzcmqrLJpdg5FpDCRI+VUlkGIbcGxqmR+48vNnUpkagmzAA+InVsGgT
86+uD/qsEXBPJw6P4Hzt1wUCtIcnd107DrQ4L0dGf8bnGJFpzxbCgFK8J8DjwjEy
WPoD5xe789K35CxQ4Be6dZ0rlemdJiphSZLOKiCn3/CldljgyY5vt1jjmZguLwLc
8KjrZufe6YwIbLpAYK2gN3oRgiMGfmxpEVv8W0qe6raejLqLHsRGOnPlI9/qZ0Rw
di7RdHYRUCpZW3dCi/corE6m11y+XSBr2CuH+wBLx0J1Se4ZFdyKXqGw11veNmiC
U1cxWeBZAgMBAAECggEAOz9jzZ/f0sLjaSEugtR8T3uMzaxiEYc9Ti6+zxjkImWv
HamEYzcdEG/KL2vHMeo+mEaipH79FtqggTRGXfaGQU3RlHVB2Sh0lPHEtvQpBHj2
u5Okb3m8npc7VOcd7wzqe64bKuhsfThYeW0ZiFJL6Cv4puqv8SNb4LcSK+JIcGCX
FdZI4CbYKcpnTn6DzMTJ/809eTecj241HVrG0F6sL9+7+I5NpqocKnGy7kwZU+lV
D7jgCmbEP02fFiqt+WLnRIAxjtqPbmcbKWmS98iA2leBTGoauf5BZ3dIXPkNF0V/
WkEbGJfvJfZ2j8QK2fpuxvu/4X9oSKz/Gnpd7EyZKQKBgQDzJ492JVDOltsGKMFc
2dq7hgSsCXh1e2Uro0TXsDDt9QJzbpBmyfc88Bn3A21eZmM/mXt0WqAm8qvCzkCk
SzgWEQ3LWXK6YchOXH1f2KsCQDJRrAnJDvZ9ACbnAkBdfv1zJWrEZzKNS2GgX4A8
UMypD4wy857PFIzNcflk5VmdAwKBgQDsOI2L1X86r1Id7CCNNjsVO+WK8NWyYLzQ
sYWmZ5ia0ax18BFPVf14yTwr3iNUWYkwDLWYDvoWPXsukqUx+ErJb1Ex8/pOb4Za
VZSv1AU8CVKw07q4aCv0u8kgsazUdnPYPIJyjWeAq+DJyPJGrxcLC6m17pvmZlse
ovunHHrIcwKBgBlJ5AHFn8HcrqrVUkb14WnfZlz+HykYzEzdjitgh7Y35SlcNmh6
WNxdnFW0ZYuL2oaiz9QqGL30PyjjipWLvSmOXIDprKT1xn9f3yo4vpBdXN5hpiLj
C3+0pFciRM90haeUHKaQlRFc5apoRE4BmUd79njWHY4r9PjpAaccs1VBAoGAb8sz
8gHQfM2MZ2zg2vPXcm0ubVsdR5ZQxdO+FMrihiLFI+vBxXJEy20xdPFkdv8I1hyt
tJEh+nSOX7Q3Sjbf7gUowCNfSs+tHgNyouQMtiUTZWxO6gcjBmf4/+gWi1aj7Kp2
chBJ87NkZxsazij/FtMmNlv+nEPEy/MGdkgcM9ECgYAIkbFQehPvDKeG4gUR1K5d
cIhoHaYqn8rVdnbRJMBzbu0IOcNxgIqiOBC2ssc9R4srqbPxP1lPhz/HpIYTY1PW
S54oP3/JwnofMRC3K4CHj9gT/91C6yHA1XdNlZF/MZoaBq8jgoLU6pZS1zWSIf4e
88GRyKhERL6BWgd7EdCnOw==
-----END PRIVATE KEY-----
)";
}

TEST_F(DynXXCryptoRsaTestSuite, GenKey) {
    const auto key = dynxxCryptoRsaGenKey("QUJDRA==", true);
    EXPECT_NE(key.find("BEGIN PUBLIC KEY"), std::string::npos);
}

TEST_F(DynXXCryptoRsaTestSuite, GenKeyEmptyBase64) {
    EXPECT_TRUE(dynxxCryptoRsaGenKey("", true).empty());
    EXPECT_TRUE(dynxxCryptoRsaGenKey("", false).empty());
}

TEST_F(DynXXCryptoRsaTestSuite, GenKeyTrimmedEmptyBase64ShouldFail) {
    EXPECT_TRUE(dynxxCryptoRsaGenKey(" \r\n\t ", true).empty());
    EXPECT_TRUE(dynxxCryptoRsaGenKey(" \r\n\t ", false).empty());
}

TEST_F(DynXXCryptoRsaTestSuite, GenKeyInvalidBase64LengthShouldFail) {
    EXPECT_TRUE(dynxxCryptoRsaGenKey("QUJDRA=", true).empty());
    EXPECT_TRUE(dynxxCryptoRsaGenKey("QUJDRA=", false).empty());
}

TEST_F(DynXXCryptoRsaTestSuite, Encrypt) {
    const auto in = dynxxCodingStr2bytes("x");
    EXPECT_TRUE(dynxxCryptoRsaEncrypt(in, {}, DynXXCryptoRSAPaddingX::PKCS1).empty());
}

TEST_F(DynXXCryptoRsaTestSuite, Decrypt) {
    const auto in = dynxxCodingStr2bytes("x");
    EXPECT_TRUE(dynxxCryptoRsaDecrypt(in, {}, DynXXCryptoRSAPaddingX::PKCS1).empty());
}

TEST_F(DynXXCryptoRsaTestSuite, RoundTrip) {
    const auto in = dynxxCodingStr2bytes("rsa-roundtrip");
    const auto publicKey = dynxxCodingStr2bytes(kRsaPublicKeyPem);
    const auto privateKey = dynxxCodingStr2bytes(kRsaPrivateKeyPem);
    const auto encrypted = dynxxCryptoRsaEncrypt(in, publicKey, DynXXCryptoRSAPaddingX::PKCS1);
    ASSERT_FALSE(encrypted.empty());
    const auto decrypted = dynxxCryptoRsaDecrypt(encrypted, privateKey, DynXXCryptoRSAPaddingX::PKCS1);
    EXPECT_EQ(decrypted, in);
}



