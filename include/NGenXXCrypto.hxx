#ifndef NGENXX_INCLUDE_CRYPTO_HXX_
#define NGENXX_INCLUDE_CRYPTO_HXX_

#include "NGenXXTypes.hxx"

bool ngenxxCryptoRand(size_t len, byte *bytes);

Bytes ngenxxCryptoAesEncrypt(const Bytes &in, const Bytes &key);

Bytes ngenxxCryptoAesDecrypt(const Bytes &in, const Bytes &key);

Bytes ngenxxCryptoAesGcmEncrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, size_t tagBits,
                                      const Bytes &aad = {});

Bytes ngenxxCryptoAesGcmDecrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, size_t tagBits,
                                      const Bytes &aad = {});

enum class NGenXXCryptoRSAPaddingX : int
{
    PKCS1 = 1,
    SSLV23 = 2,
    NONE = 3,
    PKCS1_OAEP = 4,
    X931 = 5,
    PKCS1_PSS = 6
};

Bytes ngenxxCryptoRsaEncrypt(const Bytes &in, const Bytes &key, const NGenXXCryptoRSAPaddingX padding);

Bytes ngenxxCryptoRsaDecrypt(const Bytes &in, const Bytes &key, const NGenXXCryptoRSAPaddingX padding);

Bytes ngenxxCryptoHashMd5(const Bytes &in);

Bytes ngenxxCryptoHashSha1(const Bytes &in);
Bytes ngenxxCryptoHashSha256(const Bytes &in);

Bytes ngenxxCryptoBase64Encode(const Bytes &in);

Bytes ngenxxCryptoBase64Decode(const Bytes &in);

#endif // NGENXX_INCLUDE_CRYPTO_HXX_