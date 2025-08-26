#if defined(USE_KV)

#include "KV.hxx"

#include <DynXX/CXX/Log.hxx>

DynXX::Core::Store::KV::KVStore::KVStore(const std::string &root)
{
    decltype(root) sRoot(root);
    auto logLevel = MMKVLogNone;
#if defined(_WIN32)
    MMKV::initializeMMKV(string2MMKVPath_t(sRoot), logLevel);
#else
    MMKV::initializeMMKV(sRoot, logLevel);
#endif
}

std::weak_ptr<DynXX::Core::Store::KV::Connection> DynXX::Core::Store::KV::KVStore::open(const std::string &_id)
{
    return ConnPool<KV::Connection>::open(_id, [&_id]() { 
        return std::make_shared<KV::Connection>(_id); 
    });
}

void DynXX::Core::Store::KV::KVStore::close(const std::string &_id)
{
    ConnPool<KV::Connection>::close(_id);
}

DynXX::Core::Store::KV::KVStore::~KVStore()
{
    MMKV::onExit();
}

DynXX::Core::Store::KV::Connection::Connection(const std::string &_id) : _id(_id)
{
    this->kv = MMKV::mmkvWithID(_id, MMKV_MULTI_PROCESS);
    MMKV::setLogLevel(MMKVLogNone);
}

std::optional<std::string> DynXX::Core::Store::KV::Connection::readString(std::string_view k) const
{
    auto lock = std::shared_lock(this->mutex);
    if (!this->kv->containsKey(k)) [[unlikely]]
    {
        return std::nullopt;
    }
    std::string s;
    this->kv->getString(k, s);
    return s;
}

std::optional<int64_t> DynXX::Core::Store::KV::Connection::readInteger(std::string_view k) const
{
    auto lock = std::shared_lock(this->mutex);
    if (!this->kv->containsKey(k)) [[unlikely]]
    {
        return std::nullopt;
    }
    return {this->kv->getInt64(k)};
}

std::optional<double> DynXX::Core::Store::KV::Connection::readFloat(std::string_view k) const
{
    auto lock = std::shared_lock(this->mutex);
    if (!this->kv->containsKey(k)) [[unlikely]]
    {
        return std::nullopt;
    }
    return {this->kv->getDouble(k)};
}

bool DynXX::Core::Store::KV::Connection::write(std::string_view k, const Any &v) const
{
    auto lock = std::unique_lock(this->mutex);
    return std::visit(
        [k, kv = this->kv](const auto &x)
        { 
            return kv->set(x, k); 
        },
        v
    );
}

std::vector<std::string> DynXX::Core::Store::KV::Connection::allKeys() const
{
    auto lock = std::shared_lock(this->mutex);
    return this->kv->allKeys(false);
}

bool DynXX::Core::Store::KV::Connection::contains(std::string_view k) const
{
    auto lock = std::shared_lock(this->mutex);
    return this->kv->containsKey(k);
}

bool DynXX::Core::Store::KV::Connection::remove(std::string_view k) const
{
    auto lock = std::unique_lock(this->mutex);
    return this->kv->removeValueForKey(k);
}

void DynXX::Core::Store::KV::Connection::clear() const
{
    auto lock = std::unique_lock(this->mutex);
    this->kv->clearAll();
    this->kv->clearMemoryCache();
}

DynXX::Core::Store::KV::Connection::~Connection()
{
    this->kv->close();
    this->kv = nullptr;
}

#endif
