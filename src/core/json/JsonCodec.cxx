#include "JsonCodec.hxx"

#include <utility>

#include <NGenXXLog.hxx>

NGenXXJsonNodeTypeX NGenXX::Core::Json::cJSONReadType(void *const cjson)
{
    if (cjson == nullptr) [[unlikely]]
    {
        return NGenXXJsonNodeTypeX::Unknown;
    }
    switch (static_cast<cJSON *>(cjson)->type)
    {
        case cJSON_Number:
            return NGenXXJsonNodeTypeX::Number;
        case cJSON_String:
            return NGenXXJsonNodeTypeX::String;
        case cJSON_True:
        case cJSON_False:
            return NGenXXJsonNodeTypeX::Boolean;
        case cJSON_Array:
            return NGenXXJsonNodeTypeX::Array;
        case cJSON_Object:
            return NGenXXJsonNodeTypeX::Object;
        case cJSON_NULL:
            return NGenXXJsonNodeTypeX::Null;
        default:
            return NGenXXJsonNodeTypeX::Unknown;
    }
}

std::optional<std::string> NGenXX::Core::Json::cJSONToStr(void *const cjson)
{
    if (!cjson) [[unlikely]]
    {
        return std::nullopt;
    }
    const auto jsonChars = cJSON_PrintUnformatted(static_cast<cJSON *>(cjson));
    if (!jsonChars) [[unlikely]]
    {
        return std::nullopt;
    }
    std::string json(jsonChars);
    //std::free(jsonChars);
    return std::make_optional(json);
}

std::optional<std::string> NGenXX::Core::Json::jsonFromDictAny(const DictAny &dict)
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
            if (!cJSON_AddItemToObject(cjson, k.c_str(), node)) [[unlikely]]
            {
                ngenxxLogPrintF(NGenXXLogLevelX::Error, "FAILED TO ADD JSON VALUE: {}", k);
                cJSON_Delete(node);
            }
        }
    }
    auto json = cJSONToStr(cjson);
    cJSON_Delete(cjson);
    return json;
}

std::optional<DictAny> NGenXX::Core::Json::jsonToDictAny(const std::string &json)
{
    const auto cjson = cJSON_Parse(json.c_str());
    if (!cjson) [[unlikely]]
    {
        return std::nullopt;
    }

    if (cJSONReadType(cjson) != NGenXXJsonNodeTypeX::Object) [[unlikely]]
    {
        cJSON_Delete(cjson);
        return std::nullopt;
    }

    DictAny dict;
    for (auto node = cjson->child; node != nullptr; node = node->next)
    {
        std::string k(node->string);
        if (const auto type = cJSONReadType(node); type == NGenXXJsonNodeTypeX::Number)
        {
            dict.emplace(k, node->valuedouble);
        }
        else if (type == NGenXXJsonNodeTypeX::String)
        {
            dict.emplace(k, node->valuestring);
        }
        else
        {
            dict.emplace(k, cJSONToStr(node).value_or(""));
        }
    }

    cJSON_Delete(cjson);
    return std::make_optional(dict);
}

#pragma mark Decoder

void NGenXX::Core::Json::Decoder::moveImp(Decoder&& other) noexcept
{
    this->cjson = std::exchange(other.cjson, nullptr);
}

void NGenXX::Core::Json::Decoder::cleanup() noexcept
{
    if (this->cjson == nullptr) [[unlikely]]
    {
        return;
    }
    cJSON_Delete(this->cjson);
    this->cjson = nullptr;
}

NGenXX::Core::Json::Decoder::Decoder(const std::string &json)
{
    this->cjson = cJSON_Parse(json.c_str());
}

NGenXX::Core::Json::Decoder::~Decoder()
{
    this->cleanup();
}

NGenXX::Core::Json::Decoder::Decoder(Decoder &&other) noexcept
{
    this->moveImp(std::move(other));
}

NGenXX::Core::Json::Decoder &NGenXX::Core::Json::Decoder::operator=(Decoder &&other) noexcept
{
    if (this != &other) [[likely]]
    {
        this->cleanup();
        this->moveImp(std::move(other));
    }
    return *this;
}

const cJSON *NGenXX::Core::Json::Decoder::reinterpretNode(void *const node) const
{
    if (node == nullptr)
    {
        return this->cjson;
    }
    return static_cast<const cJSON *>(node);
}

void *NGenXX::Core::Json::Decoder::readNode(void *const node, const std::string &k) const
{
    if (const auto cj = this->reinterpretNode(node); cj != nullptr) [[likely]]
    {
        return cJSON_GetObjectItemCaseSensitive(cj, k.c_str());
    }
    return nullptr;
}

void *NGenXX::Core::Json::Decoder::operator[](const std::string &k) const
{
    return this->readNode(nullptr, k);
}

std::optional<std::string> NGenXX::Core::Json::Decoder::readString(void *const node) const
{
    const auto cj = this->reinterpretNode(node);
    switch(cJSONReadType(node))
    {
        case NGenXXJsonNodeTypeX::Object:
        case NGenXXJsonNodeTypeX::Array:
        {
            return cJSONToStr(node);
        }
        case NGenXXJsonNodeTypeX::String:
        {
            return std::make_optional(cj->valuestring);
        }
        case NGenXXJsonNodeTypeX::Number:
        {
            return std::make_optional(std::to_string(cj->valuedouble));
        }
        case NGenXXJsonNodeTypeX::Boolean:
        {
            return std::make_optional(cj->valueint? "true" : "false");
        }
        default:
        {
            return std::nullopt;
        }
    }
}

std::optional<double> NGenXX::Core::Json::Decoder::readNumber(void *const node) const
{
    const auto type = cJSONReadType(node);
    const auto cj = this->reinterpretNode(node);
    if (type == NGenXXJsonNodeTypeX::Number) [[likely]]
    {
        return std::make_optional(cj->valuedouble);
    } 
    if (type == NGenXXJsonNodeTypeX::String) [[likely]]
    {
        return std::make_optional(str2float64(cj->valuestring));
    }
    ngenxxLogPrintF(NGenXXLogLevelX::Error, "FAILED TO PARSE JSON NUMBER({}): INVALID NODE TYPE({})", 
                cj->string != nullptr ? cj->string: "", cj->type);
    return std::nullopt;
}

void *NGenXX::Core::Json::Decoder::readChild(void *const node) const
{
    if (const auto type = cJSONReadType(node); type == NGenXXJsonNodeTypeX::Object || type == NGenXXJsonNodeTypeX::Array) [[likely]]
    {
        return this->reinterpretNode(node)->child;
    }
    return nullptr;
}

void NGenXX::Core::Json::Decoder::readChildren(void *const node, std::function<void(size_t idx, void *const child)> &&callback) const
{
    if (const auto type = cJSONReadType(node); type != NGenXXJsonNodeTypeX::Object && type != NGenXXJsonNodeTypeX::Array) [[unlikely]]
    {
        return;
    }
    auto idx = 0;
    for (auto childNode = this->readChild(node); childNode != nullptr; childNode = this->readNext(childNode))
    {
        std::move(callback)(idx++, childNode);
    }
}

void *NGenXX::Core::Json::Decoder::readNext(void *const node) const
{
    const auto cj = this->reinterpretNode(node);
    return cj ? cj->next : nullptr;
}