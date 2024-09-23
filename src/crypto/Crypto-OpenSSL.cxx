#include "Crypto.hxx"
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"
#include <string>

#include "../../../external/openssl/include/openssl/bio.h"
#include "../../../external/openssl/include/openssl/evp.h"
#include "../../../external/openssl/include/openssl/aes.h"
#include "../../../external/openssl/include/openssl/sha.h"
#include "../../../external/openssl/include/openssl/md5.h"

constexpr int OpenSSL_OK = 1;
constexpr int OpenSSL_AES_Key_BITS = 128;

/*
void NGenXX::Crypto::AES::aesgcmEncrypt(byte *out,
                                                    const byte *key, size keyLen,
                                                    const byte *in, size inLen,
                                                    const byte *initVector, size initVectorLen,
                                                    byte *authTag, size authTagLen,
                                                    const byte *authVector, size authVectorLen)
{
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
}

void NGenXX::Crypto::AES::aesgcmDecrypt(byte *out,
                                                    const byte *key, size keyLen,
                                                    const byte *in, size inLen,
                                                    const byte *initVector, size initVectorLen,
                                                    byte *authTag, size authTagLen,
                                                    const byte *authVector, size authVectorLen)
{
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
}*/

NGenXX::Crypto::Bytes NGenXX::Crypto::AES::aesEncrypt(NGenXX::Crypto::Bytes inBytes, NGenXX::Crypto::Bytes keyBytes)
{
    const byte *in = std::get<0>(inBytes);
    const size inLen = std::get<1>(inBytes);
    const byte *key = std::get<0>(keyBytes);
    const size keyLen = std::get<1>(keyBytes);
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE)
        return EMPTY_RESULT;
    // keyLen = AES_BLOCK_SIZE * 8;
    int outLen = inLen;
    if (inLen % AES_BLOCK_SIZE != 0)
    {
        outLen = (inLen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
    byte fixedIn[outLen];
    memset(fixedIn, 0, outLen);
    memcpy(fixedIn, in, inLen);
    byte out[outLen];
    memset(out, 0, outLen);
    int offset = 0;

    AES_KEY aes_key;

    int ret = AES_set_encrypt_key(key, OpenSSL_AES_Key_BITS, &aes_key);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "AES_set_encrypt_key error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    while (offset < inLen)
    {
        AES_encrypt(fixedIn + offset, out + offset, &aes_key);
        offset += AES_BLOCK_SIZE;
    }

    return {out, (const size)outLen};
}

NGenXX::Crypto::Bytes NGenXX::Crypto::AES::aesDecrypt(NGenXX::Crypto::Bytes inBytes, NGenXX::Crypto::Bytes keyBytes)
{
    const byte *in = std::get<0>(inBytes);
    const size inLen = std::get<1>(inBytes);
    const byte *key = std::get<0>(keyBytes);
    const size keyLen = std::get<1>(keyBytes);
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE)
        return EMPTY_RESULT;
    // keyLen = AES_BLOCK_SIZE * 8;
    int outLen = inLen;
    if (inLen % AES_BLOCK_SIZE != 0)
    {
        outLen = (inLen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
    byte fixedIn[outLen];
    memset(fixedIn, 0, outLen);
    memcpy(fixedIn, in, inLen);
    byte out[outLen];
    memset(out, 0, outLen);
    int offset = 0;

    AES_KEY aes_key;

    int ret = AES_set_decrypt_key(key, OpenSSL_AES_Key_BITS, &aes_key);
    if (ret != 0)
    {
        Log::print(NGenXXLogLevelError, "AES_set_decrypt_key error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    while (offset < inLen)
    {
        AES_decrypt(fixedIn + offset, out + offset, &aes_key);
        offset += AES_BLOCK_SIZE;
    }

    return {out, (const size)outLen};
}

NGenXX::Crypto::Bytes NGenXX::Crypto::Hash::md5(NGenXX::Crypto::Bytes inBytes)
{
    const byte *in = std::get<0>(inBytes);
    const size inLen = std::get<1>(inBytes);
    if (in == NULL || inLen == 0)
        return EMPTY_RESULT;
    int outLen = 16;
    byte out[outLen];
    memset(out, 0, outLen);

    MD5_CTX md5;

    int ret = MD5_Init(&md5);
    if (ret != OpenSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "MD5_Init error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    ret = MD5_Update(&md5, in, inLen);
    if (ret != OpenSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "MD5_Update error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    ret = MD5_Final(out, &md5);
    if (ret != OpenSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "MD5_Final error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    return {out, (const size)outLen};
}

NGenXX::Crypto::Bytes NGenXX::Crypto::Hash::sha256(NGenXX::Crypto::Bytes inBytes)
{
    const byte *in = std::get<0>(inBytes);
    const size inLen = std::get<1>(inBytes);
    if (in == NULL || inLen == 0)
        return EMPTY_RESULT;
    int outLen = 32;
    byte out[outLen];
    memset(out, 0, outLen);

    SHA256_CTX sha256;

    int ret = SHA256_Init(&sha256);
    if (ret != OpenSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "SHA256_Init error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    ret = SHA256_Update(&sha256, in, inLen);
    if (ret != OpenSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "SHA256_Update error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    ret = SHA256_Final(out, &sha256);
    if (ret != OpenSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "SHA256_Final error:" + std::to_string(ret));
        return EMPTY_RESULT;
    }

    return {out, (const size)outLen};
}