#include "Crypto.hxx"
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"
#include <string>

#if defined(__OHOS__)
#include "../../../external/openssl/include/openssl/bio.h"
#include "../../../external/openssl/include/openssl/evp.h"
#include "../../../external/openssl/include/openssl/aes.h"
#include "../../../external/openssl/include/openssl/sha.h"
#include "../../../external/openssl/include/openssl/md5.h"
#else
#include "../../../external/wolfssl/wolfssl/wolfcrypt/aes.h"
#include "../../../external/wolfssl/wolfssl/wolfcrypt/md5.h"
#include "../../../external/wolfssl/wolfssl/wolfcrypt/sha.h"
#include "../../../external/wolfssl/wolfssl/wolfcrypt/sha256.h"
#endif

#define EMPTY_RESULT {NULL, 0}

/*
void NGenXX::Crypto::AES::aesgcmEncrypt(unsigned char *out,
                                                    const unsigned char *key, unsigned int keyLen,
                                                    const unsigned char *in, unsigned int inLen,
                                                    const unsigned char *initVector, unsigned int initVectorLen,
                                                    unsigned char *authTag, unsigned int authTagLen,
                                                    const unsigned char *authVector, unsigned int authVectorLen)
{
#if defined(__OHOS__)
    EVP_CIPHER_CTX *ctx;
    int outLen, tmpLen;
    BIO_dump_fp(stdout, (const char *)in, inLen);
    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, initVectorLen, NULL);
    EVP_EncryptInit_ex(ctx, NULL, NULL, key, initVector);
    EVP_EncryptUpdate(ctx, NULL, &outLen, authVector, authVectorLen);
    EVP_EncryptUpdate(ctx, out, &outLen, in, inLen);
    BIO_dump_fp(stdout, (const char *)out, outLen);
    EVP_EncryptFinal_ex(ctx, out, &outLen);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, 16, out);
    BIO_dump_fp(stdout, (const char *)out, 16);
    EVP_CIPHER_CTX_free(ctx);
#else
    Aes aes[1];
    wc_AesInit(aes, NULL, 0);
    ret = wc_AesGcmSetKey(aes, key, keyLen);
    wc_AesGcmEncrypt(aes, out, in, inLen, initVector, initVectorLen, authTag, authTagLen, authVector, authVectorLen);
#endif
}

void NGenXX::Crypto::AES::aesgcmDecrypt(unsigned char *out,
                                                    const unsigned char *key, unsigned int keyLen,
                                                    const unsigned char *in, unsigned int inLen,
                                                    const unsigned char *initVector, unsigned int initVectorLen,
                                                    unsigned char *authTag, unsigned int authTagLen,
                                                    const unsigned char *authVector, unsigned int authVectorLen)
{
#if defined(__OHOS__)
    EVP_CIPHER_CTX *ctx;
    int outLen, tmpLen, rv;
    BIO_dump_fp(stdout, (const char *)in, inLen);
    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, initVectorLen, NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, key, initVector);
    EVP_DecryptUpdate(ctx, NULL, &outLen, authVector, authVectorLen);
    EVP_DecryptUpdate(ctx, out, &outLen, in, inLen);
    BIO_dump_fp(stdout, (const char *)out, outLen);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, authTagLen, (void *)authTag);
    rv = EVP_DecryptFinal_ex(ctx, out, &outLen);
    EVP_CIPHER_CTX_free(ctx);
#else
    Aes aes[1];
    wc_AesInit(aes, NULL, 0);
    ret = wc_AesGcmSetKey(aes, key, keyLen);
    wc_AesGcmDecrypt(aes, out, in, inLen, initVector, initVectorLen, authTag, authTagLen, authVector, authVectorLen);
#endif
}*/

