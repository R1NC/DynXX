#ifndef NGENXX_CRYPTO_HXX_
#define NGENXX_CRYPTO_HXX_

#include "../../include/NGenXXTypes.h"

#ifdef __cplusplus

#include <tuple>

namespace NGenXX
{
    namespace Crypto
    {
#define EMPTY_RESULT {NULL, 0}

        typedef std::tuple<const byte *, const size> Bytes;

        namespace AES
        {

            Bytes aesEncrypt(Bytes in, Bytes key);

            Bytes aesDecrypt(Bytes in, Bytes key);
        }

        namespace Hash
        {
            Bytes md5(Bytes in);

            Bytes sha256(Bytes in);
        }
    }
}

#endif

#endif // NGENXX_CRYPTO_HXX_