#include "JsonCodec.hxx"

#include <utility>
#include <climits>

#include <NGenXXLog.hxx>
#include <NGenXXTypes.hxx>
#include <NGenXXCoding.hxx>

namespace
{
    bool isNum(const cJSON *const cj)
    {
        return cj != nullptr && cj->type == cJSON_Number;
    }

    bool isBigInt(const cJSON *const cj)
    {
        return isNum(cj) && (cj->valueint == INT_MAX && cj->valuedouble > INT_MAX) || 
            (cj->valueint == INT_MIN && cj->valuedouble < INT_MIN);
    }

    bool isFloat(const cJSON *const cj)
    {
        return isNum(cj) && !isBigInt(cj) && cj->valuedouble != static_cast<double>(cj->valueint);
    }

    int32_t readInt32(const cJSON *const cj)
    {
        return cj->valueint;
    }

    int64_t readInt64(const cJSON *const cj)
    {
        return static_cast<int64_t>(cj->valuedouble);
    }

    double readFloat(const cJSON *const cj)
    {
        return cj->valuedouble;
    }
}

NGenXXJsonNodeTypeX NGenXX::Core::Json::cJSONReadType(void *const cjson)
{
    const auto cj = static_cast<cJSON *>(cjson);
    if (cj == nullptr) [[unlikely]]
    {
        return NGenXXJsonNodeTypeX::Unknown;
    }
    switch (cj->type)
    {
        case cJSON_Number:
            if (isBigInt(cj))
            {
                return NGenXXJsonNodeTypeX::Int64;
            }
            return isFloat(cj) ? NGenXXJsonNodeTypeX::Float : NGenXXJsonNodeTypeX::Int32;
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
    std::free(jsonChars);
    json = ngenxxCodingStrTrim(json);
    return std::make_optional(json);
}

std::optional<std::string> NGenXX::Core::Json::jsonFromDictAny(const DictAny &dict)
{
    if (dict.empty()) [[unlikely]]
    {
        return std::nullopt;
    }
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
    if (json.empty()) [[unlikely]]
    {
        return std::nullopt;
    }
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
        if (node->string == nullptr)
        {
            continue;
        }
        std::string k(node->string);
        if (const auto type = cJSONReadType(node); type == NGenXXJsonNodeTypeX::Float)
        {
            dict.emplace(k, readFloat(node));
        }
        else if (type == NGenXXJsonNodeTypeX::Int32)
        {
            dict.emplace(k, readInt32(node));
        }
        else if (type == NGenXXJsonNodeTypeX::Int64)
        {
            dict.emplace(k, readInt64(node));
        }
        else if (type == NGenXXJsonNodeTypeX::String)
        {
            dict.emplace(k, wrapStr(node->valuestring));
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

void *NGenXX::Core::Json::Decoder::readNode(void *const node, const std::string_view &k) const
{
    if (const auto cj = this->reinterpretNode(node); cj != nullptr) [[likely]]
    {
        return cJSON_GetObjectItemCaseSensitive(cj, k.data());
    }
    return nullptr;
}

void *NGenXX::Core::Json::Decoder::operator[](const std::string_view &k) const
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
            return std::make_optional(wrapStr(cj->valuestring));
        }
        case NGenXXJsonNodeTypeX::Int32:
        {
            return std::make_optional(std::to_string(readInt32(cj)));
        }
        case NGenXXJsonNodeTypeX::Int64:
        {
            return std::make_optional(std::to_string(readInt64(cj)));
        }
        case NGenXXJsonNodeTypeX::Float:
        {
            return std::make_optional(std::to_string(readFloat(cj)));
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
    if (type == NGenXXJsonNodeTypeX::Int32)
    {
        return std::make_optional(readInt32(cj));
    } 
    if (type == NGenXXJsonNodeTypeX::Int64)
    {
        return std::make_optional(readInt64(cj));
    } 
    if (type == NGenXXJsonNodeTypeX::Float)
    {
        return std::make_optional(readFloat(cj));
    } 
    if (type == NGenXXJsonNodeTypeX::String)
    {
        return std::make_optional(str2float64(wrapStr(cj->valuestring)));
    }
    ngenxxLogPrintF(NGenXXLogLevelX::Error, "FAILED TO PARSE JSON NUMBER({}): INVALID NODE TYPE({})", 
                cj->string, cj->type);
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
    const auto& cbk = std::move(callback);
    for (auto childNode = this->readChild(node); childNode != nullptr; childNode = this->readNext(childNode))
    {
        cbk(idx++, childNode);
    }
}

void *NGenXX::Core::Json::Decoder::readNext(void *const node) const
{
    const auto cj = this->reinterpretNode(node);
    return cj ? cj->next : nullptr;
}