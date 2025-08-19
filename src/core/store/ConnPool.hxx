#ifndef DYNXX_SRC_CORE_STORE_CONN_POOL_HXX_
#define DYNXX_SRC_CORE_STORE_CONN_POOL_HXX_

#if defined(__cplusplus)

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <functional>

#include <DynXX/CXX/Types.hxx>

namespace DynXX::Core::Store {

    template<typename ConnT>
    class ConnPool {
    public:
        ConnPool() = default;

        ConnPool(const ConnPool &) = delete;

        ConnPool &operator=(const ConnPool &) = delete;

        ConnPool(ConnPool &&) = delete;

        ConnPool &operator=(ConnPool &&) = delete;

        void close(const std::string &_id) {
            auto lock = std::lock_guard(this->mutex);
            auto it = this->conns.find(_id);
            if (it != this->conns.end()) {
                it->second.reset();
                this->conns.erase(it);
            }
        }

        void closeAll() {
            auto lock = std::lock_guard(this->mutex);
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
        std::weak_ptr<ConnT> open(const std::string &_id, std::function<std::shared_ptr<ConnT>()> &&creatorF) {
            auto lock = std::lock_guard(this->mutex);
            
            auto it = this->conns.find(_id);
            if (it != this->conns.end()) {
                return it->second;
            }
            
            try {
                auto conn = creatorF();
                if (!conn) [[unlikely]] {
                    return {};
                }
                this->conns.emplace(_id, conn);
                return conn;
            } catch (...) {
                return {};
            }
        }
    
    private:
        std::unordered_map<std::string, std::shared_ptr<ConnT> > conns;
        std::mutex mutex;
    };
}

#endif

#endif // DYNXX_SRC_CORE_STORE_CONN_POOL_HXX_
