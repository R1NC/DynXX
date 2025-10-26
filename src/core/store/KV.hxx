#pragma once

#include <MMKV.h>

#include <shared_mutex>

#include <DynXX/CXX/Types.hxx>

#include "ConnPool.hxx"

namespace DynXX::Core::Store::KV {

    class Connection {
        public:
            Connection() = default;

            explicit Connection(CidT cid, MMKV *kv);

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

            CidT cid() const { return this->_cid; }

            ~Connection();

        private:
            const CidT _cid{0};
            MMKV *kv{nullptr};
            mutable std::shared_mutex mutex;
    };

    class KVStore : public ConnPool<KV::Connection> {
    public:
        KVStore() = delete;
        KVStore(const KVStore &) = delete;
        KVStore &operator=(const KVStore &) = delete;
        KVStore(KVStore &&) = delete;
        KVStore &operator=(KVStore &&) = delete;

        explicit KVStore(const std::string &root);

        std::weak_ptr<KV::Connection> open(const std::string &_id);

        ~KVStore();
    };
}  // namespace DynXX::Core::Store::KV
