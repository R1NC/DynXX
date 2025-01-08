#if defined(USE_WOLF_SSL)
#include "Crypto.hxx"
#include <NGenXXLog.hxx>
#include "../util/TypeUtil.hxx"
#include <string>
#include <cstring>

#include <wolfssl/wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/md5.h>
#include <wolfssl/wolfcrypt/sha.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/coding.h>

constexpr int WolfSSL_OK = 0;

bool NGenXX::Crypto::rand(const size_t len, byte *bytes)
{
    if (len <= 0 || bytes == NULL)
    {
        return false;
    }
    int ret = wolfSSL_RAND_bytes(bytes, len);
    return ret == WOLFSSL_SUCCESS;
}

const Bytes NGenXX::Crypto::AES::encrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    auto in = inBytes.data(), key = keyBytes.data();
    auto inLen = inBytes.size(), keyLen = keyBytes.size();
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE)
    {
        return BytesEmpty;
    }
    // keyLen = AES_BLOCK_SIZE * 8;
    int outLen = inLen;
    if (inLen % AES_BLOCK_SIZE != 0)
    {
        outLen = (inLen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
    byte fixedIn[outLen];
    std::memset(fixedIn, 0, outLen);
    std::memcpy(fixedIn, in, inLen);
    byte out[outLen];
    std::memset(out, 0, outLen);
    int offset = 0;

    Aes aes[1];

    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "AES_set_decrypt_key error:{}", ret);
        return BytesEmpty;
    }

    ret = wc_AesSetKey(aes, key, keyLen, out, AES_ENCRYPTION);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesSetKey_AES_ENCRYPTION error:{}", ret);
        return BytesEmpty;
    }

    while (offset < inLen)
    {
        ret = wc_AesEncryptDirect(aes, out + offset, fixedIn + offset);
        if (ret != WolfSSL_OK)
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesEncryptDirect error:{}", ret);
            return BytesEmpty;
        }
        offset += AES_BLOCK_SIZE;
    }

    wc_AesFree(aes);

    return wrapBytes(out, outLen);
}

const Bytes NGenXX::Crypto::AES::decrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    auto in = inBytes.data(), key = keyBytes.data();
    auto inLen = inBytes.size(), keyLen = keyBytes.size();
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE)
    {
        return BytesEmpty;
    }
    // keyLen = AES_BLOCK_SIZE * 8;
    int outLen = inLen;
    if (inLen % AES_BLOCK_SIZE != 0)
    {
        outLen = (inLen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
    byte fixedIn[outLen];
    std::memset(fixedIn, 0, outLen);
    std::memcpy(fixedIn, in, inLen);
    byte out[outLen];
    std::memset(out, 0, outLen);
    int offset = 0;

    Aes aes[1];

    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesInit error:{}", ret);
        return BytesEmpty;
    }

    ret = wc_AesSetKey(aes, key, keyLen, out, AES_DECRYPTION);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesSetKey_AES_DECRYPTION error:{}", ret);
        return BytesEmpty;
    }

    while (offset < inLen)
    {
        ret = wc_AesDecryptDirect(aes, out + offset, fixedIn + offset);
        if (ret != WolfSSL_OK)
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesDecryptDirect error:{}", ret);
            return BytesEmpty;
        }
        offset += AES_BLOCK_SIZE;
    }

    wc_AesFree(aes);

    return trimBytes(wrapBytes(out, outLen));
}

const Bytes NGenXX::Crypto::AES::gcmEncrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, const size_t tagBits)
{
    if (!NGenXX::Crypto::AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits))
    {
        return BytesEmpty;
    }
    auto in = inBytes.data(), key = keyBytes.data(), initVector = initVectorBytes.data(), aad = aadBytes.data();
    auto inLen = inBytes.size(), keyLen = keyBytes.size(), initVectorLen = initVectorBytes.size(), aadLen = aadBytes.size();
    const size_t tagLen = tagBits / 8;

    byte tag[tagLen];
    std::memset(tag, 0, tagLen);
    int outLen = inLen + tagLen;
    byte out[outLen];
    std::memset(out, 0, outLen);

    Aes aes[1];
    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesInit error:{}", ret);
        return BytesEmpty;
    }

    ret = wc_AesGcmSetKey(aes, key, keyLen);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesGcmSetKey error:{}", ret);
        return BytesEmpty;
    }

    ret = wc_AesGcmEncrypt(aes, out, in, inLen, initVector, initVectorLen, tag, tagLen, aad, aadLen);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesGcmEncrypt error:{}", ret);
        return BytesEmpty;
    }
    std::memcpy(out + inLen, tag, tagLen);

    wc_AesFree(aes);

    return wrapBytes(out, outLen);
}

