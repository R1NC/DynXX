#include "JsonDecoder.hxx"

#include <utility>

#include <NGenXXLog.hxx>

std::string NGenXX::Json::dictAnyToJson(const DictAny &dict)
{
    auto cjson = cJSON_CreateObject();
    for (const auto &[k, v] : dict) 
    {
        cJSON *node = nullptr;
        if (std::holds_alternative<int64_t>(v)) 
        {
            node = cJSON_CreateNumber(std::get<int64_t>(v));
        } 
        else if (std::holds_alternative<double>(v)) 
        {
            node = cJSON_CreateNumber(std::get<double>(v));
        } 
        else if (std::holds_alternative<std::string>(v)) 
        {
            node = cJSON_CreateString(std::get<std::string>(v).c_str());
        }
        if (node) [[likely]]
        {
            cJSON_AddItemToObject(cjson, k.c_str(), node);
        }
    }
    std::string json = cJSON_PrintUnformatted(cjson);
    cJSON_Delete(cjson);
    return json;
}

DictAny NGenXX::Json::dictAnyFromJson(const std::string &json)
{
    DictAny dict;
    auto cjson = cJSON_Parse(json.c_str());
    if (!cjson) [[unlikely]]
    {
        return dict;
    }

    if (cJSON_IsObject(cjson)) [[likely]]
    {
        auto node = cjson->child;
        while (node)
        {
            std::string k(node->string);
            if (cJSON_IsNumber(node))
            {
                dict.emplace(k, node->valuedouble);
            }
            else if (cJSON_IsString(node))
            {
                dict.emplace(k, node->valuestring);
            }
            node = node->next;
        }
    }

    cJSON_Delete(cjson);
    return dict;
}

std::string NGenXX::Json::dictToJson(const Dict &dict)
{
    auto cjson = cJSON_CreateObject();
    for (const auto &[k, v] : dict) 
    {
        auto node = cJSON_CreateString(v.c_str());
        cJSON_AddItemToObject(cjson, k.c_str(), node);
    }
    std::string json = cJSON_PrintUnformatted(cjson);
    cJSON_Delete(cjson);
    return json;
}

Dict NGenXX::Json::dictFromJson(const std::string &json)
{
    Dict dict;
    auto cjson = cJSON_Parse(json.c_str());
    if (!cjson) [[unlikely]]
    {
        return dict;
    }

    if (cJSON_IsObject(cjson)) [[likely]]
    {
        auto node = cjson->child;
        while (node)
        {
            std::string k(node->string);
            if (cJSON_IsString(node)) [[likely]]
            {
                dict.emplace(k, node->valuestring);
            }
            node = node->next;
        }
    }

    cJSON_Delete(cjson);
    return dict;
}

void NGenXX::Json::Decoder::moveImp(Decoder&& other) noexcept
{
    this->cjson = std::exchange(other.cjson, nullptr);
}

void NGenXX::Json::Decoder::cleanup() noexcept
{
    if (this->cjson == nullptr) [[unlikely]]
    {
        return;
    }
    cJSON_Delete(this->cjson);
    this->cjson = nullptr;
}

NGenXX::Json::Decoder::Decoder(const std::string &json)
{
    this->cjson = cJSON_Parse(json.c_str());
}

NGenXX::Json::Decoder::~Decoder()
{
    this->cleanup();
}

NGenXX::Json::Decoder::Decoder(NGenXX::Json::Decoder &&other) noexcept
{
    this->moveImp(std::move(other));
}

NGenXX::Json::Decoder &NGenXX::Json::Decoder::operator=(NGenXX::Json::Decoder &&other) noexcept
{
    if (this != &other) [[likely]]
    {
        this->cleanup();
        this->moveImp(std::move(other));
    }
    return *this;
}

const cJSON *NGenXX::Json::Decoder::reinterpretNode(const void *const node) const
{
    if (node == nullptr)
    {
        return this->cjson;
    }
    return static_cast<const cJSON *>(node);
}

bool NGenXX::Json::Decoder::isArray(const void *const node) const
{
    auto cj = this->reinterpretNode(node);
    return cj ? cJSON_IsArray(cj) : false;
}

bool NGenXX::Json::Decoder::isObject(const void *const node) const
{
    auto cj = this->reinterpretNode(node);
    return cj ? cJSON_IsObject(cj) : false;
}

void *NGenXX::Json::Decoder::readNode(const void *const node, const std::string &k) const
{
    auto cj = this->reinterpretNode(node);
    if (cj != nullptr) [[likely]]
    {
        return cJSON_GetObjectItemCaseSensitive(cj, k.c_str());
    }
    return nullptr;
}

std::string NGenXX::Json::Decoder::readString(const void *const node) const
{
    std::string s;
    auto cj = this->reinterpretNode(node);
    if (cj != nullptr && cJSON_IsString(cj) && cj->valuestring) [[likely]]
    {
        return cj->valuestring;
    }
    return s;
}

double NGenXX::Json::Decoder::readNumber(const void *const node) const
{
    auto num = 0.0;
    auto cj = this->reinterpretNode(node);
    if (cj != nullptr) [[likely]]
    {
        if (cJSON_IsNumber(cj)) [[likely]]
        {
            num = cj->valuedouble;
        } 
        else if (cJSON_IsString(cj)) [[likely]]
        {
            try
            {
                num = std::stod(cj->valuestring);
            }
            catch (const std::exception &e)
            {
                ngenxxLogPrintF(NGenXXLogLevelX::Error, "FAILED TO PARSE JSON NUMBER({}): INVALID STRING VALUE", cj->string ?: "");
            }
        }
        else [[unlikely]]
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "FAILED TO PARSE JSON NUMBER({}): INVALID NODE TYPE({})", cj->string ?: "", cj->type);
        }
    }
    return num;
}

void *NGenXX::Json::Decoder::readChild(const void *const node) const
{
    auto cj = this->reinterpretNode(node);
    if (cj != nullptr && (this->isArray(cj) || this->isObject(cj))) [[likely]]
    {
        return cj->child;
    }
    return nullptr;
}

void NGenXX::Json::Decoder::readChildren(const void *const node, std::function<void(size_t idx, const void * child)> &&callback) const
{
    if (!this->isArray(node) && !this->isObject(node)) [[unlikely]]
    {
        return;
    }
    auto childNode = this->readChild(node);
    auto idx = 0;
    while (childNode)
    {
        std::move(callback)(idx++, childNode);
        childNode = this->readNext(childNode);
    }
}

void *NGenXX::Json::Decoder::readNext(const void *const node) const
{
    auto cj = this->reinterpretNode(node);
    return cj ? cj->next : nullptr;
}