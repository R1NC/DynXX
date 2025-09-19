#include "Crypto.hxx"

#include <utility>
#include <algorithm>
#include <array>
#if defined(__cpp_lib_ranges)
#include <ranges>
#endif

#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#include <DynXX/CXX/Coding.hxx>

#include "../concurrent/ConcurrentUtil.hxx"

namespace
{
    constexpr auto OK = 1;
    constexpr auto AES_Key_BITS = 128;
    constexpr auto MaxErrMsgLen = 256;

    using enum DynXXLogLevelX;

    std::string errMsg()
    {
        
        if (const auto err = ERR_get_error(); err != 0) [[likely]]
        {
            std::string errMsg;
            errMsg.resize(MaxErrMsgLen);
            ERR_error_string_n(err, errMsg.data(), errMsg.size());
            if (const auto actualLen = std::strlen(errMsg.data()); actualLen != 0) [[likely]] 
            {
                errMsg.resize(actualLen);
                return errMsg;
            }
        }

        return {};
    }

    const EVP_CIPHER* aesGcmCipher(const int keyLen)
    {
        switch (keyLen)
        {
        case 16:
            return EVP_aes_128_gcm();
        case 24:
        case 32:
            return EVP_aes_256_gcm();
        default:
            return nullptr;
        }
    }

    class EvpCipher
    {
    public:
        EvpCipher(const EvpCipher &) = delete;
        EvpCipher &operator=(const EvpCipher &) = delete;
        EvpCipher(EvpCipher &&) = delete;
        EvpCipher &operator=(EvpCipher &&) = delete;

        explicit EvpCipher()
        {
            this->ctx = EVP_CIPHER_CTX_new();
        }

        bool valid() const
        {
            return this->ctx != nullptr;
        }

        int ctrl(int type, int arg, RawPtr ptr = nullptr)
        {
            if (!valid()) [[unlikely]]
            {
                return -1;
            }
            return EVP_CIPHER_CTX_ctrl(this->ctx, type, arg, ptr);
        }

        int init(const bool encrypt, const EVP_CIPHER *type, const unsigned char *key, const unsigned char *iv, ENGINE *impl = nullptr)
        {
            if (!valid()) [[unlikely]]
            {
                return -1;
            }
            return encrypt ? EVP_EncryptInit_ex(this->ctx, type, impl, key, iv) : EVP_DecryptInit_ex(this->ctx, type, impl, key, iv);
        }

        int update(const bool encrypt, unsigned char *out, int *outl, const unsigned char *in, int inl)
        {
            if (!valid()) [[unlikely]]
            {
                return -1;
            }
            return encrypt ? EVP_EncryptUpdate(this->ctx, out, outl, in, inl) : EVP_DecryptUpdate(this->ctx, out, outl, in, inl);
        }

        int finish(const bool encrypt, unsigned char *out, int *outl)
        {
            if (!valid()) [[unlikely]]
            {
                return -1;
            }
            return encrypt ? EVP_EncryptFinal_ex(this->ctx, out, outl) : EVP_DecryptFinal_ex(this->ctx, out, outl);
        }

        ~EvpCipher()
        {
            if (valid()) [[likely]]
            {
                EVP_CIPHER_CTX_free(this->ctx);
                this->ctx = nullptr;
            }
        }

    private:
        EVP_CIPHER_CTX *ctx;
    };

    class EvpDigest {
    public:
        EvpDigest(const EvpDigest &) = delete;
        EvpDigest &operator=(const EvpDigest &) = delete;
        EvpDigest(EvpDigest &&) = delete;
        EvpDigest &operator=(EvpDigest &&) = delete;

        EvpDigest()
        {
            this->ctx = EVP_MD_CTX_create();
        }

        ~EvpDigest()
        {
            if (valid()) [[likely]]
            {
                EVP_MD_CTX_destroy(this->ctx);
                this->ctx = nullptr;
            }
        }

        bool valid() const
        {
            return this->ctx != nullptr;
        }

