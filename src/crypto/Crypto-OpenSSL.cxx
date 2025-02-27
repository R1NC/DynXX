#include "Crypto.hxx"

#include <string>
#include <cstring>

#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/buffer.h>

#include <NGenXXLog.hxx>
#include "../util/TypeUtil.hxx"

constexpr auto OpenSSL_OK = 1;
constexpr auto OpenSSL_AES_Key_BITS = 128;

bool NGenXX::Crypto::rand(const size_t len, byte *bytes)
{
    if (len == 0 || bytes == NULL) [[unlikely]]
    {
        return false;
    }
    auto ret = RAND_bytes(bytes, static_cast<int>(len));
    return ret != -1;
}

Bytes NGenXX::Crypto::AES::encrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    auto in = inBytes.data(), key = keyBytes.data();
    auto inLen = inBytes.size(), keyLen = keyBytes.size();
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE) [[unlikely]]
    {
        return BytesEmpty;
    }
    // keyLen = AES_BLOCK_SIZE * 8;
    auto outLen = inLen;
    if (inLen % AES_BLOCK_SIZE != 0)
    {
        outLen = (inLen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
    byte fixedIn[outLen];
    std::memset(fixedIn, 0, outLen);
    std::memcpy(fixedIn, in, inLen);
    byte out[outLen];
    std::memset(out, 0, outLen);

    AES_KEY aes_key;

    auto ret = AES_set_encrypt_key(key, OpenSSL_AES_Key_BITS, &aes_key);
    if (ret != 0)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "AES_set_encrypt_key error:{}", ret);
        return BytesEmpty;
    }
    
    decltype(inLen) offset(0);
    while (offset < inLen)
    {
        AES_encrypt(fixedIn + offset, out + offset, &aes_key);
        offset += AES_BLOCK_SIZE;
    }

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Crypto::AES::decrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    auto in = inBytes.data(), key = keyBytes.data();
    auto inLen = inBytes.size(), keyLen = keyBytes.size();
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE) [[unlikely]]
    {
        return BytesEmpty;
    }
    // keyLen = AES_BLOCK_SIZE * 8;
    auto outLen = inLen;
    if (inLen % AES_BLOCK_SIZE != 0)
    {
        outLen = (inLen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
    byte fixedIn[outLen];
    std::memset(fixedIn, 0, outLen);
    std::memcpy(fixedIn, in, inLen);
    byte out[outLen];
    std::memset(out, 0, outLen);

    AES_KEY aes_key;

    auto ret = AES_set_decrypt_key(key, OpenSSL_AES_Key_BITS, &aes_key);
    if (ret != 0)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "AES_set_decrypt_key error:{}", ret);
        return BytesEmpty;
    }

    decltype(inLen) offset(0);
    while (offset < inLen)
    {
        AES_decrypt(fixedIn + offset, out + offset, &aes_key);
        offset += AES_BLOCK_SIZE;
    }

    return trimBytes(wrapBytes(out, outLen));
}

const EVP_CIPHER *aesGcmCipher(const Bytes &keyBytes)
{
    auto key = keyBytes.data();
    auto keyLen = keyBytes.size();
    if (key != NULL && keyLen > 0)
    {
        if (keyLen == 16)
        {
            return EVP_aes_128_gcm();
        }
        if (keyLen == 24)
        {
            return EVP_aes_256_gcm();
        }
        if (keyLen == 32)
        {
            return EVP_aes_128_gcm();
        }
    }
    return NULL;
}

