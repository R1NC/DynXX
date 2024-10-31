#include "KV.hxx"
#include "../../include/NGenXXLog.hxx"

NGenXX::Store::KV::KV(const std::string &root)
{
    std::string sRoot(root);
#ifdef _WIN32
    MMKV::initializeMMKV(string2MMKVPath_t(sRoot));
#else
    MMKV::initializeMMKV(sRoot);
#endif
}

NGenXX::Store::KV::Connection *NGenXX::Store::KV::open(const std::string &_id)
{
    return new NGenXX::Store::KV::Connection(_id);
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
    std::string s;
    this->kv->getString(k.c_str(), s, "");
    return s;
}

const int64_t NGenXX::Store::KV::Connection::readInteger(const std::string &k)
{
    return this->kv->getInt64(k.c_str());
}

const double NGenXX::Store::KV::Connection::readFloat(const std::string &k)
{
    return this->kv->getDouble(k.c_str());
}

bool NGenXX::Store::KV::Connection::write(const std::string &k, const Any &v)
{
    return std::visit([&](auto &x) {
        return this->kv->set(x, k);
    }, v);
}

const std::vector<std::string> NGenXX::Store::KV::Connection::allKeys()
{
    return this->kv->allKeys(false);
}

bool NGenXX::Store::KV::Connection::contains(const std::string &k)
{
    return this->kv->containsKey(k.c_str());
}

void NGenXX::Store::KV::Connection::remove(const std::string &k)
{
    return this->kv->removeValueForKey(k.c_str());
}

void NGenXX::Store::KV::Connection::clear()
{
    this->kv->clearAll();
    this->kv->clearMemoryCache();
}

NGenXX::Store::KV::Connection::~Connection()
{
    this->kv->close();
}