        int init(const EVP_MD *type, ENGINE *impl = nullptr)
        {
            if (!valid()) [[unlikely]]
            {
                return -1;
            }
            return EVP_DigestInit_ex(this->ctx, type, impl);
        }

        int update(const void *d, const size_t cnt)
        {
            if (!valid()) [[unlikely]]
            {
                return -1;
            }
            return EVP_DigestUpdate(this->ctx, d, cnt);
        }

        int finish(unsigned char *md, unsigned int *s)
        {
            if (!valid()) [[unlikely]]
            {
                return -1;
            }
            return EVP_DigestFinal_ex(this->ctx, md, s);
        }

    private:
        EVP_MD_CTX *ctx;
    };

    Bytes evpHash(BytesView inBytes, const EVP_MD* md) {
        const auto in = inBytes.data();
        const auto inLen = inBytes.size();
        if (in == nullptr || inLen == 0)  [[unlikely]]
        {
            return {};
        }
        unsigned int outLen = EVP_MD_size(md);
        Bytes out(static_cast<size_t>(outLen), 0);

        auto digest = EvpDigest();
        if (!digest.valid()) [[unlikely]]
        {
            dynxxLogPrintF(Error, "EVP_MD_CTX_create failed, err: {}", errMsg());
            return {};
        }

        auto ret = digest.init(md);
        if (ret != OK) [[unlikely]]
        {
            dynxxLogPrintF(Error, "EVP_DigestInit_ex failed, ret: {}, err: {}", ret, errMsg());
            return {};
        }

        ret = digest.update(in, inLen);
        if (ret != OK) [[unlikely]]
        {
            dynxxLogPrintF(Error, "EVP_DigestUpdate failed, ret: {}, err: {}", ret, errMsg());
            return {};
        }

        ret = digest.finish(out.data(), &outLen);
        if (ret != OK) [[unlikely]]
        {
            dynxxLogPrintF(Error, "EVP_DigestFinal_ex failed, ret: {}, err: {}", ret, errMsg());
            return {};
        }

        out.resize(outLen);
        return out;
    }

    struct Base64CharValidator {
        static constexpr size_t charIdx(char c) noexcept {
            //The range of byte(unsigned char) is just [0, 255], same as the size of validTable!
            return static_cast<byte>(c);
        }
        
        static constexpr std::array<bool, 256> createValidTable() noexcept {
            std::array<bool, 256> table{};
            constexpr std::string_view validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
            for (const auto c : validChars) {
                table[charIdx(c)] = true;
            }
            return table;
        }
        
        bool operator()(char c) const noexcept {
            static const auto table = createValidTable();
            return table[charIdx(c)];
        }
    };
}

