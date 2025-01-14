#ifndef NGENXX_SRC_CRYPTO_HXX_
#define NGENXX_SRC_CRYPTO_HXX_

#if defined(__cplusplus)

#include <NGenXXTypes.hxx>

namespace NGenXX
{
    namespace Crypto
    {
        bool rand(const size_t len, byte *bytes);

        namespace AES
        {

            const Bytes encrypt(const Bytes &in, const Bytes &key);

            const Bytes decrypt(const Bytes &in, const Bytes &key);

            static constexpr bool checkGcmParams(const Bytes &in, const Bytes &key, const Bytes &initVector, const Bytes &aad, const size_t tagBits)
            {
                auto inBytes = in.data(), keyBytes = key.data(), initVectorBytes = initVector.data(), aadBytes = aad.data();
                auto inLen = in.size(), keyLen = key.size(), initVectorLen = initVector.size(), aadLen = aad.size();
                auto tagLen = tagBits / 8;
                if (inBytes == NULL || inLen <= 0)
                {
                    return false;
                }
                if (keyBytes == NULL || (keyLen != 16 && keyLen != 24 && keyLen != 32))
                {
                    return false;
                }
                if (initVectorBytes == NULL || initVectorLen != 12)
                {
                    return false;
                }
                if (aadLen > 16 || (aadLen > 0 && aadBytes == NULL))
                {
                    return false;
                }
                if (tagBits != 96 && tagBits != 104 && tagBits != 112 && tagBits != 120 && tagBits != 128)
                {
                    return false;
                }
                if (inLen <= tagLen)
                {
                    return false;
                }
                return true;
            }

            const Bytes gcmEncrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, const Bytes &aad, const size_t tagBits);

            const Bytes gcmDecrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, const Bytes &aad, const size_t tagBits);
        }

        namespace Hash
        {
            constexpr size_t MD5_BYTES_LEN = 16;
            constexpr size_t SHA256_BYTES_LEN = 32;

            const Bytes md5(const Bytes &in);

            const Bytes sha256(const Bytes &in);
        }

        namespace Base64
        {
            const Bytes encode(const Bytes &in);

            const Bytes decode(const Bytes &in);
        }
    }
}

#endif

#endif // NGENXX_SRC_CRYPTO_HXX_