Bytes NGenXX::Crypto::AES::gcmEncrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, const size_t tagBits)
{
    if (!NGenXX::Crypto::AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return BytesEmpty;
    }
    auto in = inBytes.data(), key = keyBytes.data(), initVector = initVectorBytes.data(), aad = aadBytes.data();
    auto inLen = inBytes.size(), keyLen = keyBytes.size(), initVectorLen = initVectorBytes.size(), aadLen = aadBytes.size();
    auto tagLen = tagBits / 8;

    byte tag[tagLen];
    std::memset(tag, 0, tagLen);
    auto outLen = inLen + tagLen;
    byte out[outLen];
    std::memset(out, 0, outLen);

    auto cipher = aesGcmCipher(keyBytes);

    EVP_CIPHER_CTX *ctx;

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_new failed");
        return BytesEmpty;
    }

    auto ret = EVP_EncryptInit_ex(ctx, cipher, NULL, NULL, NULL);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptInit_ex cipher error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }

    ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(initVectorLen), NULL);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }

    ret = EVP_EncryptInit_ex(ctx, NULL, NULL, key, initVector);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptInit_ex initVector error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }

    int len;

    if (aad != NULL && aadLen > 0)
    {
        ret = EVP_EncryptUpdate(ctx, NULL, &len, aad, static_cast<int>(aadLen));
        if (ret != OpenSSL_OK)
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptUpdate aad error:{}", ret);
            EVP_CIPHER_CTX_free(ctx);
            return BytesEmpty;
        }
    }

    ret = EVP_EncryptUpdate(ctx, out, &len, in, static_cast<int>(inLen));
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptUpdate error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }

    ret = EVP_EncryptFinal_ex(ctx, out, &len);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptFinal_ex error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }

    ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, static_cast<int>(tagLen), tag);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_GET_TAG error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }
    std::memcpy(out + inLen, tag, tagLen);

    EVP_CIPHER_CTX_free(ctx);

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Crypto::AES::gcmDecrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, const size_t tagBits)
{
    if (!NGenXX::Crypto::AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return BytesEmpty;
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

    auto cipher = aesGcmCipher(keyBytes);

    EVP_CIPHER_CTX *ctx;

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_CIPHER_CTX_new failed");
        return BytesEmpty;
    }

    auto ret = EVP_DecryptInit_ex(ctx, cipher, NULL, NULL, NULL);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptInit_ex cipher error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }

    ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(initVectorLen), NULL);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmdDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }

    ret = EVP_DecryptInit_ex(ctx, NULL, NULL, key, initVector);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptInit_ex initVector error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }

    int len;

    if (aad != NULL && aadLen > 0)
    {
        ret = EVP_DecryptUpdate(ctx, NULL, &len, aad, static_cast<int>(aadLen));
        if (ret != OpenSSL_OK)
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptUpdate aad error:{}", ret);
            EVP_CIPHER_CTX_free(ctx);
            return BytesEmpty;
        }
    }

    ret = EVP_DecryptUpdate(ctx, out, &len, in, static_cast<int>(inLen));
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptUpdate error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }

    ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, static_cast<int>(tagLen), tag);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_TAG error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }

    ret = EVP_DecryptFinal_ex(ctx, out, &len);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptFinal_ex error:{}", ret);
        EVP_CIPHER_CTX_free(ctx);
        return BytesEmpty;
    }

    EVP_CIPHER_CTX_free(ctx);

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Crypto::Hash::md5(const Bytes &inBytes)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == NULL || inLen == 0) [[unlikely]]
    {
        return BytesEmpty;
    }
    auto outLen = MD5_BYTES_LEN;
    byte out[outLen];
    std::memset(out, 0, outLen);

    MD5_CTX md5;

    auto ret = MD5_Init(&md5);
    if (ret != OpenSSL_OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Init error:{}", ret);
        return BytesEmpty;
    }

    ret = MD5_Update(&md5, in, inLen);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Update error:{}", ret);
        return BytesEmpty;
    }

    ret = MD5_Final(out, &md5);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Final error:{}", ret);
        return BytesEmpty;
    }

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Crypto::Hash::sha1(const Bytes &inBytes)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == NULL || inLen == 0)  [[unlikely]]
    {
        return BytesEmpty;
    }
    unsigned int outLen = EVP_MAX_MD_SIZE;
    byte out[outLen];
    memset(out, 0, outLen);

    auto ctx = EVP_MD_CTX_create();
    auto md = EVP_sha1();

    auto ret = EVP_DigestInit_ex(ctx, md, NULL);
    if (ret != OpenSSL_OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestInit_ex error:{}", ret);
        return BytesEmpty;
    }

    ret = EVP_DigestUpdate(ctx, in, inLen);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestUpdate error:{}", ret);
        return BytesEmpty;
    }

    ret = EVP_DigestFinal_ex(ctx, out, &outLen);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestFinal_ex error:{}", ret);
        return BytesEmpty;
    }

    EVP_MD_CTX_destroy(ctx);

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Crypto::Hash::sha256(const Bytes &inBytes)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == NULL || inLen == 0) [[unlikely]]
    {
        return BytesEmpty;
    }
    auto outLen = SHA256_BYTES_LEN;
    byte out[outLen];
    std::memset(out, 0, outLen);

    SHA256_CTX sha256;

    auto ret = SHA256_Init(&sha256);
    if (ret != OpenSSL_OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Init error:{}", ret);
        return BytesEmpty;
    }

    ret = SHA256_Update(&sha256, in, inLen);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Update error:{}", ret);
        return BytesEmpty;
    }

    ret = SHA256_Final(out, &sha256);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Final error:{}", ret);
        return BytesEmpty;
    }

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Crypto::Base64::encode(const Bytes &inBytes)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == NULL || inLen == 0) [[unlikely]]
    {
        return BytesEmpty;
    }

    BIO *b64, *bmem;

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    b64 = BIO_push(b64, bmem);

    BIO_write(b64, in, static_cast<int>(inLen));
    BIO_flush(b64);

    char *outBytes = nullptr;
    auto outLen = BIO_get_mem_data(bmem, &outBytes);
    if (outLen == 0)
    {
        BIO_free_all(b64);
        return BytesEmpty;
    }

    auto out = wrapBytes(reinterpret_cast<byte *>(outBytes), outLen);

    BIO_free_all(b64);

    return out;
}