namespace DynXX::Core::Crypto {

// Rand

Bytes rand(size_t len)
{
    if (len == 0) [[unlikely]]
    {
        return {};
    }
    Bytes out(len, 0);

    static std::once_flag flag;
    std::call_once(flag, []() {
        RAND_poll();
    });
    
    if (RAND_status() != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "RAND_poll failed, status: {}", RAND_status());
    }
    if (const auto ret = RAND_bytes(out.data(), static_cast<int>(len)); ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "RAND_bytes failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }
    return out;
}

// AES

Bytes AES::encrypt(BytesView inBytes, BytesView keyBytes)
{
    if (inBytes.empty() || keyBytes.size() != AES_BLOCK_SIZE) [[unlikely]]
    {
        return {};
    }

    const auto inLen = inBytes.size();
    
    //ECB-PKCS5 Padding:
    auto outLen = inLen;
    const auto paddingSize = AES_BLOCK_SIZE - (inLen % AES_BLOCK_SIZE);
    const auto paddingData = static_cast<byte>(paddingSize);
    outLen += paddingSize;

    Bytes fixedIn(outLen, paddingData);
    std::memcpy(fixedIn.data(), inBytes.data(), inLen);
    Bytes out(outLen, 0);

    AES_KEY aes_key;

    if (const auto ret = AES_set_encrypt_key(keyBytes.data(), AES_Key_BITS, &aes_key); ret != 0) [[unlikely]]
    {
        dynxxLogPrintF(Error, "AES_set_encrypt_key failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }
    
    for (auto offset(0z); offset < outLen; offset += AES_BLOCK_SIZE)
    {
        AES_encrypt(fixedIn.data() + offset, out.data() + offset, &aes_key);
    }

    return out;
}

Bytes AES::decrypt(BytesView inBytes, BytesView keyBytes)
{
    if (inBytes.empty() || keyBytes.size() != AES_BLOCK_SIZE || inBytes.size() % AES_BLOCK_SIZE != 0) [[unlikely]]
    {
        return {};
    }

    const auto inLen = inBytes.size();

    Bytes fixedIn(inLen, 0);
    std::memcpy(fixedIn.data(), inBytes.data(), inLen);
    Bytes out(inLen, 0);

    AES_KEY aes_key;
    if (const auto ret = AES_set_decrypt_key(keyBytes.data(), AES_Key_BITS, &aes_key); ret != 0) [[unlikely]]
    {
        dynxxLogPrintF(Error, "AES_set_decrypt_key failed, ret: {}, err: {}", ret, errMsg());
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

// AES-GCM

Bytes AES::gcmEncrypt(BytesView inBytes, BytesView keyBytes, BytesView initVectorBytes, BytesView aadBytes, size_t tagBits)
{
    if (!AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return {};
    }
    const auto inLen = inBytes.size();
    const auto tagLen = tagBits / 8;

    Bytes tag(tagLen, 0);
    const auto outLen = inLen;
    Bytes out(outLen, 0);

    auto cipher = EvpCipher();
    if (!cipher.valid()) [[unlikely]]
    {
        dynxxLogPrint(Error, "aesGcmEncrypt EVP_CIPHER_CTX_new failed");
        return {};
    }

    auto ret = cipher.init(true, aesGcmCipher(keyBytes.size()), nullptr, nullptr);
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmEncrypt EVP_EncryptInit_ex cipher failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }

    ret = cipher.ctrl(EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(initVectorBytes.size()));
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }

    ret = cipher.init(true, nullptr, keyBytes.data(), initVectorBytes.data());
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmEncrypt EVP_EncryptInit_ex initVector failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }

    int len;

    if (const auto aadLen = aadBytes.size(); aadLen > 0) [[likely]]
    {
        ret = cipher.update(true, nullptr, &len, aadBytes.data(), static_cast<int>(aadLen));
        if (ret != OK) [[unlikely]]
        {
            dynxxLogPrintF(Error, "aesGcmEncrypt EVP_EncryptUpdate aad failed, ret: {}, err: {}", ret, errMsg());
            return {};
        }
    }

    ret = cipher.update(true, out.data(), &len, inBytes.data(), static_cast<int>(inLen));
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmEncrypt EVP_EncryptUpdate encrypt failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }

    ret = cipher.finish(true, out.data(), &len);
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmEncrypt EVP_EncryptFinal_ex encrypt failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }

    ret = cipher.ctrl(EVP_CTRL_GCM_GET_TAG, static_cast<int>(tagLen), tag.data());
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_GET_TAG failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }
    out.insert(out.end(), tag.begin(), tag.end());

    return out;
}

