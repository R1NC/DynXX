#if defined(USE_KV)

#include "KV.hxx"

#include <DynXX/CXX/Log.hxx>

namespace {
    using enum DynXXLogLevelX;
}

namespace DynXX::Core::Store::KV {

KVStore::KVStore(const std::string &root)
{
    decltype(root) sRoot(root);
    auto logLevel = MMKVLogNone;
#if defined(_WIN32)
    MMKV::initializeMMKV(string2MMKVPath_t(sRoot), logLevel);
#else
    MMKV::initializeMMKV(sRoot, logLevel);
#endif
}

std::weak_ptr<Connection> KVStore::open(const std::string &_id)
{
    const auto cid = genCid(_id);
    return ConnPool<Connection>::open(cid, [cid, &_id]() { 
        const auto kv = MMKV::mmkvWithID(_id, MMKV_MULTI_PROCESS);
        if (!kv) [[unlikely]]
        {
            dynxxLogPrint(Error, "MMKV mmkvWithID failed");
            return std::shared_ptr<Connection>(nullptr);
        }
        return std::make_shared<Connection>(cid, kv); 
    });
}

KVStore::~KVStore()
{
    MMKV::onExit();
}

Connection::Connection(const CidT cid, MMKV *kv) : _cid(cid), kv(kv)
{
    MMKV::setLogLevel(MMKVLogNone);
}

std::optional<std::string> Connection::readString(std::string_view k) const
{
    const auto lock = std::shared_lock(this->mutex);
    if (!this->kv->containsKey(k)) [[unlikely]]
    {
        return std::nullopt;
    }
    std::string s;
    this->kv->getString(k, s);
    return s;
}

std::optional<int64_t> Connection::readInteger(std::string_view k) const
{
    const auto lock = std::shared_lock(this->mutex);
    if (!this->kv->containsKey(k)) [[unlikely]]
    {
        return std::nullopt;
    }
    return {this->kv->getInt64(k)};
}

std::optional<double> Connection::readFloat(std::string_view k) const
{
    const auto lock = std::shared_lock(this->mutex);
    if (!this->kv->containsKey(k)) [[unlikely]]
    {
        return std::nullopt;
    }
    return {this->kv->getDouble(k)};
}

bool Connection::write(std::string_view k, const Any &v) const
{
    const auto lock = std::unique_lock(this->mutex);
    return std::visit(
        [k, kv = this->kv](const auto &x)
        { 
            return kv->set(x, k); 
        },
        v
    );
}

std::vector<std::string> Connection::allKeys() const
{
    const auto lock = std::shared_lock(this->mutex);
    return this->kv->allKeys(false);
}

bool Connection::contains(std::string_view k) const
{
    const auto lock = std::shared_lock(this->mutex);
    return this->kv->containsKey(k);
}

bool Connection::remove(std::string_view k) const
{
    const auto lock = std::unique_lock(this->mutex);
    return this->kv->removeValueForKey(k);
}

void Connection::clear() const
{
    const auto lock = std::unique_lock(this->mutex);
    this->kv->clearAll();
    this->kv->clearMemoryCache();
}

Connection::~Connection()
{
    this->kv->close();
    this->kv = nullptr;
}

} // namespace DynXX::Core::Store::KV

#endif
