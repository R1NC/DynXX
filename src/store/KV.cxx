#include "KV.hxx"

#include <NGenXXLog.hxx>

NGenXX::Store::KV::KV(const std::string &root)
{
    decltype(root) sRoot(root);
    auto logLevel = MMKVLogNone;
#if defined(_WIN32)
    MMKV::initializeMMKV(string2MMKVPath_t(sRoot), logLevel);
#else
    MMKV::initializeMMKV(sRoot, logLevel);
#endif
}

std::weak_ptr<NGenXX::Store::KV::Connection> NGenXX::Store::KV::open(const std::string &_id)
{
    auto lock = std::lock_guard(this->mutex);
    if (this->conns.contains(_id))
    {
        return this->conns.at(_id);
    }
    this->conns.emplace(_id, std::make_shared<Connection>(_id));
    return this->conns.at(_id);
}

void NGenXX::Store::KV::close(const std::string &_id)
{
    auto lock = std::lock_guard(this->mutex);
    if (this->conns.contains(_id))
    {
        this->conns.at(_id).reset();
        this->conns.erase(_id);
    }
}

void NGenXX::Store::KV::closeAll()
{
    auto lock = std::lock_guard(this->mutex);
    for (auto &[_, v] : this->conns) 
    {
        v.reset();
    }
    this->conns.clear();
}

NGenXX::Store::KV::~KV()
{
    MMKV::onExit();
}

NGenXX::Store::KV::Connection::Connection(const std::string &_id)
{
    this->kv = MMKV::mmkvWithID(_id, MMKV_MULTI_PROCESS);
    MMKV::setLogLevel(MMKVLogNone);
}

std::string NGenXX::Store::KV::Connection::readString(const std::string_view &k) const
{
    auto lock = std::shared_lock(this->mutex);
    std::string s;
    this->kv->getString(k, s);
    return s;
}

int64_t NGenXX::Store::KV::Connection::readInteger(const std::string_view &k) const
{
    auto lock = std::shared_lock(this->mutex);
    return this->kv->getInt64(k);
}

double NGenXX::Store::KV::Connection::readFloat(const std::string_view &k) const
{
    auto lock = std::shared_lock(this->mutex);
    return this->kv->getDouble(k);
}

bool NGenXX::Store::KV::Connection::write(const std::string_view &k, const Any &v) const
{
    auto lock = std::unique_lock(this->mutex);
    return std::visit(
        [k, &kv = this->kv](auto &x)
        { 
            return kv->set(x, k); 
        },
        v
    );
}

std::vector<std::string> NGenXX::Store::KV::Connection::allKeys() const
{
    auto lock = std::shared_lock(this->mutex);
    return this->kv->allKeys(false);
}

bool NGenXX::Store::KV::Connection::contains(const std::string_view &k) const
{
    auto lock = std::shared_lock(this->mutex);
    return this->kv->containsKey(k);
}

bool NGenXX::Store::KV::Connection::remove(const std::string_view &k) const
{
    auto lock = std::unique_lock(this->mutex);
    return this->kv->removeValueForKey(k);
}

void NGenXX::Store::KV::Connection::clear() const
{
    auto lock = std::unique_lock(this->mutex);
    this->kv->clearAll();
    this->kv->clearMemoryCache();
}

NGenXX::Store::KV::Connection::~Connection()
{
    this->kv->close();
}