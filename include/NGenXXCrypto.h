#ifndef NGENXX_CRYPTO_H_
#define NGENXX_CRYPTO_H_

#include "NGenXXTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief AES Encrypt
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param keyBytes key Key bytes data
     * @param keyLen key bytes length（MUST BE 16!!）
     * @param outLen a pointer to read output bytes length
     * @return output bytes data（ALIGNED ON A 16 BYTE BOUNDARY!!）
     */
    const byte *ngenxx_crypto_aes_encrypt(const byte *inBytes, const size inLen, const byte *keyBytes, const size keyLen, size *outLen);

    /**
     * @brief AES Decrypt
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param keyBytes key Key bytes data
     * @param keyLen key bytes length（MUST BE 16!!）
     * @param outLen a pointer to read output bytes length
     * @return output bytes data（ALIGNED ON A 16 BYTE BOUNDARY!!）
     */
    const byte *ngenxx_crypto_aes_decrypt(const byte *inBytes, const size inLen, const byte *keyBytes, const size keyLen, size *outLen);

    /**
     * @brief MD5 hash
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param outLen a pointer to read output bytes length
     * @return output bytes data
     */
    const byte *ngenxx_crypto_hash_md5(const byte *inBytes, const size inLen, size *outLen);

    /**
     * @brief SHA256
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param outLen a pointer to read output bytes length
     * @return output bytes data
     */
    const byte *ngenxx_crypto_hash_sha256(const byte *inBytes, const size inLen, size *outLen);

    /**
     * @brief Base64 encode
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param outLen a pointer to read output bytes length
     * @return output bytes data
     */
    const byte *ngenxx_crypto_base64_encode(const byte *inBytes, const size inLen, size *outLen);

    /**
     * @brief Base64 decode
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param outLen a pointer to read output bytes length
     * @return output bytes data
     */
    const byte *ngenxx_crypto_base64_decode(const byte *inBytes, const size inLen, size *outLen);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_CRYPTO_H_