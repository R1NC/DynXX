#include "Crypto.hxx"
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"
#include <string>

#include "../../../external/wolfssl/wolfssl/wolfcrypt/aes.h"
#include "../../../external/wolfssl/wolfssl/wolfcrypt/md5.h"
#include "../../../external/wolfssl/wolfssl/wolfcrypt/sha.h"
#include "../../../external/wolfssl/wolfssl/wolfcrypt/sha256.h"

/*
void NGenXX::Crypto::AES::aesgcmEncrypt(unsigned char *out,
                                                    const unsigned char *key, unsigned int keyLen,
                                                    const unsigned char *in, unsigned int inLen,
                                                    const unsigned char *initVector, unsigned int initVectorLen,
                                                    unsigned char *authTag, unsigned int authTagLen,
                                                    const unsigned char *authVector, unsigned int authVectorLen)
{
    Aes aes[1];
    wc_AesInit(aes, NULL, 0);
    ret = wc_AesGcmSetKey(aes, key, keyLen);
    wc_AesGcmEncrypt(aes, out, in, inLen, initVector, initVectorLen, authTag, authTagLen, authVector, authVectorLen);
}

void NGenXX::Crypto::AES::aesgcmDecrypt(unsigned char *out,
                                                    const unsigned char *key, unsigned int keyLen,
                                                    const unsigned char *in, unsigned int inLen,
                                                    const unsigned char *initVector, unsigned int initVectorLen,
                                                    unsigned char *authTag, unsigned int authTagLen,
                                                    const unsigned char *authVector, unsigned int authVectorLen)
{
    Aes aes[1];
    wc_AesInit(aes, NULL, 0);
    ret = wc_AesGcmSetKey(aes, key, keyLen);
    wc_AesGcmDecrypt(aes, out, in, inLen, initVector, initVectorLen, authTag, authTagLen, authVector, authVectorLen);
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

    Aes aes[1];

    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "AES_set_decrypt_key error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    ret = wc_AesSetKey(aes, key, keyLen, out, AES_ENCRYPTION);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "wc_AesSetKey_AES_ENCRYPTION error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    while (offset < inLen)
    {
        ret = wc_AesEncryptDirect(aes, out + offset, fixedIn + offset);
        if (ret != 0)
        {
            Log::print(NGenXXLogLevelError, "wc_AesEncryptDirect error:" + std::to_string(ret));
            return EMPTY_RESULT;
        }
        offset += AES_BLOCK_SIZE;
    }

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

    Aes aes[1];

    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "wc_AesInit error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    ret = wc_AesSetKey(aes, key, keyLen, out, AES_DECRYPTION);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "wc_AesSetKey_AES_DECRYPTION error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    while (offset < inLen)
    {
        ret = wc_AesDecryptDirect(aes, out + offset, fixedIn + offset);
        if (ret != 0)
        {
            Log::print(NGenXXLogLevelError, "wc_AesDecryptDirect error:" + std::to_string(ret));
            return EMPTY_RESULT;
        }
        offset += AES_BLOCK_SIZE;
    }

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

    wc_Md5 md5;

    int ret = wc_InitMd5(&md5);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "wc_InitMd5 error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    ret = wc_Md5Update(&md5, (byte *)in, inLen);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "wc_Md5Update error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    ret = wc_Md5Final(&md5, out);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "wc_Md5Final error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    wc_Md5Free(&md5);

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

    wc_Sha256 sha256;

    int ret = wc_InitSha256(&sha256);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "wc_InitSha256 error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    wc_Sha256Update(&sha256, (byte *)in, inLen);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "wc_Sha256Update error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    wc_Sha256Final(&sha256, out);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "wc_Sha256Final error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    wc_Sha256Free(&sha256);

    return {out, (const unsigned int)outLen};
}