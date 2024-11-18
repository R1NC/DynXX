#ifndef NGENXX_STORE_KV_HXX_
#define NGENXX_STORE_KV_HXX_

#ifdef __cplusplus

#include <string>
#include <map>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

#include "../../external/mmkv/Core/MMKV.h"
#include "../../include/NGenXXTypes.hxx"

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
                mutable std::shared_mutex mutex;

            public:
                Connection(const std::string &_id);

                const std::string readString(const std::string &k);

                const int64_t readInteger(const std::string &k);

                const double readFloat(const std::string &k);
                
                bool write(const std::string &k, const Any &v);

                const std::vector<std::string> allKeys();

                bool contains(const std::string &k);

                bool remove(const std::string &k);

                void clear();

                ~Connection();
            };

            Connection *open(const std::string &_id);

            void closeAll();

            ~KV();

        private:
            std::unordered_map<std::string, Connection*> conns;
            std::mutex mutex;
        };
    }
}

#endif

#endif // NGENXX_STORE_KV_HXX_