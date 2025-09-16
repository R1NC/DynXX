#include "JsonCodec.hxx"

#include <utility>
#include <climits>

#include <DynXX/CXX/Log.hxx>
#include <DynXX/CXX/Coding.hxx>
#include <DynXX/CXX/Memory.hxx>

#include "../util/MemUtil.hxx"

namespace
{
    using enum DynXXJsonNodeTypeX;
    using enum DynXXLogLevelX;
    using namespace DynXX::Core::Util::Mem;

    bool isNum(const cJSON *const cj)
    {
        return cj != nullptr && cj->type == cJSON_Number;
    }

    bool isBigInt(const cJSON *const cj)
    {
        return isNum(cj) && (
            (cj->valueint == INT_MAX && cj->valuedouble > INT_MAX) 
            ||
            (cj->valueint == INT_MIN && cj->valuedouble < INT_MIN)
        );
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

DynXXJsonNodeTypeX DynXX::Core::Json::nodeReadType(const DynXXJsonNodeHandle node)
{
    const auto cj = addr2ptr<cJSON>(node);
    if (cj == nullptr) [[unlikely]]
    {
        return Unknown;
    }
    switch (cj->type)
    {
        case cJSON_Number:
            if (isBigInt(cj))
            {
                return Int64;
            }
            return isFloat(cj) ? Float : Int32;
        case cJSON_String:
            return String;
        case cJSON_True:
        case cJSON_False:
            return Boolean;
        case cJSON_Array:
            return Array;
        case cJSON_Object:
            return Object;
        case cJSON_NULL:
            return Null;
        default:
            return Unknown;
    }
}

std::optional<std::string> DynXX::Core::Json::nodeReadName(const DynXXJsonNodeHandle node)
{
    const auto cj = addr2ptr<cJSON>(node);
    if (cj == nullptr || cj->string == nullptr) [[unlikely]]
    {
        return std::nullopt;
    }
    return {cj->string};
}

std::optional<std::string> DynXX::Core::Json::nodeToStr(const DynXXJsonNodeHandle node)
{
    if (!node) [[unlikely]]
    {
        return std::nullopt;
    }
    auto jsonChars = cJSON_PrintUnformatted(addr2ptr<cJSON>(node));
    if (!jsonChars) [[unlikely]]
    {
        return std::nullopt;
    }
    std::string json(jsonChars);
    freeX(jsonChars);
    json = dynxxCodingStrTrim(json);
    return {json};
}

std::optional<std::string> DynXX::Core::Json::jsonFromDictAny(const DictAny &dict)
{
    if (dict.empty()) [[unlikely]]
    {
        return std::nullopt;
    }
    const auto cjson = cJSON_CreateObject();

    const auto parseStringF = [](const std::string &s) -> cJSON* {
        if ((s.starts_with("[") && s.ends_with("]"))
            || (s.starts_with("{") && s.ends_with("}")))
        {
            if (const auto cjsonNodes = cJSON_Parse(s.c_str()))
            {
                return cjsonNodes;
            }
            else [[unlikely]]
            {
                dynxxLogPrintF(Error, "FAILED TO PARSE JSON VALUE: {}", s);
                return nullptr;
            }
        }
        else
        {
            return cJSON_CreateString(s.c_str());
        }
    };

    for (const auto &[k, v] : dict) 
    {
        if (const auto node = std::visit([&](const auto &x) {
            if constexpr (std::is_same_v<std::decay_t<decltype(x)>, std::string>) {
                return parseStringF(x);
            } else {
                return cJSON_CreateNumber(static_cast<double>(x));
            }
        }, v)) [[likely]]
        {
            if (!cJSON_AddItemToObject(cjson, k.c_str(), node)) [[unlikely]]
            {
                dynxxLogPrintF(Error, "FAILED TO ADD JSON VALUE: {}", k);
                cJSON_Delete(node);
            }
        }
    }
    auto json = nodeToStr(ptr2addr(cjson));
    cJSON_Delete(cjson);
    return json;
}

std::optional<DictAny> DynXX::Core::Json::jsonToDictAny(const std::string &json)
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

    if (nodeReadType(ptr2addr(cjson)) != Object) [[unlikely]]
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
        const auto type = nodeReadType(ptr2addr(node));
        switch(type) {
            case Float:
            {
                dict.emplace(k, readFloat(node));
                break;
            }
            case Int32:
            {
                dict.emplace(k, readInt32(node));
                break;
            }
            case Int64:
            {
                dict.emplace(k, readInt64(node));
                break;
            }
            case String:
            {
                dict.emplace(k, makeStr(node->valuestring));
                break;
            }
            default:
            {
                dict.emplace(k, nodeToStr(ptr2addr(node)).value_or(""));
                break;
            }
        }
    }

    cJSON_Delete(cjson);
    return {dict};
}

// Decoder

void DynXX::Core::Json::Decoder::moveImp(Decoder&& other) noexcept
{
    this->cjson = std::exchange(other.cjson, nullptr);
}

void DynXX::Core::Json::Decoder::cleanup() noexcept
{
    if (this->cjson == nullptr) [[unlikely]]
    {
        return;
    }
    cJSON_Delete(this->cjson);
    this->cjson = nullptr;
}

DynXX::Core::Json::Decoder::Decoder(std::string_view json)
{
    this->cjson = cJSON_Parse(json.data());
    if (this->cjson == nullptr) [[unlikely]]
    {
        dynxxLogPrintF(Error, "FAILED TO PARSE JSON: {}", json);
    }
}

