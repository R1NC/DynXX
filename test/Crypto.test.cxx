#include <gtest/gtest.h>
#include <DynXX/CXX/Crypto.hxx>
#include <DynXX/CXX/Coding.hxx>

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

TEST(Crypto, DynxxCryptoRand) {
    constexpr auto kRandSize = 32;
    constexpr auto kRounds = 6;

    auto previous = dynxxCryptoRand(kRandSize);
    ASSERT_EQ(previous.size(), kRandSize);

    bool hasDifference = false;
    for (int i = 0; i < kRounds; ++i) {
        auto current = dynxxCryptoRand(kRandSize);
        ASSERT_EQ(current.size(), kRandSize);
        if (current != previous) {
            hasDifference = true;
        }
        previous = std::move(current);
    }

    EXPECT_TRUE(hasDifference);
}

TEST(Crypto, DynxxCryptoAesEncrypt) {
    const auto in = dynxxCodingStr2bytes("hello-dynxx");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    const auto out = dynxxCryptoAesEncrypt(in, key);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out, in);
}

TEST(Crypto, DynxxCryptoAesDecrypt) {
    const auto in = dynxxCodingStr2bytes("hello-dynxx");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    const auto encrypted = dynxxCryptoAesEncrypt(in, key);
    ASSERT_FALSE(encrypted.empty());
    EXPECT_EQ(dynxxCryptoAesDecrypt(encrypted, key), in);
}

TEST(Crypto, DynxxCryptoAesGcmEncrypt) {
    const auto in = dynxxCodingStr2bytes("hello-gcm");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    const auto iv = dynxxCryptoRand(12);
    const auto out = dynxxCryptoAesGcmEncrypt(in, key, iv, 128);
    EXPECT_GT(out.size(), in.size());
}

TEST(Crypto, DynxxCryptoAesGcmDecrypt) {
    const auto in = dynxxCodingStr2bytes("hello-gcm");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    const auto iv = dynxxCryptoRand(12);
    const auto encrypted = dynxxCryptoAesGcmEncrypt(in, key, iv, 128);
    ASSERT_FALSE(encrypted.empty());
    EXPECT_EQ(dynxxCryptoAesGcmDecrypt(encrypted, key, iv, 128), in);
}

TEST(Crypto, DynxxCryptoRsaGenKey) {
    const auto key = dynxxCryptoRsaGenKey("QUJDRA==", true);
    EXPECT_NE(key.find("BEGIN PUBLIC KEY"), std::string::npos);
}

TEST(Crypto, DynxxCryptoRsaEncrypt) {
    const auto in = dynxxCodingStr2bytes("x");
    EXPECT_TRUE(dynxxCryptoRsaEncrypt(in, {}, DynXXCryptoRSAPaddingX::PKCS1).empty());
}

TEST(Crypto, DynxxCryptoRsaDecrypt) {
    const auto in = dynxxCodingStr2bytes("x");
    EXPECT_TRUE(dynxxCryptoRsaDecrypt(in, {}, DynXXCryptoRSAPaddingX::PKCS1).empty());
}

TEST(Crypto, DynxxCryptoHashMd5) {
    EXPECT_EQ(dynxxCryptoHashMd5(dynxxCodingStr2bytes("abc")).size(), 16U);
}

TEST(Crypto, DynxxCryptoHashSha1) {
    EXPECT_EQ(dynxxCryptoHashSha1(dynxxCodingStr2bytes("abc")).size(), 20U);
}

TEST(Crypto, DynxxCryptoHashSha256) {
    EXPECT_EQ(dynxxCryptoHashSha256(dynxxCodingStr2bytes("abc")).size(), 32U);
}

TEST(Crypto, DynxxCryptoBase64Encode) {
    const auto encoded = dynxxCryptoBase64Encode(dynxxCodingStr2bytes("abc"), true);
    EXPECT_EQ(dynxxCodingBytes2str(encoded), "YWJj");
}

TEST(Crypto, DynxxCryptoBase64Decode) {
    const auto decoded = dynxxCryptoBase64Decode(dynxxCodingStr2bytes("YWJj"), true);
    EXPECT_EQ(dynxxCodingBytes2str(decoded), "abc");
}

TEST(Crypto, Base64RoundTrip) {
    const auto in = dynxxCodingStr2bytes("base64-roundtrip");
    EXPECT_EQ(dynxxCryptoBase64Decode(dynxxCryptoBase64Encode(in, true), true), in);
}

TEST(Crypto, AesRoundTrip) {
    const auto in = dynxxCodingStr2bytes("aes-roundtrip");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    EXPECT_EQ(dynxxCryptoAesDecrypt(dynxxCryptoAesEncrypt(in, key), key), in);
}

TEST(Crypto, AesGcmRoundTrip) {
    const auto in = dynxxCodingStr2bytes("aes-gcm-roundtrip");
    const auto key = dynxxCodingStr2bytes("1234567890abcdef");
    const auto iv = dynxxCryptoRand(12);
    EXPECT_EQ(dynxxCryptoAesGcmDecrypt(dynxxCryptoAesGcmEncrypt(in, key, iv, 128), key, iv, 128), in);
}

TEST(Crypto, RsaRoundTrip) {
    const auto in = dynxxCodingStr2bytes("rsa-roundtrip");
    const auto publicKey = dynxxCodingStr2bytes(kRsaPublicKeyPem);
    const auto privateKey = dynxxCodingStr2bytes(kRsaPrivateKeyPem);
    const auto encrypted = dynxxCryptoRsaEncrypt(in, publicKey, DynXXCryptoRSAPaddingX::PKCS1);
    ASSERT_FALSE(encrypted.empty());
    const auto decrypted = dynxxCryptoRsaDecrypt(encrypted, privateKey, DynXXCryptoRSAPaddingX::PKCS1);
    EXPECT_EQ(decrypted, in);
}
