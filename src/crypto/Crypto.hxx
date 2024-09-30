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
                const unsigned char *inBytes = std::get<0>(in);
                const unsigned int inLen = std::get<1>(in);
                const unsigned char *keyBytes = std::get<0>(key);
                const unsigned int keyLen = std::get<1>(key);
                const unsigned char *inVectorBytes = std::get<0>(initVector);
                const unsigned int inVectorLen = std::get<1>(initVector);
                const byte *aadBytes = std::get<0>(aad);
                const size aadLen = std::get<1>(aad);
                const unsigned int tagLen = tagBits / 8;
                if (inBytes == NULL || inLen <= 0) return false;
                if (keyBytes == NULL || (keyLen != 16 && keyLen != 24 && keyLen != 32)) return false;
                if (inVectorBytes == NULL || inVectorLen != 12) return false;
                if (aadLen > 0 && aadBytes == NULL) return false;
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
            constexpr int calcDecodedLen(const Bytes inBytes)
            {
                const byte *in = std::get<0>(inBytes);
                const size len = std::get<1>(inBytes);
                size_t padding = 0;
                if (in[len - 1] == '=' && in[len - 2] == '=')
                    padding = 2;
                else if (in[len - 1] == '=')
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