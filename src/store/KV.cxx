#include "KV.hxx"
#include "../../include/NGenXXLog.hxx"

NGenXX::Store::KV::KV(const std::string &root)
{
    std::string sRoot(root);
    MMKVLogLevel logLevel = MMKVLogNone;
#ifdef _WIN32
    MMKV::initializeMMKV(string2MMKVPath_t(sRoot), logLevel);
#else
    MMKV::initializeMMKV(sRoot, logLevel);
#endif
}

NGenXX::Store::KV::Connection *NGenXX::Store::KV::open(const std::string &_id)
{
    const std::lock_guard<std::mutex> lock(this->mutex);
    auto conn = this->conns[_id];
    if (conn == nullptr)
    {
        conn = new NGenXX::Store::KV::Connection(_id);
        this->conns[_id] = conn;
    }
    return conn;
}

void NGenXX::Store::KV::closeAll()
{
    const std::lock_guard<std::mutex> lock(this->mutex);
    for (auto &it : this->conns)
    {
        delete it.second;
        it.second = nullptr;
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
    this->kv->setLogLevel(MMKVLogNone);
}

const std::string NGenXX::Store::KV::Connection::readString(const std::string &k)
{
    std::shared_lock lock(this->mutex);
    std::string s;
    this->kv->getString(k, s, "");
    return s;
}

const int64_t NGenXX::Store::KV::Connection::readInteger(const std::string &k)
{
    std::shared_lock lock(this->mutex);
    return this->kv->getInt64(k);
}

const double NGenXX::Store::KV::Connection::readFloat(const std::string &k)
{
    std::shared_lock lock(this->mutex);
    return this->kv->getDouble(k);
}

bool NGenXX::Store::KV::Connection::write(const std::string &k, const Any &v)
{
    std::unique_lock lock(this->mutex);
    return std::visit(
        [k, &kv = this->kv](auto &x)
        { 
            return kv->set(x, k); 
        },
        v
    );
}

const std::vector<std::string> NGenXX::Store::KV::Connection::allKeys()
{
    std::shared_lock lock(this->mutex);
    return this->kv->allKeys(false);
}

bool NGenXX::Store::KV::Connection::contains(const std::string &k)
{
    std::shared_lock lock(this->mutex);
    return this->kv->containsKey(k);
}

bool NGenXX::Store::KV::Connection::remove(const std::string &k)
{
    std::unique_lock lock(this->mutex);
    return this->kv->removeValueForKey(k);
}

void NGenXX::Store::KV::Connection::clear()
{
    std::unique_lock lock(this->mutex);
    this->kv->clearAll();
    this->kv->clearMemoryCache();
}

NGenXX::Store::KV::Connection::~Connection()
{
    std::unique_lock lock(this->mutex);
    this->kv->close();
}