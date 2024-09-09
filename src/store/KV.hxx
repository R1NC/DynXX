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

        public:
            KV(const std::string &root);

            class Connection
            {
            private:
                MMKV *kv;

            public:
                Connection(const std::string &_id);

                const std::string readString(const std::string &k);

                long long readInteger(const std::string &k);

                double readFloat(const std::string &k);

                template <typename T>
                bool write(const std::string &k, T v)
                {
                    return this->kv->set(v, k);
                }

                bool contains(const std::string &k);

                void clear();

                ~Connection();
            };

            Connection *open(const std::string &_id);

            ~KV();
        };
    }
}

#endif

#endif // NGENXX_STORE_KV_HXX_