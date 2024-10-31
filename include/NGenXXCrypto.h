#ifndef NGENXX_INCLUDE_CRYPTO_H_
#define NGENXX_INCLUDE_CRYPTO_H_

#include "NGenXXTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Generate random bytes
     * @param len bytes length
     * @param bytes A pointer to store the output bytes
     * @return success or not
     */
    bool ngenxx_crypto_rand(const size_t len, byte *bytes);

    /**
     * @brief AES Encrypt
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param keyBytes key Key bytes data
     * @param keyLen key bytes length（MUST BE 16!!）
     * @param outLen a pointer to read output bytes length
     * @return output bytes data（ALIGNED ON A 16 BYTE BOUNDARY!!）
     */
    const byte *ngenxx_crypto_aes_encrypt(const byte *inBytes, const size_t inLen, const byte *keyBytes, const size_t keyLen, size_t *outLen);

    /**
     * @brief AES Decrypt
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param keyBytes key Key bytes data
     * @param keyLen key bytes length（MUST BE 16!!）
     * @param outLen a pointer to read output bytes length
     * @return output bytes data（ALIGNED ON A 16 BYTE BOUNDARY!!）
     */
    const byte *ngenxx_crypto_aes_decrypt(const byte *inBytes, const size_t inLen, const byte *keyBytes, const size_t keyLen, size_t *outLen);

    /**
     * @brief AES GCM Encrypt
     * @param inBytes bytes data to be encrypt
     * @param inLen bytes length to be encrypt
     * @param keyBytes key bytes data
     * @param keyLen key bytes length（MUST BE 16/24/32!!）
     * @param initVectorBytes initVector bytes data
     * @param initVectorLen initVector bytes length（MUST BE 12!!）
     * @param tagBits tag bits length（MUST BE 96/104/112/120/128!!）
     * @param aadBytes Additional Authentication Data (AAD) bytes（Can be `NULL`）
     * @param aadLen Additional Authentication Data (AAD) length（Must <= 16）
     * @param outLen output bytes length（include the tag）
     * @return output bytes data（With a tag on tail）
     */
    const byte *ngenxx_crypto_aes_gcm_encrypt(const byte *inBytes, const size_t inLen,
                                              const byte *keyBytes, const size_t keyLen,
                                              const byte *initVectorBytes, const size_t initVectorLen,
                                              const byte *aadBytes, const size_t aadLen,
                                              const size_t tagBits, size_t *outLen);

    /**
     * @brief AES GCM Decrypt
     * @param inBytes bytes data to be decrypt（with a tag on tail）
     * @param inLen bytes length to be decrypt（include the tag）
     * @param keyBytes key bytes data
     * @param keyLen key bytes length（MUST BE 16/24/32!!）
     * @param initVectorBytes initVector bytes data
     * @param initVectorLen initVector bytes length（MUST BE 12!!）
     * @param aadBytes Additional Authentication Data (AAD) bytes（Can be `NULL`）
     * @param aadLen Additional Authentication Data (AAD) length（Must <= 16）
     * @param tagBits tag bits length（MUST BE 96//104/112/120/128!!）
     * @param outLen output bytes length（Not include the tag）
     * @return output bytes data（No tag on tail）
     */
    const byte *ngenxx_crypto_aes_gcm_decrypt(const byte *inBytes, const size_t inLen,
                                              const byte *keyBytes, const size_t keyLen,
                                              const byte *initVectorBytes, const size_t initVectorLen,
                                              const byte *aadBytes, const size_t aadLen,
                                              const size_t tagBits, size_t *outLen);

    /**
     * @brief MD5 hash
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param outLen a pointer to read output bytes length
     * @return output bytes data
     */
    const byte *ngenxx_crypto_hash_md5(const byte *inBytes, const size_t inLen, size_t *outLen);

    /**
     * @brief SHA256
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param outLen a pointer to read output bytes length
     * @return output bytes data
     */
    const byte *ngenxx_crypto_hash_sha256(const byte *inBytes, const size_t inLen, size_t *outLen);

    /**
     * @brief Base64 encode
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param outLen a pointer to read output bytes length
     * @return output bytes data
     */
    const byte *ngenxx_crypto_base64_encode(const byte *inBytes, const size_t inLen, size_t *outLen);

    /**
     * @brief Base64 decode
     * @param inBytes input bytes data
     * @param inLen input bytes length
     * @param outLen a pointer to read output bytes length
     * @return output bytes data
     */
    const byte *ngenxx_crypto_base64_decode(const byte *inBytes, const size_t inLen, size_t *outLen);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_INCLUDE_CRYPTO_H_