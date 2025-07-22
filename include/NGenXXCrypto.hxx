#ifndef NGENXX_INCLUDE_CRYPTO_HXX_
#define NGENXX_INCLUDE_CRYPTO_HXX_

#include "NGenXXTypes.hxx"

bool ngenxxCryptoRand(size_t len, byte *bytes);

Bytes ngenxxCryptoAesEncrypt(BytesView in, BytesView key);

Bytes ngenxxCryptoAesDecrypt(BytesView in, BytesView key);

Bytes ngenxxCryptoAesGcmEncrypt(BytesView in, BytesView key, BytesView initVector, size_t tagBits,
                                BytesView aad = {});

Bytes ngenxxCryptoAesGcmDecrypt(BytesView in, BytesView key, BytesView initVector, size_t tagBits,
                                BytesView aad = {});

enum class NGenXXCryptoRSAPaddingX : int {
    PKCS1 = 1,
    SSLV23 = 2,
    NONE = 3,
    PKCS1_OAEP = 4,
    X931 = 5,
    PKCS1_PSS = 6
};

std::string ngenxxCryptoRsaGenKey(std::string_view base64, bool isPublic);

Bytes ngenxxCryptoRsaEncrypt(BytesView in, BytesView key, NGenXXCryptoRSAPaddingX padding);

Bytes ngenxxCryptoRsaDecrypt(BytesView in, BytesView key, NGenXXCryptoRSAPaddingX padding);

Bytes ngenxxCryptoHashMd5(BytesView in);

Bytes ngenxxCryptoHashSha1(BytesView in);

Bytes ngenxxCryptoHashSha256(BytesView in);

Bytes ngenxxCryptoBase64Encode(BytesView in, bool noNewLines = true);

Bytes ngenxxCryptoBase64Decode(BytesView in, bool noNewLines = true);

#endif // NGENXX_INCLUDE_CRYPTO_HXX_
