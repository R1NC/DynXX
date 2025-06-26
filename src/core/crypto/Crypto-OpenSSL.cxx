#include "Crypto.hxx"

#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#include <NGenXXLog.hxx>

namespace
{
    constexpr auto OK = 1;
    constexpr auto AES_Key_BITS = 128;

    constexpr auto MD5_BYTES_LEN = 16uz;
    constexpr auto SHA256_BYTES_LEN = 32uz;

    class EvpCipherCtx
    {
    public:
        EvpCipherCtx()
        {
            raw = EVP_CIPHER_CTX_new();
        }
        ~EvpCipherCtx()
        {
            if (raw != nullptr) [[likely]]
            {
                EVP_CIPHER_CTX_free(raw);
                raw = nullptr;
            }
        }
        EvpCipherCtx(const EvpCipherCtx &) = delete;
        EvpCipherCtx &operator=(const EvpCipherCtx &) = delete;
        EvpCipherCtx(EvpCipherCtx &&) = delete;
        EvpCipherCtx &operator=(EvpCipherCtx &&) = delete;
        EVP_CIPHER_CTX *raw;
    };

    const EVP_CIPHER *aesGcmCipher(const Bytes &keyBytes)
    {
        const auto key = keyBytes.data();
        const auto keyLen = keyBytes.size();
        if (key != nullptr && keyLen > 0) [[likely]]
        {
            if (keyLen == 16)
            {
                return EVP_aes_128_gcm();
            }
            if (keyLen == 24 || keyLen == 32)
            {
                return EVP_aes_256_gcm();
            }
        }
        return nullptr;
    }

    RSA *createRSA(const Bytes &key, bool isPublic)
    {
        RSA *rsa = nullptr;
        const auto bio = BIO_new_mem_buf(key.data(), -1);
        if (!bio) [[unlikely]]
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "Failed to create BIO mem buffer for RSA");
            return rsa;
        }
        if (isPublic) 
        {
            rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
        } 
        else 
        {
            rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
        }
        if (!rsa) [[unlikely]]
        {
            char sErr[256];
            ERR_error_string_n(ERR_get_error(), sErr, sizeof(sErr));
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "RSA Failed to create key: {}", std::string(sErr));
        }
        BIO_free(bio);
        return rsa;
    }
}

#pragma mark Rand

bool NGenXX::Core::Crypto::rand(size_t len, byte *bytes)
{
    if (len == 0 || bytes == nullptr) [[unlikely]]
    {
        return false;
    }
    const auto ret = RAND_bytes(bytes, static_cast<int>(len));
    return ret != -1;
}

#pragma mark AES

Bytes NGenXX::Core::Crypto::AES::encrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    const auto in = inBytes.data(), key = keyBytes.data();
    const auto inLen = inBytes.size(), keyLen = keyBytes.size();
    if (in == nullptr || inLen == 0 || key == nullptr || keyLen != AES_BLOCK_SIZE) [[unlikely]]
    {
        return {};
    }
    
    //ECB-PKCS5 Padding:
    auto outLen = inLen;
    const auto paddingSize = AES_BLOCK_SIZE - (inLen % AES_BLOCK_SIZE);
    const auto paddingData = static_cast<byte>(paddingSize);
    outLen += paddingSize;

    Bytes fixedIn(outLen, paddingData);
    std::memcpy(fixedIn.data(), in, inLen);
    Bytes out(outLen, 0);

    AES_KEY aes_key;

    if (const auto ret = AES_set_encrypt_key(key, AES_Key_BITS, &aes_key); ret != 0) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "AES_set_encrypt_key error:{}", ret);
        return {};
    }
    
    for (auto offset(0z); offset < outLen; offset += AES_BLOCK_SIZE)
    {
        AES_encrypt(fixedIn.data() + offset, out.data() + offset, &aes_key);
    }

    return out;
}

