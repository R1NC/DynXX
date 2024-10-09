#ifndef NGENXX_CRYPTO_HXX_
#define NGENXX_CRYPTO_HXX_

#ifdef __cplusplus

#include "../NGenXX-Types.hxx"

#include <tuple>

namespace NGenXX
{
    namespace Crypto
    {
        bool rand(const size len, byte *bytes);

        namespace AES
        {

            const Bytes encrypt(const Bytes in, const Bytes key);

            const Bytes decrypt(const Bytes in, const Bytes key);

            static constexpr bool checkGcmParams(const Bytes in, const Bytes key, const Bytes initVector, const Bytes aad, const size tagBits)
            {
                const byte *inBytes = in.first;
                const size inLen = in.second;
                const byte *keyBytes = key.first;
                const size keyLen = key.second;
                const byte *inVectorBytes = initVector.first;
                const size inVectorLen = initVector.second;
                const byte *aadBytes = aad.first;
                const size aadLen = aad.second;
                const size tagLen = tagBits / 8;
                if (inBytes == NULL || inLen <= 0) return false;
                if (keyBytes == NULL || (keyLen != 16 && keyLen != 24 && keyLen != 32)) return false;
                if (inVectorBytes == NULL || inVectorLen != 12) return false;
                if (aadLen > 16 || (aadLen > 0 && aadBytes == NULL)) return false;
                if (tagBits != 96 && tagBits != 104 && tagBits != 112 && tagBits != 120 && tagBits != 128) return false;
                if (inLen <= tagLen) return false;
                return true;
            }

            const Bytes gcmEncrypt(const Bytes in, const Bytes key, const Bytes initVector, const Bytes aad, const size tagBits);

            const Bytes gcmDecrypt(const Bytes in, Bytes key, const Bytes initVector, const Bytes aad, const size tagBits);
        }

        namespace Hash
        {
            constexpr size MD5_BYTES_LEN = 16;
            constexpr size SHA256_BYTES_LEN = 32;

            const Bytes md5(const Bytes in);

            const Bytes sha256(const Bytes in);
        }

        namespace Base64
        {
            constexpr int calcDecodedLen(const Bytes in)
            {
                const byte *bytes = in.first;
                const size len = in.second;
                size_t padding = 0;
                if (bytes[len - 1] == '=' && bytes[len - 2] == '=')
                    padding = 2;
                else if (bytes[len - 1] == '=')
                    padding = 1;
                return (len * 3) / 4 - padding;
            }

            const Bytes encode(const Bytes in);

            const Bytes decode(const Bytes in);
        }
    }
}

#endif

#endif // NGENXX_CRYPTO_HXX_