#ifndef NGENXX_STORE_KV_HXX_
#define NGENXX_STORE_KV_HXX_

#ifdef __cplusplus

#include <string>
#include "../../external/mmkv/Core/MMKV.h"
#include "../NGenXX-Types.hxx"

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

                const int64_t readInteger(const std::string &k);

                const double readFloat(const std::string &k);
                
                bool write(const std::string &k, const Any &v);

                const std::vector<std::string> allKeys();

                bool contains(const std::string &k);

                void remove(const std::string &k);

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