Bytes NGenXX::Core::Crypto::AES::decrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    const auto in = inBytes.data(), key = keyBytes.data();
    const auto inLen = inBytes.size(), keyLen = keyBytes.size();
    if (in == nullptr || inLen == 0 || key == nullptr 
        || keyLen != AES_BLOCK_SIZE || inLen % AES_BLOCK_SIZE != 0) [[unlikely]]
    {
        return {};
    }

    Bytes fixedIn(inLen, 0);
    std::memcpy(fixedIn.data(), in, inLen);
    Bytes out(inLen, 0);

    AES_KEY aes_key;
    if (const auto ret = AES_set_decrypt_key(key, AES_Key_BITS, &aes_key); ret != 0) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "AES_set_decrypt_key error:{}", ret);
        return {};
    }

    for (auto offset(0z); offset < inLen; offset += AES_BLOCK_SIZE)
    {
        AES_decrypt(fixedIn.data() + offset, out.data() + offset, &aes_key);
    }

    // Proper PKCS5/PKCS7 unpadding
    auto paddingSize = static_cast<size_t>(out[inLen - 1]);
    if (paddingSize == 0 || paddingSize > AES_BLOCK_SIZE) [[unlikely]]
    {
        return {};
    }

    // Verify padding bytes
    for (decltype(paddingSize) i = 0; i < paddingSize; i++) 
    {
        if (out[inLen - 1 - i] != paddingSize) [[unlikely]]
        {
            return {};
        }
    }
    
    out.resize(inLen - paddingSize);
    return out;
}

#pragma mark AES-GCM

Bytes NGenXX::Core::Crypto::AES::gcmEncrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, size_t tagBits)
{
    if (!checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return {};
    }
    const auto in = inBytes.data(), key = keyBytes.data(), initVector = initVectorBytes.data(), aad = aadBytes.data();
    const auto inLen = inBytes.size(), keyLen = keyBytes.size(), initVectorLen = initVectorBytes.size(), aadLen = aadBytes.size();
    const auto tagLen = tagBits / 8;

    Bytes tag(tagLen, 0);
    const auto outLen = inLen;
    Bytes out(outLen, 0);

    const auto cipher = aesGcmCipher(keyBytes);

    const EvpCipherCtx evpCipherCtx;
    if (evpCipherCtx.raw == nullptr) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_new failed");
        return {};
    }

    auto ret = EVP_EncryptInit_ex(evpCipherCtx.raw, cipher, nullptr, nullptr, nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptInit_ex cipher error:{}", ret);
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evpCipherCtx.raw, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(initVectorLen), nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN error:{}", ret);
        return {};
    }

    ret = EVP_EncryptInit_ex(evpCipherCtx.raw, nullptr, nullptr, key, initVector);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptInit_ex initVector error:{}", ret);
        return {};
    }

    int len;

    if (aad != nullptr && aadLen > 0) [[likely]]
    {
        ret = EVP_EncryptUpdate(evpCipherCtx.raw, nullptr, &len, aad, static_cast<int>(aadLen));
        if (ret != OK) [[unlikely]]
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptUpdate aad error:{}", ret);
            return {};
        }
    }

    ret = EVP_EncryptUpdate(evpCipherCtx.raw, out.data(), &len, in, static_cast<int>(inLen));
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptUpdate error:{}", ret);
        return {};
    }

    ret = EVP_EncryptFinal_ex(evpCipherCtx.raw, out.data(), &len);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptFinal_ex error:{}", ret);
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evpCipherCtx.raw, EVP_CTRL_GCM_GET_TAG, static_cast<int>(tagLen), tag.data());
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_GET_TAG error:{}", ret);
        return {};
    }
    out.insert(out.end(), tag.begin(), tag.end());

    return out;
}

