#ifndef NGENXX_SRC_CRYPTO_HXX_
#define NGENXX_SRC_CRYPTO_HXX_

#if defined(__cplusplus)

#include <NGenXXTypes.hxx>

namespace NGenXX
{
    namespace Crypto
    {
        bool rand(size_t len, byte *bytes);

        namespace AES
        {

            Bytes encrypt(const Bytes &in, const Bytes &key);

            Bytes decrypt(const Bytes &in, const Bytes &key);

            static inline constexpr bool checkGcmParams(const Bytes &in, const Bytes &key, const Bytes &initVector, const Bytes &aad, size_t tagBits)
            {
                auto inBytes = in.data(), keyBytes = key.data(), initVectorBytes = initVector.data(), aadBytes = aad.data();
                auto inLen = in.size(), keyLen = key.size(), initVectorLen = initVector.size(), aadLen = aad.size();
                auto tagLen = tagBits / 8;
                if (inBytes == nullptr || inLen == 0)
                {
                    return false;
                }
                if (keyBytes == nullptr || (keyLen != 16 && keyLen != 24 && keyLen != 32))
                {
                    return false;
                }
                if (initVectorBytes == nullptr || initVectorLen != 12)
                {
                    return false;
                }
                if (aadLen > 16 || (aadLen > 0 && aadBytes == nullptr))
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

            Bytes gcmEncrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, const Bytes &aad, size_t tagBits);

            Bytes gcmDecrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, const Bytes &aad, size_t tagBits);
        }

        namespace RSA
        {
            Bytes encrypt(const Bytes &in, const Bytes &key, int padding);
            
            Bytes decrypt(const Bytes &in, const Bytes &key, int padding);
        }

        namespace Hash
        {
            Bytes md5(const Bytes &in);

            Bytes sha1(const Bytes &in);
            Bytes sha256(const Bytes &in);
        }

        namespace Base64
        {
            Bytes encode(const Bytes &in);

            Bytes decode(const Bytes &in);
        }
    }
}

#endif

#endif // NGENXX_SRC_CRYPTO_HXX_