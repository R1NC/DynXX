#if defined(USE_KV)

#include "KV.hxx"

#include <DynXX/CXX/Log.hxx>

namespace {
    using enum DynXXLogLevelX;
}

namespace DynXX::Core::Store::KV {

KVStore::KVStore(std::string_view root)
{
    if (root.empty()) [[unlikely]]
    {
        dynxxLogPrint(Error, "KVStore root empty");
        return;
    }
    const auto sRoot = std::string(root);
    auto logLevel = MMKVLogNone;
    MMKV::initializeMMKV(
#if defined(_WIN32)
        string2MMKVPath_t(sRoot)
#else
        sRoot
#endif
        , logLevel);
}

std::weak_ptr<Connection> KVStore::open(std::string_view _id)
{
    if (_id.empty()) [[unlikely]]
    {
        dynxxLogPrint(Error, "KVStore.open _id empty");
        return {};
    }
    const auto cid = genCid(_id);
    return ConnPool<Connection>::open(cid, [cid, _id]() { 
        return std::make_shared<Connection>(cid, _id); 
    });
}

KVStore::~KVStore()
{
    MMKV::onExit();
}

Connection::Connection(CidT cid, std::string_view _id) : 
_cid(cid), kv(MMKV::mmkvWithID(std::string(_id), MMKV_MULTI_PROCESS), KVDeleter())
{
    if (kv == nullptr) [[unlikely]]
    {
        dynxxLogPrint(Error, "MMKV mmkvWithID failed");
    }
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
        [k, kv = this->kv.get()](const auto &x)
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

} // namespace DynXX::Core::Store::KV

#endif