const Bytes NGenXX::Crypto::AES::gcmDecrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, const size_t tagBits)
{
    if (!NGenXX::Crypto::AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits))
    {
        return BytesEmpty;
    }
    auto in = inBytes.data(), key = keyBytes.data(), initVector = initVectorBytes.data(), aad = aadBytes.data();
    auto inLen_ = inBytes.size(), keyLen = keyBytes.size(), initVectorLen = initVectorBytes.size(), aadLen = aadBytes.size();
    const size_t tagLen = tagBits / 8;

    size_t inLen = inLen_ - tagLen;
    byte tag[tagLen];
    std::memcpy(tag, in + inLen, tagLen);
    int outLen = inLen;
    byte out[outLen];
    std::memset(out, 0, outLen);

    Aes aes[1];
    int ret = wc_AesInit(aes, NULL, 0);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesInit error:{}", ret);
        return BytesEmpty;
    }

    ret = wc_AesGcmSetKey(aes, key, keyLen);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesGcmSetKey error:{}", ret);
        return BytesEmpty;
    }

    ret = wc_AesGcmDecrypt(aes, out, in, inLen, initVector, initVectorLen, tag, tagLen, aad, aadLen);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesGcmDecrypt error:{}", ret);
        return BytesEmpty;
    }

    wc_AesFree(aes);

    return wrapBytes(out, outLen);
}

const Bytes NGenXX::Crypto::Hash::md5(const Bytes &inBytes)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == NULL || inLen == 0)
    {
        return BytesEmpty;
    }
    int outLen = 16;
    byte out[outLen];
    std::memset(out, 0, outLen);

    wc_Md5 md5;

    int ret = wc_InitMd5(&md5);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_InitMd5 error:{}", ret);
        return BytesEmpty;
    }

    ret = wc_Md5Update(&md5, (byte *)in, inLen);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_Md5Update error:{}", ret);
        return BytesEmpty;
    }

    ret = wc_Md5Final(&md5, out);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_Md5Final error:{}", ret);
        return BytesEmpty;
    }

    wc_Md5Free(&md5);

    return wrapBytes(out, outLen);
}

const Bytes NGenXX::Crypto::Hash::sha256(const Bytes &inBytes)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == NULL || inLen == 0)
    {
        return BytesEmpty;
    }
    int outLen = 32;
    byte out[outLen];
    std::memset(out, 0, outLen);

    wc_Sha256 sha256;

    int ret = wc_InitSha256(&sha256);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_InitSha256 error:{}", ret);
        return BytesEmpty;
    }

    wc_Sha256Update(&sha256, (byte *)in, inLen);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_Sha256Update error:{}", ret);
        return BytesEmpty;
    }

    wc_Sha256Final(&sha256, out);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_Sha256Final error:{}", ret);
        return BytesEmpty;
    }

    wc_Sha256Free(&sha256);

    return wrapBytes(out, outLen);
}

const Bytes NGenXX::Crypto::Base64::encode(const Bytes &inBytes)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == NULL || inLen == 0)
    {
        return BytesEmpty;
    }

    word32 outLen = 0;
    int ret = Base64_Encode_NoNl(in, inLen, NULL, &outLen);

    byte outBuffer[outLen + 1];
    std::memset(outBuffer, 0, outLen + 1);

    ret = Base64_Encode_NoNl(in, inLen, outBuffer, &outLen);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Base64_Encode error:{}", ret);
        return BytesEmpty;
    }

    return wrapBytes(outBuffer, outLen);
}

const Bytes NGenXX::Crypto::Base64::decode(const Bytes &inBytes)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == NULL || inLen == 0)
    {
        return BytesEmpty;
    }

    word32 outLen = inLen * 2;
    byte outBuffer[outLen + 1];
    std::memset(outBuffer, 0, outLen + 1);

    int ret = Base64_Decode(in, inLen, outBuffer, &outLen);
    if (ret != WolfSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Base64_Decode error:{}", ret);
        return BytesEmpty;
    }

    return wrapBytes(outBuffer, outLen);
}
#endif