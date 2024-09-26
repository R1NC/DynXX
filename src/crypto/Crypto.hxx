#ifndef NGENXX_CRYPTO_HXX_
#define NGENXX_CRYPTO_HXX_

#ifdef __cplusplus

#include "../NGenXX-Types.hxx"

#include <tuple>

namespace NGenXX
{
    namespace Crypto
    {
        namespace AES
        {
            const Bytes aesEncrypt(const Bytes in, const Bytes key);

            const Bytes aesDecrypt(const Bytes in, const Bytes key);
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