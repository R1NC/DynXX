#pragma once

#include <random>

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

        Bytes encrypt(BytesView in, BytesView key, DynXXCryptoRSAPaddingX padding);

        Bytes decrypt(BytesView in, BytesView key, DynXXCryptoRSAPaddingX padding);
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
}  // namespace DynXX::Core::Crypto
