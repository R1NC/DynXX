#ifndef DYNXX_SRC_CORE_STORE_CONN_POOL_HXX_
#define DYNXX_SRC_CORE_STORE_CONN_POOL_HXX_

#if defined(__cplusplus)

#include <memory>
#include <mutex>
#include <shared_mutex>
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

        ConnPool(const ConnPool &) = delete;

        ConnPool &operator=(const ConnPool &) = delete;

        ConnPool(ConnPool &&) = delete;

        ConnPool &operator=(ConnPool &&) = delete;

        void close(const CidT cid) {
            auto lock = std::scoped_lock(this->mutex);
            auto it = this->conns.find(cid);
            if (it != this->conns.end()) {
                it->second.reset();
                this->conns.erase(it);
            }
        }

        void closeAll() {
            auto lock = std::scoped_lock(this->mutex);
            for (auto &[_, v] : this->conns) 
            {
                v.reset();
            }
            this->conns.clear();
        }

        ~ConnPool() {
            this->closeAll();
        }

    protected:
        std::weak_ptr<ConnT> open(const CidT cid, std::function<std::shared_ptr<ConnT>()> &&creatorF) {
            auto lock = std::scoped_lock(this->mutex);
            
            if (auto it = this->conns.find(cid); it != this->conns.end() && it->second) {
                return it->second;
            }
            
            if (auto conn = std::move(creatorF)(); conn) {
                this->conns.emplace(cid, conn);
                return conn;
            }
            
            return {};
        }
    
    private:
        std::mutex mutex;
        std::unordered_map<CidT, std::shared_ptr<ConnT>> conns;
    };
}

#endif

#endif // DYNXX_SRC_CORE_STORE_CONN_POOL_HXX_
