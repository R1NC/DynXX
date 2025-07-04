#include "KV.hxx"

#include <NGenXXLog.hxx>

NGenXX::Core::Store::KV::KV(const std::string &root)
{
    decltype(root) sRoot(root);
    auto logLevel = MMKVLogNone;
#if defined(_WIN32)
    MMKV::initializeMMKV(string2MMKVPath_t(sRoot), logLevel);
#else
    MMKV::initializeMMKV(sRoot, logLevel);
#endif
}

std::weak_ptr<NGenXX::Core::Store::KV::Connection> NGenXX::Core::Store::KV::open(const std::string &_id)
{
    auto lock = std::lock_guard(this->mutex);
    if (this->conns.contains(_id))
    {
        return this->conns.at(_id);
    }
    this->conns.emplace(_id, std::make_shared<Connection>(_id));
    return this->conns.at(_id);
}

void NGenXX::Core::Store::KV::close(const std::string &_id)
{
    auto lock = std::lock_guard(this->mutex);
    if (this->conns.contains(_id))
    {
        this->conns.at(_id).reset();
        this->conns.erase(_id);
    }
}

void NGenXX::Core::Store::KV::closeAll()
{
    auto lock = std::lock_guard(this->mutex);
    for (auto &[_, v] : this->conns) 
    {
        v.reset();
    }
    this->conns.clear();
}

NGenXX::Core::Store::KV::~KV()
{
    MMKV::onExit();
}

NGenXX::Core::Store::KV::Connection::Connection(const std::string &_id)
{
    this->kv = MMKV::mmkvWithID(_id, MMKV_MULTI_PROCESS);
    MMKV::setLogLevel(MMKVLogNone);
}

std::optional<std::string> NGenXX::Core::Store::KV::Connection::readString(std::string_view k) const
{
    auto lock = std::shared_lock(this->mutex);
    if (!this->kv->containsKey(k)) [[unlikely]]
    {
        return std::nullopt;
    }
    std::string s;
    this->kv->getString(k, s);
    return std::make_optional(s);
}

std::optional<int64_t> NGenXX::Core::Store::KV::Connection::readInteger(std::string_view k) const
{
    auto lock = std::shared_lock(this->mutex);
    if (!this->kv->containsKey(k)) [[unlikely]]
    {
        return std::nullopt;
    }
    return std::make_optional(this->kv->getInt64(k));
}

std::optional<double> NGenXX::Core::Store::KV::Connection::readFloat(std::string_view k) const
{
    auto lock = std::shared_lock(this->mutex);
    if (!this->kv->containsKey(k)) [[unlikely]]
    {
        return std::nullopt;
    }
    return std::make_optional(this->kv->getDouble(k));
}

bool NGenXX::Core::Store::KV::Connection::write(std::string_view k, const Any &v) const
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

std::vector<std::string> NGenXX::Core::Store::KV::Connection::allKeys() const
{
    auto lock = std::shared_lock(this->mutex);
    return this->kv->allKeys(false);
}

bool NGenXX::Core::Store::KV::Connection::contains(std::string_view k) const
{
    auto lock = std::shared_lock(this->mutex);
    return this->kv->containsKey(k);
}

bool NGenXX::Core::Store::KV::Connection::remove(std::string_view k) const
{
    auto lock = std::unique_lock(this->mutex);
    return this->kv->removeValueForKey(k);
}

void NGenXX::Core::Store::KV::Connection::clear() const
{
    auto lock = std::unique_lock(this->mutex);
    this->kv->clearAll();
    this->kv->clearMemoryCache();
}

NGenXX::Core::Store::KV::Connection::~Connection()
{
    this->kv->close();
    this->kv = nullptr;
}