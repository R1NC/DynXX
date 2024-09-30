#include "Crypto.hxx"
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"
#include <string>

#include "../../../external/wolfssl/wolfssl/ssl.h"
#include "../../../external/wolfssl/wolfssl/wolfcrypt/aes.h"
#include "../../../external/wolfssl/wolfssl/wolfcrypt/md5.h"
#include "../../../external/wolfssl/wolfssl/wolfcrypt/sha.h"
#include "../../../external/wolfssl/wolfssl/wolfcrypt/sha256.h"
#include "../../../external/wolfssl/wolfssl/wolfcrypt/coding.h"

constexpr int WolfSSL_OK = 0;

bool NGenXX::Crypto::rand(const size len, byte *bytes)
{
    if (len <= 0 || bytes == NULL) return false;
    int ret = wolfSSL_RAND_bytes(bytes, len);
    return ret == WOLFSSL_SUCCESS;
}

const NGenXX::Bytes NGenXX::Crypto::AES::encrypt(const NGenXX::Bytes inBytes, const NGenXX::Bytes keyBytes)
{
    const byte *in = inBytes.first;
    const size inLen = inBytes.second;
    const byte *key = keyBytes.first;
    const size keyLen = keyBytes.second;
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE)
        return BytesEmpty;
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

    Aes aes[1];

    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "AES_set_decrypt_key error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = wc_AesSetKey(aes, key, keyLen, out, AES_ENCRYPTION);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_AesSetKey_AES_ENCRYPTION error:" + std::to_string(ret));
        return BytesEmpty;
    }

    while (offset < inLen)
    {
        ret = wc_AesEncryptDirect(aes, out + offset, fixedIn + offset);
        if (ret != WolfSSL_OK)
        {
            Log::print(NGenXXLogLevelError, "wc_AesEncryptDirect error:" + std::to_string(ret));
            return BytesEmpty;
        }
        offset += AES_BLOCK_SIZE;
    }

    wc_AesFree(aes);

    return {out, (const size)outLen};
}

const NGenXX::Bytes NGenXX::Crypto::AES::decrypt(const NGenXX::Bytes inBytes, const NGenXX::Bytes keyBytes)
{
    const byte *in = inBytes.first;
    const size inLen = inBytes.second;
    const byte *key = keyBytes.first;
    const size keyLen = keyBytes.second;
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE)
        return BytesEmpty;
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

    Aes aes[1];

    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_AesInit error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = wc_AesSetKey(aes, key, keyLen, out, AES_DECRYPTION);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_AesSetKey_AES_DECRYPTION error:" + std::to_string(ret));
        return BytesEmpty;
    }

    while (offset < inLen)
    {
        ret = wc_AesDecryptDirect(aes, out + offset, fixedIn + offset);
        if (ret != WolfSSL_OK)
        {
            Log::print(NGenXXLogLevelError, "wc_AesDecryptDirect error:" + std::to_string(ret));
            return BytesEmpty;
        }
        offset += AES_BLOCK_SIZE;
    }

    wc_AesFree(aes);

    return {out, (const size)outLen};
}

const NGenXX::Bytes NGenXX::Crypto::AES::gcmEncrypt(const NGenXX::Bytes inBytes, const NGenXX::Bytes keyBytes, const NGenXX::Bytes initVectorBytes, const NGenXX::Bytes aadBytes, const size tagBits)
{
    if (!NGenXX::Crypto::AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) return BytesEmpty;
    const byte *in = inBytes.first;
    size inLen = inBytes.second;
    const byte *key = keyBytes.first;
    const size keyLen = keyBytes.second;
    const byte *initVector = initVectorBytes.first;
    const size initVectorLen = initVectorBytes.second;
    const byte *aad = aadBytes.first;
    const size aadLen = aadBytes.second;
    const size tagLen = tagBits / 8;

    unsigned char tag[tagLen];
    std::memset(tag, 0, tagLen);
    size outLen = inLen + tagLen;
    unsigned char out[outLen];
    std::memset(out, 0, outLen);
    
    Aes aes[1];
    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_AesInit error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = wc_AesGcmSetKey(aes, key, keyLen);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_AesGcmSetKey error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = wc_AesGcmEncrypt(aes, out, in, inLen, initVector, initVectorLen, tag, tagLen, aad, aadLen);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_AesGcmEncrypt error:" + std::to_string(ret));
        return BytesEmpty;
    }
    std::memcpy(out + inLen, tag, tagLen);

    wc_AesFree(aes);
    
    return {out, outLen};
}