Bytes AES::gcmDecrypt(BytesView inBytes, BytesView keyBytes, BytesView initVectorBytes, BytesView aadBytes, size_t tagBits)
{
    if (!AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return {};
    }
    const auto inLen_ = inBytes.size();
    const auto tagLen = tagBits / 8;

    const auto inLen = inLen_ - tagLen;
    Bytes tag(tagLen, 0);
    std::memcpy(tag.data(), inBytes.data() + inLen, tagLen);
    const auto outLen = inLen;
    Bytes out(outLen, 0);

    auto cipher = EvpCipher();
    if (!cipher.valid()) [[unlikely]]
    {
        dynxxLogPrint(Error, "aesGcmDecrypt EVP_CIPHER_CTX_new failed");
        return {};
    }

    auto ret = cipher.init(false, aesGcmCipher(keyBytes.size()), nullptr, nullptr);
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmDecrypt EVP_DecryptInit_ex cipher failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }

    ret = cipher.ctrl(EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(initVectorBytes.size()));
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }

    ret = cipher.init(false, nullptr, keyBytes.data(), initVectorBytes.data());
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmDecrypt EVP_DecryptInit_ex initVector failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }

    int len;

    if (const auto aadLen = aadBytes.size(); aadLen > 0) [[likely]]
    {
        ret = cipher.update(false, nullptr, &len, aadBytes.data(), static_cast<int>(aadLen));
        if (ret != OK) [[unlikely]]
        {
            dynxxLogPrintF(Error, "aesGcmDecrypt EVP_DecryptUpdate aad failed, ret: {}, err: {}", ret, errMsg());
            return {};
        }
    }

    ret = cipher.update(false, out.data(), &len, inBytes.data(), static_cast<int>(inLen));
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmDecrypt EVP_DecryptUpdate decrypt failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }

    ret = cipher.ctrl(EVP_CTRL_GCM_SET_TAG, static_cast<int>(tagLen), tag.data());
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_TAG failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }

    ret = cipher.finish(false, out.data(), &len);
    if (ret != OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "aesGcmDecrypt EVP_DecryptFinal_ex decrypt failed, ret: {}, err: {}", ret, errMsg());
        return {};
    }

    out.resize(outLen);
    return out;
}

// MD5

Bytes Hash::md5(BytesView inBytes)
{
    const auto md = EVP_md5();
    return evpHash(inBytes, md);
}

// SHA1

Bytes Hash::sha1(BytesView inBytes)
{
    const auto md = EVP_sha1();
    return evpHash(inBytes, md);
}

// SHA256

Bytes Hash::sha256(BytesView inBytes)
{
    const auto md = EVP_sha256();
    return evpHash(inBytes, md);
}

// RSA

std::string RSA::genKey(std::string_view base64, bool isPublic) 
{
    if (base64.empty()) [[unlikely]]
    {
        dynxxLogPrint(Error, "RSA genKey: empty base64 input");
        return {};
    }
    
    const auto cleanedBase64 = dynxxCodingStrTrim(base64);
    if (!Base64::validate(cleanedBase64)) [[unlikely]]
    {
        dynxxLogPrint(Error, "RSA genKey: invalid cleanedBase64");
        return {};
    }
    
    using namespace std::string_view_literals;
    
    constexpr auto ChunkSize = 64uz; 
    constexpr auto DividerS = "-----"sv;
    constexpr auto BeginS = "BEGIN "sv;
    constexpr auto EndS = "END "sv;
    constexpr auto KeyS = " KEY"sv;
    constexpr auto NewLineS = "\n"sv;
    constexpr auto PublicS = "PUBLIC"sv;
    constexpr auto PrivateS = "PRIVATE"sv;

    const auto mid = isPublic ? PublicS : PrivateS;
    const auto numChunks = (cleanedBase64.size() + ChunkSize - 1) / ChunkSize;
    const auto headerSize = DividerS.size() + BeginS.size() + mid.size() + KeyS.size() + DividerS.size() + NewLineS.size(); 
    const auto footerSize = DividerS.size() + EndS.size() + mid.size() + KeyS.size() + DividerS.size() + NewLineS.size(); 
    const auto contentSize = cleanedBase64.size() + numChunks * NewLineS.size();
    
    std::string result; 
    result.reserve(headerSize + contentSize + footerSize); 
    
    result.append(DividerS).append(BeginS).append(mid).append(KeyS).append(DividerS).append(NewLineS); 
    
    for (size_t i = 0; i < cleanedBase64.size(); i += ChunkSize) 
    { 
        const auto remainingSize = cleanedBase64.size() - i;
        const auto currentChunkSize = std::min(ChunkSize, remainingSize);
        result.append(cleanedBase64.substr(i, currentChunkSize)).append(NewLineS); 
    } 
    
    result.append(DividerS).append(EndS).append(mid).append(KeyS).append(DividerS).append(NewLineS); 
    
    return result; 
}

