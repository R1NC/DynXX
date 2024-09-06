#ifndef NGENXX_STORE_KV_HXX_
#define NGENXX_STORE_KV_HXX_

#ifdef __cplusplus

#include <string>
#include "../../external/mmkv/Core/MMKV.h"

namespace NGenXX
{
    namespace Store
    {
        class KV
        {
        private:
            MMKV *kv;

        public:
            KV(const std::string &uid);

            const std::string readString(const std::string &k);

            bool writeString(const std::string &k, const std::string &v);

            long long readInteger(const std::string &k);

            bool writeInteger(const std::string &k, long long v);

            double readFloat(const std::string &k);

            bool writeFloat(const std::string &k, double v);

            bool contains(const std::string &k);

            void clear();

            ~KV();
        };
    }
}

#endif

#endif // NGENXX_STORE_KV_HXX_