Bytes NGenXX::Core::Crypto::AES::gcmDecrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, size_t tagBits)
{
    if (!checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return {};
    }
    const auto in = inBytes.data(), key = keyBytes.data(), initVector = initVectorBytes.data(), aad = aadBytes.data();
    const auto inLen_ = inBytes.size(), keyLen = keyBytes.size(), initVectorLen = initVectorBytes.size(), aadLen = aadBytes.size();
    const auto tagLen = tagBits / 8;

    const auto inLen = inLen_ - tagLen;
    Bytes tag(tagLen, 0);
    std::memcpy(tag.data(), in + inLen, tagLen);
    const auto outLen = inLen;
    Bytes out(outLen, 0);

    const auto cipher = aesGcmCipher(keyBytes);

    const EvpCipherCtx evpCipherCtx;
    if (evpCipherCtx.raw == nullptr) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_CIPHER_CTX_new failed");
        return {};
    }

    auto ret = EVP_DecryptInit_ex(evpCipherCtx.raw, cipher, nullptr, nullptr, nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptInit_ex cipher error:{}", ret);
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evpCipherCtx.raw, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(initVectorLen), nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmdDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN error:{}", ret);
        return {};
    }

    ret = EVP_DecryptInit_ex(evpCipherCtx.raw, nullptr, nullptr, key, initVector);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptInit_ex initVector error:{}", ret);
        return {};
    }

    int len;

    if (aad != nullptr && aadLen > 0) [[likely]]
    {
        ret = EVP_DecryptUpdate(evpCipherCtx.raw, nullptr, &len, aad, static_cast<int>(aadLen));
        if (ret != OK) [[unlikely]]
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptUpdate aad error:{}", ret);
            return {};
        }
    }

    ret = EVP_DecryptUpdate(evpCipherCtx.raw, out.data(), &len, in, static_cast<int>(inLen));
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptUpdate error:{}", ret);
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evpCipherCtx.raw, EVP_CTRL_GCM_SET_TAG, static_cast<int>(tagLen), tag.data());
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_TAG error:{}", ret);
        return {};
    }

    ret = EVP_DecryptFinal_ex(evpCipherCtx.raw, out.data(), &len);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptFinal_ex error:{}", ret);
        return {};
    }

    out.resize(outLen);
    return out;
}

#pragma mark MD5

Bytes NGenXX::Core::Crypto::Hash::md5(const Bytes &inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }
    constexpr auto outLen = MD5_BYTES_LEN;
    Bytes out(outLen, 0);

    MD5_CTX md5;

    auto ret = MD5_Init(&md5);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Init error:{}", ret);
        return {};
    }

    ret = MD5_Update(&md5, in, inLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Update error:{}", ret);
        return {};
    }

    ret = MD5_Final(out.data(), &md5);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Final error:{}", ret);
        return {};
    }

    return out;
}

#pragma mark SHA1

Bytes NGenXX::Core::Crypto::Hash::sha1(const Bytes &inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0)  [[unlikely]]
    {
        return {};
    }
    unsigned int outLen = EVP_MAX_MD_SIZE;
    Bytes out(outLen, 0);

    const auto ctx = EVP_MD_CTX_create();
    const auto md = EVP_sha1();

    auto ret = EVP_DigestInit_ex(ctx, md, nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestInit_ex error:{}", ret);
        return {};
    }

    ret = EVP_DigestUpdate(ctx, in, inLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestUpdate error:{}", ret);
        return {};
    }

    ret = EVP_DigestFinal_ex(ctx, out.data(), &outLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestFinal_ex error:{}", ret);
        return {};
    }

    EVP_MD_CTX_destroy(ctx);

    return out;
}

#pragma mark SHA256
Bytes NGenXX::Core::Crypto::Hash::sha256(const Bytes &inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }
    constexpr auto outLen = SHA256_BYTES_LEN;
    Bytes out(outLen, 0);

    SHA256_CTX sha256;

    auto ret = SHA256_Init(&sha256);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Init error:{}", ret);
        return {};
    }

    ret = SHA256_Update(&sha256, in, inLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Update error:{}", ret);
        return {};
    }

    ret = SHA256_Final(out.data(), &sha256);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Final error:{}", ret);
        return {};
    }

    return out;
}

#pragma mark RSA