RSA::Codec::Codec(BytesView key, int padding) : padding(padding)
{
    this->bmem = BIO_new_mem_buf(key.data(), static_cast<int>(key.size()));
    if (!this->bmem) [[unlikely]]
    {
        dynxxLogPrintF(Error, "Failed to create BIO mem buffer for RSA, err: {}", errMsg());
    }
}

void RSA::Codec::moveImp(Codec &&other) noexcept
{
    this->padding = other.padding;
    this->bmem = std::exchange(other.bmem, nullptr);
    this->rsa = std::exchange(other.rsa, nullptr);
}

void RSA::Codec::cleanup() noexcept
{
    if (this->bmem != nullptr) [[likely]]
    {
        BIO_free(this->bmem);
        this->bmem = nullptr;
    }
    if (this->rsa != nullptr) [[likely]]
    {
        RSA_free(this->rsa);
        this->rsa = nullptr;
    }
}

RSA::Codec::Codec(Codec &&other) noexcept
    : padding(other.padding)
{
    this->moveImp(std::move(other));
}

RSA::Codec& RSA::Codec::operator=(Codec &&other) noexcept
{
    if (this != &other) [[likely]]
    {
        this->cleanup();
        this->moveImp(std::move(other));
    }
    return *this;
}

RSA::Codec::~Codec()
{
    this->cleanup();
}

std::size_t RSA::Codec::outLen() const
{
    if (this->rsa == nullptr) [[unlikely]]
    {
        return 0;
    }
    return RSA_size(this->rsa);
}

RSA::Encrypt::Encrypt(BytesView key, int padding) : Codec(key, padding)
{
    if (this->bmem == nullptr) [[unlikely]]
    {
        return;
    }
    this->rsa = PEM_read_bio_RSA_PUBKEY(this->bmem, nullptr, nullptr, nullptr);
    if (!this->rsa) [[unlikely]]
    {
        dynxxLogPrintF(Error, "RSA Failed to create public key, err: {}", errMsg());
    }
}

std::optional<Bytes> RSA::Encrypt::process(BytesView in) const
{
    if (this->rsa == nullptr) [[unlikely]]
    {
        return std::nullopt;
    }
    Bytes outBytes(this->outLen(), 0);
    if (const auto ret = RSA_public_encrypt(static_cast<int>(in.size()), in.data(), outBytes.data(), this->rsa, this->padding); ret == -1) [[unlikely]]
    {
        dynxxLogPrintF(Error, "RSA encrypt failed, ret: {}, err: {}", ret, errMsg());
        return std::nullopt;
    }
    return {outBytes};
}

RSA::Decrypt::Decrypt(BytesView key, int padding) : Codec(key, padding)
{
    if (this->bmem == nullptr) [[unlikely]]
    {
        return;
    }
    this->rsa = PEM_read_bio_RSAPrivateKey(this->bmem, nullptr, nullptr, nullptr);
    if (!this->rsa) [[unlikely]]
    {
        dynxxLogPrintF(Error, "RSA Failed to create private key, err: {}", errMsg());
    }
}

std::optional<Bytes> RSA::Decrypt::process(BytesView in) const
{
    if (this->rsa == nullptr) [[unlikely]]
    {
        return std::nullopt;
    }
    Bytes outBytes(this->outLen(), 0);
    if (const auto ret = RSA_private_decrypt(static_cast<int>(in.size()), in.data(), outBytes.data(), this->rsa, this->padding); ret == -1) [[unlikely]]
    {
        dynxxLogPrintF(Error, "RSA decrypt failed, ret: {}, err: {}", ret, errMsg());
        return std::nullopt;
    }
    return {outBytes};
}

// Base64

bool Base64::validate(std::string_view in)
{
    if (in.empty() || in.size() % 4 != 0) [[unlikely]]
    {
        return false;
    }
    
    static constexpr Base64CharValidator validator{};
    
#if defined(__cpp_lib_ranges)
    return std::ranges::all_of(in, validator);
#else
    for (const auto c : in)
    {
        if (!validator(c)) [[unlikely]]
        {
            return false;
        }
    }
    return true;
#endif
}

