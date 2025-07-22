#ifndef NGENXX_SRC_CORE_CRYPTO_HXX_
#define NGENXX_SRC_CORE_CRYPTO_HXX_

#if defined(__cplusplus)

#include <random>

#include <openssl/bio.h>
#include <openssl/rsa.h>

#include <NGenXXTypes.hxx>

#include "../concurrent/ConcurrentUtil.hxx"

namespace NGenXX::Core::Crypto {
    bool rand(size_t len, byte *bytes);

    template<NumberT T>
    std::optional<T> rand(T min, T max) {
        if (min > max || max <= 0) {
            return std::nullopt;
        }

        static std::mt19937 generator;
        Concurrent::callOnce([]() {
            std::random_device rd;
            generator.seed(rd());
        });

        std::uniform_int_distribution<T> distribution(min, max);
        return std::make_optional(distribution(generator));
    }

    namespace AES {
        Bytes encrypt(BytesView in, BytesView key);

        Bytes decrypt(BytesView in, BytesView key);

        static constexpr auto checkGcmParams(BytesView in, BytesView key, BytesView initVector,
                                             BytesView aad, const size_t tagBits) {
            const auto inBytes = in.data(), keyBytes = key.data(), initVectorBytes = initVector.data(), aadBytes = aad.
                    data();
            const auto inLen = in.size(), keyLen = key.size(), initVectorLen = initVector.size(), aadLen = aad.size();
            const auto tagLen = tagBits / 8;
            if (inBytes == nullptr || inLen == 0) {
                return false;
            }
            if (keyBytes == nullptr || (keyLen != 16 && keyLen != 24 && keyLen != 32)) {
                return false;
            }
            if (initVectorBytes == nullptr || initVectorLen != 12) {
                return false;
            }
            if (aadLen > 16 || (aadLen > 0 && aadBytes == nullptr)) {
                return false;
            }
            if (tagBits != 96 && tagBits != 104 && tagBits != 112 && tagBits != 120 && tagBits != 128) {
                return false;
            }
            if (inLen <= tagLen) {
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
        
            explicit Codec(BytesView key, int padding);
        
            virtual std::optional<Bytes> process(BytesView in) const = 0;
        
            size_t outLen() const;
        
            virtual ~Codec();
        
        protected:
            void moveImp(Codec &&other) noexcept;
            void cleanup() noexcept;
            
            BIO *bmem{nullptr};
            rsa_st *rsa{nullptr};
            const int padding;
        };
    
        class Encrypt final : public Codec
        {
        public:
            Encrypt() = delete;
            Encrypt(const Encrypt &) = delete;
            Encrypt &operator=(const Encrypt &) = delete;
            Encrypt(Encrypt &&other) noexcept = default;
            Encrypt &operator=(Encrypt &&other) noexcept = default;
            ~Encrypt() = default;
        
            explicit Encrypt(BytesView key, int padding);
        
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
            ~Decrypt() = default;
        
            explicit Decrypt(BytesView key, int padding);
        
            std::optional<Bytes> process(BytesView in) const override;
        };
    }

    namespace Hash {
        Bytes md5(BytesView in);

        Bytes sha1(BytesView in);

        Bytes sha256(BytesView in);
    }

    namespace Base64 {
        Bytes encode(BytesView in, bool noNewLines = true);

        Bytes decode(BytesView in, bool noNewLines = true);
    }
}

#endif

#endif // NGENXX_SRC_CORE_CRYPTO_HXX_
