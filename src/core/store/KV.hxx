#ifndef DYNXX_SRC_CORE_STORE_KV_HXX_
#define DYNXX_SRC_CORE_STORE_KV_HXX_

#if defined(__cplusplus)

#include <unordered_map>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include <MMKV.h>

#include <DynXX/CXX/Types.hxx>

namespace DynXX::Core::Store {
    class KV {
    public:
        KV() = delete;

        explicit KV(const std::string &root);

        KV(const KV &) = delete;

        KV &operator=(const KV &) = delete;

        KV(KV &&) = delete;

        KV &operator=(KV &&) = delete;

        class Connection {
        public:
            Connection() = delete;

            explicit Connection(const std::string &_id);

            Connection(const Connection &) = delete;

            Connection &operator=(const Connection &) = delete;

            Connection(Connection &&) = delete;

            Connection &operator=(Connection &&) = delete;

            std::optional<std::string> readString(std::string_view k) const;

            std::optional<int64_t> readInteger(std::string_view k) const;

            std::optional<double> readFloat(std::string_view k) const;

            [[nodiscard]] bool write(std::string_view k, const Any &v) const;

            std::vector<std::string> allKeys() const;

            bool contains(std::string_view k) const;

            [[nodiscard]] bool remove(std::string_view k) const;

            void clear() const;

            ~Connection();

        private:
            MMKV *kv{nullptr};
            mutable std::shared_mutex mutex;
        };

        std::weak_ptr<Connection> open(const std::string &_id);

        void close(const std::string &_id);

        void closeAll();

        ~KV();

    private:
        std::unordered_map<std::string, std::shared_ptr<Connection> > conns;
        std::mutex mutex;
    };
}

#endif

#endif // DYNXX_SRC_STORE_KV_HXX_