Bytes Base64::encode(BytesView inBytes, bool noNewLines)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }

    const auto b64 = BIO_new(BIO_f_base64());
    if (!b64) [[unlikely]]
    {
        dynxxLogPrintF(Error, "Failed to create BIO for Base64, err: {}", errMsg());
        return {};
    }

    const auto bsmem = BIO_new(BIO_s_mem());
    if (!bsmem) [[unlikely]]
    {
        dynxxLogPrintF(Error, "Failed to create BIO mem for Base64, err: {}", errMsg());
        BIO_free(b64);
        return {};
    }

    if (noNewLines)
    {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }
    
    const auto bpush = BIO_push(b64, bsmem);
    if (!bpush) [[unlikely]]
    {
        dynxxLogPrintF(Error, "Failed to push BIO mem for Base64, err: {}", errMsg());
        BIO_free_all(b64);
        return {};
    }

    if (const auto writeResult = BIO_write(b64, in, static_cast<int>(inLen)); writeResult <= 0) [[unlikely]]
    {
        dynxxLogPrintF(Error, "Failed to write to Base64 BIO, err: {}", errMsg());
        BIO_free_all(b64);
        return {};
    }

    if (const auto flushResult = BIO_flush(b64); flushResult <= 0) [[unlikely]]
    {
        dynxxLogPrintF(Error, "Failed to flush Base64 BIO, err: {}", errMsg());
        BIO_free_all(b64);
        return {};
    }

    char *outBytes = nullptr;
    const auto outLen = BIO_get_mem_data(bpush, &outBytes);
    if (outLen <= 0 || outBytes == nullptr) [[unlikely]]
    {
        dynxxLogPrintF(Error, "Failed to get Base64 encoded data, err: {}", errMsg());
        BIO_free_all(b64);
        return {};
    }

    // Copy data before freeing BIO to avoid potential memory issues
    Bytes out(outLen);
    std::memcpy(out.data(), outBytes, outLen);

    BIO_free_all(b64);

    return out;
}

Bytes Base64::decode(BytesView inBytes, bool noNewLines)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }

    // Validate Base64 characters
    for (size_t i = 0; i < inLen; i++)
    {
        if (std::isalnum(in[i]) || in[i] == '+' || in[i] == '/' || in[i] == '=')
        {
            continue;
        }
        dynxxLogPrintF(Error, "Invalid Base64 char: {}", in[i]);
        return {};
    }

    // Create memory BIO with explicit length
    const auto bmem = BIO_new_mem_buf(in, static_cast<int>(inLen));
    if (!bmem) [[unlikely]]
    {
        dynxxLogPrintF(Error, "Failed to create BIO mem buffer for Base64, err: {}", errMsg());
        return {};
    }

    const auto b64 = BIO_new(BIO_f_base64());
    if (!b64) [[unlikely]]
    {
        dynxxLogPrintF(Error, "Failed to create BIO for Base64, err: {}", errMsg());
        BIO_free_all(bmem);
        return {};
    }

    const auto bpush = BIO_push(b64, bmem);
    if (!bpush) [[unlikely]]
    {
        dynxxLogPrintF(Error, "Failed to push BIO mem for Base64, err: {}", errMsg());
        BIO_free_all(b64);
        return {};
    }

    if (noNewLines)
    {
        BIO_set_flags(bpush, BIO_FLAGS_BASE64_NO_NL);
    }

    auto outLen = (inLen * 3) / 4 + 3; // Add padding for safety
    Bytes outBytes(outLen, 0);
    
    const auto bytesRead = BIO_read(bpush, outBytes.data(), static_cast<int>(outLen));
    if (bytesRead <= 0) [[unlikely]]
    {
        dynxxLogPrintF(Error, "Failed to decode Base64, err: {}", errMsg());
        BIO_free_all(bpush);
        return {};
    }

    BIO_free_all(bpush);
    outBytes.resize(bytesRead);
    return outBytes;
}

} // namespace DynXX::Core::Crypto