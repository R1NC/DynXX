#if defined(USE_WOLF_SSL)
#include "Crypto.hxx"

#include <string>
#include <cstring>

#include <wolfssl/wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/md5.h>
#include <wolfssl/wolfcrypt/sha.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/coding.h>

#include <NGenXXLog.hxx>
#include "../util/TypeUtil.hxx"

namespace 
{
    constexpr auto OK = 0;
}

bool NGenXX::Core::Crypto::rand(size_t len, byte *bytes)
{
    if (len == 0 || bytes == nullptr) [[unlikely]]
    {
        return false;
    }
    auto ret = wolfSSL_RAND_bytes(bytes, len);
    return ret == WOLFSSL_SUCCESS;
}

Bytes NGenXX::Core::Crypto::AES::encrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    auto in = inBytes.data(), key = keyBytes.data();
    auto inLen = inBytes.size(), keyLen = keyBytes.size();
    if (in == nullptr || inLen == 0 || key == nullptr || keyLen != AES_BLOCK_SIZE) [[unlikely]]
    {
        return {};
    }

    //ECB-PKCS5 Padding:
    auto outLen = inLen;
    auto paddingSize = AES_BLOCK_SIZE - (inLen % AES_BLOCK_SIZE);
    auto paddingData = static_cast<byte>(paddingSize);
    outLen += paddingSize;

    byte fixedIn[outLen];
    std::memset(fixedIn, paddingData, outLen);
    std::memcpy(fixedIn, in, inLen);
    byte out[outLen];
    std::memset(out, 0, outLen);
    int offset = 0;

    Aes aes[1];

    auto ret = wc_AesInit(aes, nullptr, 0);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "AES_set_decrypt_key error:{}", ret);
        return {};
    }

    ret = wc_AesSetKey(aes, key, keyLen, out, AES_ENCRYPTION);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesSetKey_AES_ENCRYPTION error:{}", ret);
        return {};
    }

    while (offset < inLen)
    {
        ret = wc_AesEncryptDirect(aes, out + offset, fixedIn + offset);
        if (ret != OK)
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesEncryptDirect error:{}", ret);
            return {};
        }
        offset += AES_BLOCK_SIZE;
    }

    wc_AesFree(aes);

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Core::Crypto::AES::decrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    auto in = inBytes.data(), key = keyBytes.data();
    auto inLen = inBytes.size(), keyLen = keyBytes.size();
    if (in == nullptr || inLen == 0 || key == nullptr || keyLen != AES_BLOCK_SIZE) [[unlikely]]
    {
        return {};
    }
    
    //ECB-PKCS5 UnPadding:
    auto paddingSize = static_cast<size_t>(inBytes[inLen - 1]);
    if (paddingSize < inLen && paddingSize < AES_BLOCK_SIZE)
    {
        inLen -= paddingSize;
    }
    auto outLen = inLen;

    byte fixedIn[outLen];
    std::memcpy(fixedIn, in, inLen);
    byte out[outLen];
    std::memset(out, 0, outLen);
    int offset = 0;

    Aes aes[1];

    auto ret = wc_AesInit(aes, nullptr, 0);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesInit error:{}", ret);
        return {};
    }

    ret = wc_AesSetKey(aes, key, keyLen, out, AES_DECRYPTION);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesSetKey_AES_DECRYPTION error:{}", ret);
        return {};
    }

    while (offset < inLen)
    {
        ret = wc_AesDecryptDirect(aes, out + offset, fixedIn + offset);
        if (ret != OK)
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesDecryptDirect error:{}", ret);
            return {};
        }
        offset += AES_BLOCK_SIZE;
    }

    wc_AesFree(aes);

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Core::Crypto::AES::gcmEncrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, size_t tagBits)
{
    if (!NGenXX::Core::Crypto::AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return {};
    }
    auto in = inBytes.data(), key = keyBytes.data(), initVector = initVectorBytes.data(), aad = aadBytes.data();
    auto inLen = inBytes.size(), keyLen = keyBytes.size(), initVectorLen = initVectorBytes.size(), aadLen = aadBytes.size();
    auto tagLen = tagBits / 8;

    byte tag[tagLen];
    std::memset(tag, 0, tagLen);
    auto outLen = inLen + tagLen;
    byte out[outLen];
    std::memset(out, 0, outLen);

    Aes aes[1];
    auto ret = wc_AesInit(aes, nullptr, 0);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesInit error:{}", ret);
        return {};
    }

    ret = wc_AesGcmSetKey(aes, key, keyLen);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesGcmSetKey error:{}", ret);
        return {};
    }

    ret = wc_AesGcmEncrypt(aes, out, in, inLen, initVector, initVectorLen, tag, tagLen, aad, aadLen);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesGcmEncrypt error:{}", ret);
        return {};
    }
    std::memcpy(out + inLen, tag, tagLen);

    wc_AesFree(aes);

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Core::Crypto::AES::gcmDecrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, size_t tagBits)
{
    if (!NGenXX::Core::Crypto::AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return {};
    }
    auto in = inBytes.data(), key = keyBytes.data(), initVector = initVectorBytes.data(), aad = aadBytes.data();
    auto inLen_ = inBytes.size(), keyLen = keyBytes.size(), initVectorLen = initVectorBytes.size(), aadLen = aadBytes.size();
    auto tagLen = tagBits / 8;

    auto inLen = inLen_ - tagLen;
    byte tag[tagLen];
    std::memcpy(tag, in + inLen, tagLen);
    auto outLen = inLen;
    byte out[outLen];
    std::memset(out, 0, outLen);

    Aes aes[1];
    auto ret = wc_AesInit(aes, nullptr, 0);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesInit error:{}", ret);
        return {};
    }

    ret = wc_AesGcmSetKey(aes, key, keyLen);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesGcmSetKey error:{}", ret);
        return {};
    }

    ret = wc_AesGcmDecrypt(aes, out, in, inLen, initVector, initVectorLen, tag, tagLen, aad, aadLen);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_AesGcmDecrypt error:{}", ret);
        return {};
    }

    wc_AesFree(aes);

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Core::Crypto::Hash::md5(const Bytes &inBytes)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }
    int outLen = 16;
    byte out[outLen];
    std::memset(out, 0, outLen);

    wc_Md5 md5;

    auto ret = wc_InitMd5(&md5);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_InitMd5 error:{}", ret);
        return {};
    }

    ret = wc_Md5Update(&md5, (byte *)in, inLen);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_Md5Update error:{}", ret);
        return {};
    }

    ret = wc_Md5Final(&md5, out);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_Md5Final error:{}", ret);
        return {};
    }

    wc_Md5Free(&md5);

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Core::Crypto::Hash::sha256(const Bytes &inBytes)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }
    int outLen = 32;
    byte out[outLen];
    std::memset(out, 0, outLen);

    wc_Sha256 sha256;

    auto ret = wc_InitSha256(&sha256);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_InitSha256 error:{}", ret);
        return {};
    }

    ret = wc_Sha256Update(&sha256, (byte *)in, inLen);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_Sha256Update error:{}", ret);
        return {};
    }

    ret = wc_Sha256Final(&sha256, out);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "wc_Sha256Final error:{}", ret);
        return {};
    }

    wc_Sha256Free(&sha256);

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Core::Crypto::Base64::encode(const Bytes &inBytes, bool noNewLines)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }

    word32 outLen = 0;
    auto ret = Base64_Encode_NoNl(in, inLen, nullptr, &outLen);

    byte outBuffer[outLen + 1];
    std::memset(outBuffer, 0, outLen + 1);

    ret = Base64_Encode_NoNl(in, inLen, outBuffer, &outLen);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Base64_Encode error:{}", ret);
        return {};
    }

    return wrapBytes(outBuffer, outLen);
}

Bytes NGenXX::Core::Crypto::Base64::decode(const Bytes &inBytes, bool noNewLines)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }

    word32 outLen = inLen * 2;
    byte outBuffer[outLen + 1];
    std::memset(outBuffer, 0, outLen + 1);

    auto ret = Base64_Decode(in, inLen, outBuffer, &outLen);
    if (ret != OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Base64_Decode error:{}", ret);
        return {};
    }

    return wrapBytes(outBuffer, outLen);
}
#endif