#ifndef DYNXX_INCLUDE_CXX_CRYPTO_HXX_
#define DYNXX_INCLUDE_CXX_CRYPTO_HXX_

#include "Types.hxx"

Bytes dynxxCryptoRand(size_t len);

Bytes dynxxCryptoAesEncrypt(BytesView in, BytesView key);

Bytes dynxxCryptoAesDecrypt(BytesView in, BytesView key);

Bytes dynxxCryptoAesGcmEncrypt(BytesView in, BytesView key, BytesView initVector, size_t tagBits,
                                BytesView aad = {});

Bytes dynxxCryptoAesGcmDecrypt(BytesView in, BytesView key, BytesView initVector, size_t tagBits,
                                BytesView aad = {});

enum class DynXXCryptoRSAPaddingX : int {
    PKCS1 = 1,
    SSLV23 = 2,
    NONE = 3,
    PKCS1_OAEP = 4,
    X931 = 5,
    PKCS1_PSS = 6
};

std::string dynxxCryptoRsaGenKey(std::string_view base64, bool isPublic);

Bytes dynxxCryptoRsaEncrypt(BytesView in, BytesView key, DynXXCryptoRSAPaddingX padding);

Bytes dynxxCryptoRsaDecrypt(BytesView in, BytesView key, DynXXCryptoRSAPaddingX padding);

Bytes dynxxCryptoHashMd5(BytesView in);

Bytes dynxxCryptoHashSha1(BytesView in);

Bytes dynxxCryptoHashSha256(BytesView in);

Bytes dynxxCryptoBase64Encode(BytesView in, bool noNewLines = true);

Bytes dynxxCryptoBase64Decode(BytesView in, bool noNewLines = true);

#endif // DYNXX_INCLUDE_CXX_CRYPTO_HXX_
