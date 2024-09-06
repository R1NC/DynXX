#include "KV.hxx"

using namespace mmkv;

NGenXX::Store::KV::KV(const std::string &uid)
{
    this->kv = MMKV::mmkvWithID(uid, MMKV_MULTI_PROCESS);
}

const std::string NGenXX::Store::KV::readString(const std::string &k)
{
    std::string v;
    this->kv->getString(k.c_str(), v);
    return v;
}

bool NGenXX::Store::KV::writeString(const std::string &k, const std::string &v)
{
    return this->kv->set(v, k);
}

long long NGenXX::Store::KV::readInteger(const std::string &k)
{
    return this->kv->getInt32(k.c_str());
}

bool NGenXX::Store::KV::writeInteger(const std::string &k, long long v)
{
    return this->kv->set(v, k);
}

double NGenXX::Store::KV::readFloat(const std::string &k)
{
    return this->kv->getDouble(k.c_str());
}

bool NGenXX::Store::KV::writeFloat(const std::string &k, double v)
{
    return this->kv->set(v, k);
}

bool NGenXX::Store::KV::contains(const std::string &k)
{
    return this->kv->containsKey(k.c_str());
}

void NGenXX::Store::KV::clear()
{
    this->kv->clearAll();
    this->kv->clearMemoryCache();
}

NGenXX::Store::KV::~KV()
{
    this->kv->close();
}