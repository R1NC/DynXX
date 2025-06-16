#ifndef NGENXX_SRC_CORE_CRYPTO_HXX_
#define NGENXX_SRC_CORE_CRYPTO_HXX_

#if defined(__cplusplus)

#include <random>

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
        Bytes encrypt(const Bytes &in, const Bytes &key);

        Bytes decrypt(const Bytes &in, const Bytes &key);

        static constexpr auto checkGcmParams(const Bytes &in, const Bytes &key, const Bytes &initVector,
                                             const Bytes &aad, const size_t tagBits) {
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

        Bytes gcmEncrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, const Bytes &aad, size_t tagBits);

        Bytes gcmDecrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, const Bytes &aad, size_t tagBits);
    }

    namespace RSA {
        Bytes encrypt(const Bytes &in, const Bytes &key, int padding);

        Bytes decrypt(const Bytes &in, const Bytes &key, int padding);
    }

    namespace Hash {
        Bytes md5(const Bytes &in);

        Bytes sha1(const Bytes &in);

        Bytes sha256(const Bytes &in);
    }

    namespace Base64 {
        Bytes encode(const Bytes &in);

        Bytes decode(const Bytes &in);
    }
}

#endif

#endif // NGENXX_SRC_CORE_CRYPTO_HXX_