std::string NGenXX::Core::Crypto::RSA::genKey(const std::string_view &base64, bool isPublic) 
{ 
    constexpr auto chunkSize = 64; 
    using namespace std::string_view_literals;
    
    constexpr auto divider = "-----"sv;
    constexpr auto beginStr = "BEGIN "sv;
    constexpr auto endStr = "END "sv;
    constexpr auto keyStr = " KEY"sv;
    constexpr auto newLineStr = "\n"sv;
    const auto mid = isPublic ? "PUBLIC"sv : "RSA PRIVATE"sv;
    
    const auto headerSize = divider.size() + beginStr.size() + mid.size() + keyStr.size() + divider.size() + newLineStr.size(); 
    const auto footerSize = divider.size() + endStr.size() + mid.size() + keyStr.size() + divider.size() + newLineStr.size(); 
    const auto contentSize = base64.size() + (base64.size() / chunkSize) + (base64.size() % chunkSize ? 1 : 0); 
    
    std::string result; 
    result.reserve(headerSize + contentSize + footerSize); 
    
    result.append(divider).append(beginStr).append(mid).append(keyStr).append(divider).append(newLineStr); 
    
    for (size_t i = 0; i < base64.size(); i += chunkSize) 
    { 
        result.append(base64.substr(i, chunkSize)).append(newLineStr); 
    } 
    
    result.append(divider).append(endStr).append(mid).append(keyStr).append(divider).append(newLineStr); 
    
    return result; 
}

Bytes NGenXX::Core::Crypto::RSA::encrypt(const Bytes &in, const Bytes &key, int padding)
{
    const auto rsa = createRSA(key, true);
    if (rsa == nullptr) [[unlikely]]
    {
        return {};
    }
    const auto outLen = RSA_size(rsa);
    Bytes outBytes(outLen, 0);
    if (const auto ret = RSA_public_encrypt(static_cast<int>(in.size()), in.data(), outBytes.data(), rsa, padding); ret == -1) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "RSA encrypt error:{}", ret);
        RSA_free(rsa);
        return {};
    }
    RSA_free(rsa);
    return outBytes;
}

Bytes NGenXX::Core::Crypto::RSA::decrypt(const Bytes &in, const Bytes &key, int padding)
{
    const auto rsa = createRSA(key, false);
    if (rsa == nullptr) [[unlikely]]
    {
        return {};
    }
    const auto outLen = RSA_size(rsa);
    Bytes outBytes(outLen, 0);
    if (const auto ret = RSA_private_decrypt(static_cast<int>(in.size()), in.data(), outBytes.data(), rsa, padding); ret == -1) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "RSA decrypt error:{}", ret);
        RSA_free(rsa);
        return {};
    }
    RSA_free(rsa);
    return outBytes;
}

#pragma mark Base64

Bytes NGenXX::Core::Crypto::Base64::encode(const Bytes &inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }

    BIO *b64 = BIO_new(BIO_f_base64());
    if (!b64) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "Failed to create BIO for Base64");
        return {};
    }

    BIO *bmem = BIO_new(BIO_s_mem());
    if (!bmem) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "Failed to create BIO mem for Base64");
        return {};
    }

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    b64 = BIO_push(b64, bmem);

    BIO_write(b64, in, static_cast<int>(inLen));
    BIO_flush(b64);

    char *outBytes = nullptr;
    const auto outLen = BIO_get_mem_data(bmem, &outBytes);
    if (outLen == 0) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "Failed to encode Base64");
        BIO_free_all(b64);
        return {};
    }

    auto out = wrapBytes(reinterpret_cast<byte *>(outBytes), outLen);

    BIO_free_all(b64);

    return out;
}

Bytes NGenXX::Core::Crypto::Base64::decode(const Bytes &inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }

    for (size_t i = 0; i < inLen; i++)
    {
        if (std::isalnum(in[i]) || in[i] == '+' || in[i] == '/' || in[i] == '=')
        {
            continue;
        }
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Invalid Base64 character:{}", in[i]);
        return {};
    }

    BIO *bio = BIO_new_mem_buf(in, -1);
    if (!bio) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "Failed to create BIO mem buffer for Base64");
        return {};
    }

    BIO *b64 = BIO_new(BIO_f_base64());
    if (!b64) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "Failed to create BIO for Base64");
        BIO_free_all(bio);
        return {};
    }

    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    auto outLen = inLen * 3 / 4;
    Bytes outBytes(outLen, 0);
    outLen = BIO_read(bio, outBytes.data(), static_cast<int>(outLen));
    if (outLen == 0) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "Failed to decode Base64");
        BIO_free_all(bio);
        return {};
    }

    BIO_free_all(bio);
    outBytes.resize(outLen);
    return outBytes;
}
