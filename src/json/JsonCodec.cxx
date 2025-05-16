#include "JsonCodec.hxx"

#include <utility>

#include <NGenXXLog.hxx>

std::string NGenXX::Json::cJSON2Str(void *const cjson)
{
    if (!cjson) [[unlikely]]
    {
        return {};
    }
    const auto jsonChars = cJSON_PrintUnformatted(static_cast<cJSON *>(cjson));
    if (!jsonChars) [[unlikely]]
    {
        return {};
    }
    std::string json(jsonChars);
    std::free(jsonChars);
    return json;
}

std::string NGenXX::Json::jsonFromDictAny(const DictAny &dict)
{
    const auto cjson = cJSON_CreateObject();
    for (const auto &[k, v] : dict) 
    {
        cJSON *node = nullptr;
        if (std::holds_alternative<int64_t>(v)) 
        {
            node = cJSON_CreateNumber(static_cast<double>(std::get<int64_t>(v)));
        } 
        else if (std::holds_alternative<double>(v)) 
        {
            node = cJSON_CreateNumber(std::get<double>(v));
        } 
        else if (std::holds_alternative<std::string>(v)) 
        {
            auto s = std::get<std::string>(v);
            if ((s.starts_with("[") && s.ends_with("]"))
                || (s.starts_with("{") && s.ends_with("}")))
            {
                if (const auto cjsonNodes = cJSON_Parse(s.c_str()))
                {
                    node = cjsonNodes;
                }
                else [[unlikely]]
                {
                    ngenxxLogPrintF(NGenXXLogLevelX::Error, "FAILED TO PARSE JSON VALUE: {}", s);
                }
            }
            else
            {
                node = cJSON_CreateString(s.c_str());
            }
        }
        if (node) [[likely]]
        {
            cJSON_AddItemToObject(cjson, k.c_str(), node);
        }
    }
    auto json = cJSON2Str(cjson);
    cJSON_Delete(cjson);
    return json;
}

DictAny NGenXX::Json::jsonToDictAny(const std::string &json)
{
    DictAny dict;
    const auto cjson = cJSON_Parse(json.c_str());
    if (!cjson) [[unlikely]]
    {
        return dict;
    }

    if (cJSON_IsObject(cjson)) [[likely]]
    {
        for (auto node = cjson->child; node != nullptr; node = node->next)
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
            else
            {
                dict.emplace(k, cJSON2Str(node));
            }
        }
    }

    cJSON_Delete(cjson);
    return dict;
}

#pragma mark Decoder

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

NGenXX::Json::Decoder::Decoder(Decoder &&other) noexcept
{
    this->moveImp(std::move(other));
}

NGenXX::Json::Decoder &NGenXX::Json::Decoder::operator=(Decoder &&other) noexcept
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
    const auto cj = this->reinterpretNode(node);
    return cj ? cJSON_IsArray(cj) : false;
}

bool NGenXX::Json::Decoder::isObject(const void *const node) const
{
    const auto cj = this->reinterpretNode(node);
    return cj ? cJSON_IsObject(cj) : false;
}

void *NGenXX::Json::Decoder::readNode(const void *const node, const std::string &k) const
{
    if (const auto cj = this->reinterpretNode(node); cj != nullptr) [[likely]]
    {
        return cJSON_GetObjectItemCaseSensitive(cj, k.c_str());
    }
    return nullptr;
}

void *NGenXX::Json::Decoder::operator[](const std::string &k) const
{
    return this->readNode(nullptr, k);
}

std::string NGenXX::Json::Decoder::readString(const void *const node) const
{
    const auto cj = this->reinterpretNode(node);
    if (cj == nullptr) [[unlikely]]
    {
        return {};
    }
    if (cJSON_IsBool(cj))
    {
        return cj->valueint ? "true" : "false";
    }
    if (cJSON_IsNumber(cj))
    {
        return std::to_string(cj->valuedouble);
    }
    if (cJSON_IsString(cj) && cj->valuestring)
    {
        return cj->valuestring;
    }
    if (!cJSON_IsNull(cj))
    {
        return cJSON_PrintUnformatted(cj);
    }
    return {};
}

double NGenXX::Json::Decoder::readNumber(const void *const node) const
{
    auto num = 0.0;
    if (const auto cj = this->reinterpretNode(node); cj != nullptr) [[likely]]
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
            catch (const std::exception &_)
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
    if (const auto cj = this->reinterpretNode(node); cj != nullptr && (this->isArray(cj) || this->isObject(cj))) [[likely]]
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
    auto idx = 0;
    for (auto childNode = this->readChild(node); childNode != nullptr; childNode = this->readNext(childNode))
    {
        std::move(callback)(idx++, childNode);
    }
}

void *NGenXX::Json::Decoder::readNext(const void *const node) const
{
    const auto cj = this->reinterpretNode(node);
    return cj ? cj->next : nullptr;
}