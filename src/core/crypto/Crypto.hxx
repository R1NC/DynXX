#ifndef DYNXX_SRC_CORE_CRYPTO_HXX_
#define DYNXX_SRC_CORE_CRYPTO_HXX_

#if defined(__cplusplus)

#include <random>
#include <mutex>
#include <functional>

#include <openssl/bio.h>
#include <openssl/rsa.h>

#include <DynXX/CXX/Types.hxx>
#include <DynXX/CXX/Log.hxx>
#include <DynXX/CXX/Crypto.hxx>

#include "../concurrent/ConcurrentUtil.hxx"

namespace DynXX::Core::Crypto {
    Bytes rand(size_t len);

    template<NumberT T>
    std::optional<T> rand(T min, T max) {
        if (min > max || max <= 0) {
            return std::nullopt;
        }

        static std::mt19937 generator;
        static std::once_flag flag;
        std::call_once(flag, []() {
            std::random_device rd;
            generator.seed(rd());
        });

        std::uniform_int_distribution<T> distribution(min, max);
        return {distribution(generator)};
    }

    namespace AES {
        Bytes encrypt(BytesView in, BytesView key);

        Bytes decrypt(BytesView in, BytesView key);

        static constexpr auto checkGcmParams(BytesView in, BytesView key, BytesView initVector,
                                             BytesView aad, const size_t tagBits) {
            using enum DynXXLogLevelX;
            const auto inLen = in.size();
            if (in.empty()) {
                dynxxLogPrint(Error, "aesGcm invalid inBytes");
                return false;
            }
            if (const auto keyLen = key.size(); keyLen % 8 != 0 || keyLen < 16 || keyLen > 32) {
                dynxxLogPrint(Error, "aesGcm invalid keyBytes");
                return false;
            }
            if (const auto initVectorLen = initVector.size(); initVectorLen != 12) {
                dynxxLogPrint(Error, "aesGcm invalid initVectorBytes");
                return false;
            }
            if (const auto aadLen = aad.size(); aadLen > 16) {
                dynxxLogPrint(Error, "aesGcm invalid aadBytes");
                return false;
            }
            if (tagBits % 8 != 0 || tagBits / 8 >= inLen || tagBits < 96 || tagBits > 128) {
                dynxxLogPrint(Error, "aesGcm invalid tagBits");
                return false;
            }
            return true;
        }

        Bytes gcmEncrypt(BytesView in, BytesView key, BytesView initVector, BytesView aad, size_t tagBits);

        Bytes gcmDecrypt(BytesView in, BytesView key, BytesView initVector, BytesView aad, size_t tagBits);
    }

    namespace RSA {
        std::string genKey(std::string_view base64, bool isPublic);
        
        class Codec
        {
        public:
            Codec() = delete;
            Codec(const Codec &) = delete;
            Codec &operator=(const Codec &) = delete;
            Codec(Codec &&other) noexcept;
            Codec &operator=(Codec &&other) noexcept;
        
            explicit Codec(BytesView key, DynXXCryptoRSAPaddingX padding);
        
            virtual std::optional<Bytes> process(BytesView in) const = 0;
        
            size_t outLen() const;
        
            virtual ~Codec();
        
        protected:
            void moveImp(Codec &&other) noexcept;
            void cleanup() noexcept;
            
            BIO *bmem{nullptr};
            rsa_st *rsa{nullptr};
            DynXXCryptoRSAPaddingX padding;
        };
    
        class Encrypt final : public Codec
        {
        public:
            Encrypt() = delete;
            Encrypt(const Encrypt &) = delete;
            Encrypt &operator=(const Encrypt &) = delete;
            Encrypt(Encrypt &&other) noexcept = default;
            Encrypt &operator=(Encrypt &&other) noexcept = default;
            ~Encrypt() override = default;
        
            explicit Encrypt(BytesView key, DynXXCryptoRSAPaddingX padding);
        
            std::optional<Bytes> process(BytesView in) const override;
        };
    
        class Decrypt final : public Codec
        {
        public:
            Decrypt() = delete;
            Decrypt(const Decrypt &) = delete;
            Decrypt &operator=(const Decrypt &) = delete;
            Decrypt(Decrypt &&other) noexcept = default;
            Decrypt &operator=(Decrypt &&other) noexcept = default;
            ~Decrypt() override = default;
        
            explicit Decrypt(BytesView key, DynXXCryptoRSAPaddingX padding);
        
            std::optional<Bytes> process(BytesView in) const override;
        };
    }

    namespace Hash {
        Bytes md5(BytesView in);

        Bytes sha1(BytesView in);

        Bytes sha256(BytesView in);
    }

    namespace Base64 {
        bool validate(std::string_view in);
        
        Bytes encode(BytesView in, bool noNewLines = true);

        Bytes decode(BytesView in, bool noNewLines = true);
    }
}

#endif

#endif // DYNXX_SRC_CORE_CRYPTO_HXX_
