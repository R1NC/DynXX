#ifndef NGENXX_CRYPTO_H_
#define NGENXX_CRYPTO_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief AES Encrypt
     * @param in input bytes data
     * @param inLen input bytes length
     * @param key key Key bytes data
     * @param keyLen key bytes length（MUST BE 16!!）
     * @param outLen a pointer to read output bytes length
     * @return output bytes data（ALIGNED ON A 16 BYTE BOUNDARY!!）
     */
    const unsigned char *ngenxx_crypto_aes_encrypt(const unsigned char *in, const unsigned int inLen,
                                                              const unsigned char *key, const unsigned int keyLen, unsigned int *outLen);

    /**
     * @brief AES Decrypt
     * @param in input bytes data
     * @param inLen input bytes length
     * @param key key Key bytes data
     * @param keyLen key bytes length（MUST BE 16!!）
     * @param outLen a pointer to read output bytes length
     * @return output bytes data（ALIGNED ON A 16 BYTE BOUNDARY!!）
     */
    const unsigned char *ngenxx_crypto_aes_decrypt(const unsigned char *in, const unsigned int inLen,
                                                              const unsigned char *key, const unsigned int keyLen, unsigned int *outLen);

    /**
     * @brief MD5 hash
     * @param in input bytes data
     * @param inLen input bytes length
     * @param outLen a pointer to read output bytes length
     * @return output bytes data
     */
    const unsigned char *ngenxx_crypto_hash_md5(const unsigned char *in, const unsigned int inLen, unsigned int *outLen);

    /**
     * @brief SHA256
     * @param in input bytes data
     * @param inLen input bytes length
     * @param outLen a pointer to read output bytes length
     * @return output bytes data
     */
    const unsigned char *ngenxx_crypto_hash_sha256(const unsigned char *in, const unsigned int inLen, unsigned int *outLen);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_CRYPTO_H_