const NGenXX::Bytes NGenXX::Crypto::AES::gcmDecrypt(const NGenXX::Bytes inBytes, const NGenXX::Bytes keyBytes, const NGenXX::Bytes initVectorBytes, const NGenXX::Bytes aadBytes, const size tagBits)
{
    if (!NGenXX::Crypto::AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) return BytesEmpty;
    const byte *in = inBytes.first;
    size inLen = inBytes.second;
    const byte *key = keyBytes.first;
    const size keyLen = keyBytes.second;
    const byte *initVector = initVectorBytes.first;
    const size initVectorLen = initVectorBytes.second;
    const byte *aad = aadBytes.first;
    const size aadLen = aadBytes.second;
    const size tagLen = tagBits / 8;

    inLen -= tagLen;
    unsigned char tag[tagLen];
    std::memcpy(tag, in + inLen, tagLen);
    size outLen = inLen;
    unsigned char out[outLen];
    memset(out, 0, outLen);

    Aes aes[1];
    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_AesInit error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = wc_AesGcmSetKey(aes, key, keyLen);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_AesGcmSetKey error:" + std::to_string(ret));
        return BytesEmpty;
    }
    
    ret = wc_AesGcmDecrypt(aes, out, in, inLen, initVector, initVectorLen, tag, tagLen, aad, aadLen);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_AesGcmDecrypt error:" + std::to_string(ret));
        return BytesEmpty;
    }

    wc_AesFree(aes);

    return {out, outLen};
}

const NGenXX::Bytes NGenXX::Crypto::Hash::md5(const NGenXX::Bytes inBytes)
{
    const byte *in = inBytes.first;
    const size inLen = inBytes.second;
    if (in == NULL || inLen == 0)
        return BytesEmpty;
    int outLen = MD5_BYTES_LEN;
    byte out[outLen];
    memset(out, 0, outLen);

    wc_Md5 md5;

    int ret = wc_InitMd5(&md5);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_InitMd5 error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = wc_Md5Update(&md5, (byte *)in, inLen);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_Md5Update error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = wc_Md5Final(&md5, out);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_Md5Final error:" + std::to_string(ret));
        return BytesEmpty;
    }

    wc_Md5Free(&md5);

    return {out, (const size)outLen};
}

const NGenXX::Bytes NGenXX::Crypto::Hash::sha256(const NGenXX::Bytes inBytes)
{
    const byte *in = inBytes.first;
    const size inLen = inBytes.second;
    if (in == NULL || inLen == 0)
        return BytesEmpty;
    int outLen = SHA256_BYTES_LEN;
    byte out[outLen];
    memset(out, 0, outLen);

    wc_Sha256 sha256;

    int ret = wc_InitSha256(&sha256);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_InitSha256 error:" + std::to_string(ret));
        return BytesEmpty;
    }

    wc_Sha256Update(&sha256, (byte *)in, inLen);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_Sha256Update error:" + std::to_string(ret));
        return BytesEmpty;
    }

    wc_Sha256Final(&sha256, out);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "wc_Sha256Final error:" + std::to_string(ret));
        return BytesEmpty;
    }

    wc_Sha256Free(&sha256);

    return {out, (const size)outLen};
}

const NGenXX::Bytes NGenXX::Crypto::Base64::encode(const NGenXX::Bytes inBytes)
{
    const byte *in = inBytes.first;
    const size inLen = inBytes.second;
    if (in == NULL || inLen == 0)
        return BytesEmpty;

    unsigned int outLen = 0;
    int ret = Base64_Encode_NoNl(in, inLen, NULL, &outLen);

    byte outBuffer[outLen];
    memset(outBuffer, 0, outLen);

    ret = Base64_Encode_NoNl(in, inLen, outBuffer, &outLen);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "Base64_Encode error:" + std::to_string(ret));
        return BytesEmpty;
    }

    return {outBuffer, outLen};
}

const NGenXX::Bytes NGenXX::Crypto::Base64::decode(const NGenXX::Bytes inBytes)
{
    const byte *in = inBytes.first;
    const size inLen = inBytes.second;
    if (in == NULL || inLen == 0)
        return BytesEmpty;

    unsigned int outLen = calcDecodedLen(inBytes);
    byte outBuffer[outLen];
    memset(outBuffer, 0, outLen);

    int ret = Base64_Decode(in, inLen, outBuffer, &outLen);
    if (ret != WolfSSL_OK)
    {
        Log::print(NGenXXLogLevelError, "Base64_Decode error:" + std::to_string(ret));
        return BytesEmpty;
    }

    return {outBuffer, outLen};
}