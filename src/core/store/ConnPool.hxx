#pragma once

#include <memory>
#include <mutex>
#include <functional>

#include <DynXX/CXX/Types.hxx>

namespace DynXX::Core::Store {

    using CidT = size_t;

    inline CidT genCid(DictKeyT key) {
        return std::hash<DictKeyT>{}(key);
    }

    template<typename ConnT>
    class ConnPool {
    public:
        ConnPool() = default;

        void close(const CidT cid) {
            const auto lock = std::scoped_lock(this->mutex);
            if (auto it = this->conns.find(cid); it != this->conns.end()) {
                it->second.reset();
                this->conns.erase(it);
            }
        }

        void closeAll() {
            const auto lock = std::scoped_lock(this->mutex);
            for (auto &[_, v] : this->conns) 
            {
                v.reset();
            }
            this->conns.clear();
        }

        ~ConnPool() {
            this->closeAll();
        }

        ConnPool(const ConnPool &) = delete;
        ConnPool &operator=(const ConnPool &) = delete;
        ConnPool(ConnPool &&) = delete;
        ConnPool &operator=(ConnPool &&) = delete;

    protected:
        std::weak_ptr<ConnT> open(const CidT cid, std::function<std::shared_ptr<ConnT>()> &&creatorF) {
            const auto lock = std::scoped_lock(this->mutex);
            
            if (auto it = this->conns.find(cid); it != this->conns.end() && it->second) {
                return it->second;
            }
            
            if (const auto conn = std::move(creatorF)(); conn) {
                this->conns.emplace(cid, conn);
                return conn;
            }
            
            return {};
        }
    
    private:
        std::mutex mutex;
        std::unordered_map<CidT, std::shared_ptr<ConnT>> conns;
    };
}  // namespace DynXX::Core::Store
