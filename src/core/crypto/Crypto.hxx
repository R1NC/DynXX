#pragma once

#include <random>

#include <openssl/bio.h>
#include <openssl/rsa.h>

#include <DynXX/CXX/Types.hxx>
#include <DynXX/CXX/Crypto.hxx>

namespace DynXX::Core::Crypto {
    Bytes rand(size_t len);

    template<NumberT T>
    std::optional<T> rand(T min, T max) {
        if (min > max || max <= 0) {
            return std::nullopt;
        }

        static std::mt19937 gen = []() {
            std::random_device rd;
            std::seed_seq seed{
                rd(), rd(), rd(), rd(),
                rd(), rd(), rd(), rd(), 
                rd(), rd(), rd(), rd()
            };
            return std::mt19937{seed};
        }();

        std::uniform_int_distribution<T> dist(min, max);
        return dist(gen);
    }

    namespace AES {
        Bytes encrypt(BytesView in, BytesView key);

        Bytes decrypt(BytesView in, BytesView key);

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
        
            [[nodiscard]] virtual std::optional<Bytes> process(BytesView in) const = 0;
        
            [[nodiscard]] size_t outLen() const;
        
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
        
            [[nodiscard]] std::optional<Bytes> process(BytesView in) const override;
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
        
            [[nodiscard]] std::optional<Bytes> process(BytesView in) const override;
        };
    }  // namespace RSA

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
}  // namespace DynXX::Core::Crypto