DynXX::Core::Json::Decoder::~Decoder()
{
    this->cleanup();
}

DynXX::Core::Json::Decoder::Decoder(Decoder &&other) noexcept
{
    this->moveImp(std::move(other));
}

DynXX::Core::Json::Decoder &DynXX::Core::Json::Decoder::operator=(Decoder &&other) noexcept
{
    if (this != &other) [[likely]]
    {
        this->cleanup();
        this->moveImp(std::move(other));
    }
    return *this;
}

bool DynXX::Core::Json::Decoder::valid() const
{
    return this->cjson != nullptr;
}

const cJSON *DynXX::Core::Json::Decoder::reinterpretNode(const DynXXJsonNodeHandle node) const
{
    if (!this->valid()) [[unlikely]]
    {
        return nullptr;
    }
    if (node == 0) [[likely]]
    {
        return this->cjson;
    }
    return addr2ptr<cJSON>(node);
}

DynXXJsonNodeHandle DynXX::Core::Json::Decoder::readNode(const DynXXJsonNodeHandle node, std::string_view k) const
{
    if (!this->valid() || k.empty()) [[unlikely]]
    {
        return 0;
    }
    const auto cj = this->reinterpretNode(node); 
    return ptr2addr(cJSON_GetObjectItemCaseSensitive(cj, k.data()));
}

DynXXJsonNodeHandle DynXX::Core::Json::Decoder::operator[](std::string_view k) const
{
    return this->readNode(0, k);
}

std::optional<std::string> DynXX::Core::Json::Decoder::readString(const DynXXJsonNodeHandle node) const
{
    
    if (!this->valid()) [[unlikely]]
    {
        return std::nullopt;
    }
    const auto cj = this->reinterpretNode(node);
    switch(nodeReadType(node))
    {
        case Object:
        case Array:
        {
            return nodeToStr(node);
        }
        case String:
        {
            return {makeStr(cj->valuestring)};
        }
        case Int32:
        {
            return {std::to_string(readInt32(cj))};
        }
        case Int64:
        {
            return {std::to_string(readInt64(cj))};
        }
        case Float:
        {
            return {std::to_string(readFloat(cj))};
        }
        case Boolean:
        {
            return {cj->valueint? "true" : "false"};
        }
        default:
        {
            dynxxLogPrintF(Error, "FAILED TO PARSE JSON STRING({}): INVALID NODE TYPE({})", 
                cj->string, cj->type);
            return std::nullopt;
        }
    }
}

std::optional<Num> DynXX::Core::Json::Decoder::readNumber(const DynXXJsonNodeHandle node) const
{
    if (!this->valid()) [[unlikely]]
    {
        return std::nullopt;
    }
    const auto cj = this->reinterpretNode(node);
    switch(nodeReadType(node)) {
        case Int32:
        {
            return {readInt32(cj)};
        }
        case Int64:
        {
            return {readInt64(cj)};
        }
        case Float:
        {
            return {readFloat(cj)};
        }
        case String:
        {
            const auto str = makeStr(cj->valuestring);
            if (str.find('.') != std::string::npos) {
                return {str2float64(str)};
            }
            return {str2int64(str)};
        }
        default:
        {
            dynxxLogPrintF(Error, "FAILED TO PARSE JSON NUMBER({}): INVALID NODE TYPE({})", 
                cj->string, cj->type);
            return std::nullopt;
        }
    }
}

DynXXJsonNodeHandle DynXX::Core::Json::Decoder::readChild(const DynXXJsonNodeHandle node) const
{
    if (!this->valid()) [[unlikely]]
    {
        return 0;
    }
    const auto cj = this->reinterpretNode(node);
    if (const auto type = nodeReadType(node); type == Object || type == Array) [[likely]]
    {   
        return ptr2addr(cj->child);
    }
    return 0;
}

size_t DynXX::Core::Json::Decoder::readChildrenCount(const DynXXJsonNodeHandle node) const
{
    if (!this->valid()) [[unlikely]]
    {
        return 0;
    }
    const auto cj = this->reinterpretNode(node);
    const auto type = nodeReadType(node);
    if (type == Array) [[likely]]
    {
        return cJSON_GetArraySize(cj);
    }
    if (type != Object) [[unlikely]]
    {
        return 0;
    }
    size_t count = 0;
    this->readChildren(node, [&count](size_t, const DynXXJsonNodeHandle, const DynXXJsonNodeTypeX, std::string_view) {
        count++;
    });
    return count;
}

void DynXX::Core::Json::Decoder::readChildren(const DynXXJsonNodeHandle node, std::function<void(size_t idx, const DynXXJsonNodeHandle childNode, const DynXXJsonNodeTypeX childType, std::string_view childName)> &&callback) const
{
    if (!this->valid()) [[unlikely]]
    {
        return;
    }
    if (const auto type = nodeReadType(node); type != Object && type != Array) [[unlikely]]
    {
        return;
    }
    auto idx = 0;
    const auto& cbk = std::move(callback);
    for (auto childNode = this->readChild(node); childNode != 0; childNode = this->readNext(childNode), idx++)
    {
        const auto type = nodeReadType(childNode);
        const auto name = nodeReadName(childNode).value_or("");
        cbk(idx, childNode, type, name);
    }
}

DynXXJsonNodeHandle DynXX::Core::Json::Decoder::readNext(const DynXXJsonNodeHandle node) const
{
    if (!this->valid()) [[unlikely]]
    {
        return 0;
    }
    const auto cj = this->reinterpretNode(node);
    return ptr2addr(cj->next);
}
