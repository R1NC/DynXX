#pragma once

#include <MMKV.h>

#include <shared_mutex>
#include <memory>

#include <DynXX/CXX/Types.hxx>

#include "ConnPool.hxx"

namespace DynXX::Core::Store::KV {

    class Connection {
        public:
            Connection() = delete;

            explicit Connection(CidT cid, std::string_view _id);

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

            ~Connection() = default;

        private:
            const CidT _cid{0};
            mutable std::shared_mutex mutex;

            struct KVDeleter final {
                void operator()(MMKV *kv) const noexcept {
                    if (kv != nullptr) {
                        kv->close();
                    }
                }
            };
            std::unique_ptr<MMKV, KVDeleter> kv;
    };

    class KVStore : public ConnPool<KV::Connection> {
    public:
        KVStore() = delete;
        KVStore(const KVStore &) = delete;
        KVStore &operator=(const KVStore &) = delete;
        KVStore(KVStore &&) = delete;
        KVStore &operator=(KVStore &&) = delete;

        explicit KVStore(std::string_view root);

        std::weak_ptr<KV::Connection> open(std::string_view _id);

        ~KVStore();
    };
}  // namespace DynXX::Core::Store::KV
