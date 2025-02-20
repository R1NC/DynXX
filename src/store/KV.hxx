#ifndef NGENXX_SRC_STORE_KV_HXX_
#define NGENXX_SRC_STORE_KV_HXX_

#if defined(__cplusplus)

#include <unordered_map>
#include <mutex>
#include <shared_mutex>

#include <MMKV.h>

#include <NGenXXTypes.hxx>

namespace NGenXX
{
    namespace Store
    {
        class KV
        {
        public:
            KV() = delete;
            explicit KV(const std::string &root);
            KV(const KV &) = delete;
            KV &operator=(const KV &) = delete;
            KV(KV &&) = delete;
            KV &operator=(KV &&) = delete;

            class Connection
            {
            public:
                Connection() = delete;
                explicit Connection(const std::string &_id);
                Connection(const Connection &) = delete;
                Connection &operator=(const Connection &) = delete;
                Connection(Connection &&) = delete;
                Connection &operator=(Connection &&) = delete;

                std::string readString(const std::string &k) const;

                int64_t readInteger(const std::string &k) const;

                double readFloat(const std::string &k) const;

                [[nodiscard]] bool write(const std::string &k, const Any &v) const;

                std::vector<std::string> allKeys() const;

                bool contains(const std::string &k) const;

                [[nodiscard]] bool remove(const std::string &k) const;

                void clear() const;

                ~Connection();

            private:
                MMKV *kv{nullptr};
                mutable std::shared_mutex mutex;
            };

            Connection *open(const std::string &_id);

            void close(const std::string &_id);

            void closeAll();

            ~KV();

        private:
            std::unordered_map<std::string, Connection *> conns;
            std::mutex mutex;
        };
    }
}

#endif

#endif // NGENXX_SRC_STORE_KV_HXX_