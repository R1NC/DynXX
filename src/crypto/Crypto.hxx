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
            const NGenXX::Bytes aesEncrypt(const NGenXX::Bytes in, const NGenXX::Bytes key);

            const NGenXX::Bytes aesDecrypt(const NGenXX::Bytes in, const NGenXX::Bytes key);
        }

        namespace Hash
        {
            const NGenXX::Bytes md5(const NGenXX::Bytes in);

            const NGenXX::Bytes sha256(const NGenXX::Bytes in);
        }

        namespace Base64
        {
            static constexpr int calcDecodedLen(const NGenXX::Bytes inBytes)
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

            const NGenXX::Bytes encode(const NGenXX::Bytes in);

            const NGenXX::Bytes decode(const NGenXX::Bytes in);
        }
    }
}

#endif

#endif // NGENXX_CRYPTO_HXX_