NGenXX::Crypto::Bytes NGenXX::Crypto::AES::aesEncrypt(NGenXX::Crypto::Bytes inBytes, NGenXX::Crypto::Bytes keyBytes)
{
    const unsigned char *in = std::get<0>(inBytes);
    const unsigned int inLen = std::get<1>(inBytes);
    const unsigned char *key = std::get<0>(keyBytes);
    const unsigned int keyLen = std::get<1>(keyBytes);
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE)
        return EMPTY_RESULT;
    // keyLen = AES_BLOCK_SIZE * 8;
    int outLen = inLen;
    if (inLen % AES_BLOCK_SIZE != 0)
    {
        outLen = (inLen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
    unsigned char fixedIn[outLen];
    memset(fixedIn, 0, outLen);
    memcpy(fixedIn, in, inLen);
    unsigned char out[outLen];
    memset(out, 0, outLen);
    int offset = 0;
#if defined(__OHOS__)
    AES_KEY aes_key;

    int ret = AES_set_encrypt_key(key, keyLen, &aes_key);
    if (ret != 0)
    {
        PRINT_ERROR("AES_set_encrypt_key error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    while (offset < inLen)
    {
        AES_encrypt(fixedIn + offset, out + offset, &aes_key);
        offset += AES_BLOCK_SIZE;
    }
#else
    Aes aes[1];

    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("AES_set_decrypt_key error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    ret = wc_AesSetKey(aes, key, keyLen, out, AES_ENCRYPTION);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("wc_AesSetKey_AES_ENCRYPTION error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    while (offset < inLen)
    {
        ret = wc_AesEncryptDirect(aes, out + offset, fixedIn + offset);
        if (ret != 0)
        {
            Log::print(NGenXXLogLevelError, ("wc_AesEncryptDirect error:" + std::to_string(ret)).c_str());
            return EMPTY_RESULT;
        }
        offset += AES_BLOCK_SIZE;
    }
#endif
    return {out, (const unsigned int)outLen};
}

NGenXX::Crypto::Bytes NGenXX::Crypto::AES::aesDecrypt(NGenXX::Crypto::Bytes inBytes, NGenXX::Crypto::Bytes keyBytes)
{
    const unsigned char *in = std::get<0>(inBytes);
    const unsigned int inLen = std::get<1>(inBytes);
    const unsigned char *key = std::get<0>(keyBytes);
    const unsigned int keyLen = std::get<1>(keyBytes);
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE)
        return EMPTY_RESULT;
    // keyLen = AES_BLOCK_SIZE * 8;
    int outLen = inLen;
    if (inLen % AES_BLOCK_SIZE != 0)
    {
        outLen = (inLen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
    unsigned char fixedIn[outLen];
    memset(fixedIn, 0, outLen);
    memcpy(fixedIn, in, inLen);
    unsigned char out[outLen];
    memset(out, 0, outLen);
    int offset = 0;
#if defined(__OHOS__)
    AES_KEY aes_key;

    int ret = AES_set_decrypt_key(key, keyLen, &aes_key);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("AES_set_decrypt_key error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    while (offset < inLen)
    {
        AES_decrypt(fixedIn + offset, out + offset, &aes_key);
        offset += AES_BLOCK_SIZE;
    }
#else
    Aes aes[1];

    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("wc_AesInit error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    ret = wc_AesSetKey(aes, key, keyLen, out, AES_DECRYPTION);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("wc_AesSetKey_AES_DECRYPTION error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    while (offset < inLen)
    {
        ret = wc_AesDecryptDirect(aes, out + offset, fixedIn + offset);
        if (ret != 0)
        {
            Log::print(NGenXXLogLevelError, ("wc_AesDecryptDirect error:" + std::to_string(ret)).c_str());
            return EMPTY_RESULT;
        }
        offset += AES_BLOCK_SIZE;
    }
#endif
    return {out, (const unsigned int)outLen};
}

NGenXX::Crypto::Bytes NGenXX::Crypto::Hash::md5(NGenXX::Crypto::Bytes inBytes)
{
    const unsigned char *in = std::get<0>(inBytes);
    const unsigned int inLen = std::get<1>(inBytes);
    if (in == NULL || inLen == 0)
        return EMPTY_RESULT;
    int outLen = 16;
    unsigned char out[outLen];
    memset(out, 0, outLen);
#if defined(__OHOS__)
    MD5_CTX md5;

    int ret = MD5_Init(&md5);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("MD5_Init error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    ret = MD5_Update(&md5, in, inLen);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("MD5_Update error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    ret = MD5_Final(out, &md5);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("MD5_Final error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }
#else
    wc_Md5 md5;

    int ret = wc_InitMd5(&md5);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("wc_InitMd5 error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    ret = wc_Md5Update(&md5, (byte *)in, inLen);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("wc_Md5Update error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    ret = wc_Md5Final(&md5, out);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("wc_Md5Final error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    wc_Md5Free(&md5);
#endif
    return {out, (const unsigned int)outLen};
}

NGenXX::Crypto::Bytes NGenXX::Crypto::Hash::sha256(NGenXX::Crypto::Bytes inBytes)
{
    const unsigned char *in = std::get<0>(inBytes);
    const unsigned int inLen = std::get<1>(inBytes);
    if (in == NULL || inLen == 0)
        return EMPTY_RESULT;
    int outLen = 32;
    unsigned char out[outLen];
    memset(out, 0, outLen);
#if defined(__OHOS__)
    SHA256_CTX sha256;

    int ret = SHA256_Init(&sha256);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("SHA256_Init error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    ret = SHA256_Update(&sha256, in, inLen);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("SHA256_Update error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    ret = SHA256_Final(out, &sha256);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("SHA256_Final error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }
#else
    wc_Sha256 sha256;

    int ret = wc_InitSha256(&sha256);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("wc_InitSha256 error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    wc_Sha256Update(&sha256, (byte *)in, inLen);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("wc_Sha256Update error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    wc_Sha256Final(&sha256, out);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, ("wc_Sha256Final error:" + std::to_string(ret)).c_str());
        return EMPTY_RESULT;
    }

    wc_Sha256Free(&sha256);
#endif
    return {out, (const unsigned int)outLen};
}