Bytes NGenXX::Crypto::Base64::decode(const Bytes &inBytes)
{
    auto in = inBytes.data();
    auto inLen = inBytes.size();
    if (in == NULL || inLen == 0) [[unlikely]]
    {
        return BytesEmpty;
    }

    BIO *bio, *b64;
    bio = BIO_new_mem_buf(in, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    byte outBytes[inLen * 2];
    auto outLen = BIO_read(bio, outBytes, static_cast<int>(inLen));
    if (outLen == 0)
    {
        BIO_free_all(bio);
        return BytesEmpty;
    }

    auto out = wrapBytes(outBytes, outLen);
    BIO_free_all(bio);

    return out;
}

RSA *ngenxxCryptoCreateRSA(const Bytes &key, bool isPublic)
{
    RSA *rsa = NULL;
    auto bio = BIO_new_mem_buf(key.data(), -1);
    if (!bio) 
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "RSA Failed to create BIO");
        return rsa;
    }
    if (isPublic) 
    {
        rsa = PEM_read_bio_RSA_PUBKEY(bio, &rsa, NULL, NULL);
    } 
    else 
    {
        rsa = PEM_read_bio_RSAPrivateKey(bio, &rsa, NULL, NULL);
    }
    if (!rsa) 
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "RSA Failed to create key");
    }
    BIO_free(bio);
    return rsa;
}

Bytes NGenXX::Crypto::RSA::encrypt(const Bytes &in, const Bytes &key, const int padding)
{
    auto rsa = ngenxxCryptoCreateRSA(key, true);
    auto outLen = RSA_size(rsa);
    byte outBytes[outLen];
    auto ret = RSA_public_encrypt(in.size(), in.data(), outBytes, rsa, padding);
    if (ret == -1)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "RSA encrypt error:{}", ret);
        RSA_free(rsa);
        return BytesEmpty;
    }
    auto out = wrapBytes(outBytes, outLen);
    RSA_free(rsa);
    return out;
}

Bytes NGenXX::Crypto::RSA::decrypt(const Bytes &in, const Bytes &key, const int padding)
{
    auto rsa = ngenxxCryptoCreateRSA(key, false);
    auto outLen = RSA_size(rsa);
    byte outBytes[outLen];
    auto ret = RSA_private_decrypt(in.size(), in.data(), outBytes, rsa, padding);
    if (ret == -1)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "RSA decrypt error:{}", ret);
        RSA_free(rsa);
        return BytesEmpty;
    }
    auto out = wrapBytes(outBytes, outLen);
    RSA_free(rsa);
    return out;
}