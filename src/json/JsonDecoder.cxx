#include "JsonDecoder.hxx"

#include <stdexcept>
#include <type_traits>

#include <NGenXXLog.hxx>

void NGenXX::Json::Decoder::moveImp(Decoder&& other) noexcept
{
    this->cjson = other.cjson;
    other.cjson = NULL;
}

void NGenXX::Json::Decoder::cleanup()
{
    if (this->cjson != NULL)
    {
        cJSON_Delete(this->cjson);
        this->cjson = NULL;
    }
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
    if (this != &other)
    {
        this->cleanup();
        this->moveImp(std::move(other));
    }
    return *this;
}

const cJSON *NGenXX::Json::Decoder::parseNode(const void *const node)
{
    if (node == NULL)
    {
        return this->cjson;
    }
    return reinterpret_cast<cJSON *>(std::decay_t<void *>(node));
}

bool NGenXX::Json::Decoder::isArray(const void *const node)
{
    auto cj = this->parseNode(node);
    return cj ? cJSON_IsArray(cj) : false;
}

bool NGenXX::Json::Decoder::isObject(const void *const node)
{
    auto cj = this->parseNode(node);
    return cj ? cJSON_IsObject(cj) : false;
}

void *NGenXX::Json::Decoder::readNode(const void *const node, const std::string &k)
{
    auto cj = this->parseNode(node);
    if (cj != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(cj, k.c_str());
    }
    return NULL;
}

const std::string NGenXX::Json::Decoder::readString(const void *const node)
{
    std::string s;
    auto cj = this->parseNode(node);
    if (cj != NULL && cJSON_IsString(cj) && cj->valuestring)
    {
        return cj->valuestring;
    }
    return s;
}

double NGenXX::Json::Decoder::readNumber(const void *const node)
{
    auto num = 0.0;
    auto cj = this->parseNode(node);
    if (cj != NULL)
    {
        if (cJSON_IsNumber(cj))
        {
            num = cj->valuedouble;
        }
        if (cJSON_IsString(cj))
        {
            try
            {
                num = std::stod(cj->valuestring);
            }
            catch (const std::exception &e)
            {
                ngenxxLogPrint(NGenXXLogLevelX::Error, "FAILED TO PARSE NUMBER FROM JSON");
            }
        }
    }
    return num;
}

void *NGenXX::Json::Decoder::readChild(const void *const node)
{
    auto cj = this->parseNode(node);
    if (cj != NULL && (this->isArray(cj) || this->isObject(cj)))
    {
        return cj->child;
    }
    return NULL;
}

void NGenXX::Json::Decoder::readChildren(const void *const node, const std::function<void(const size_t idx, const void *const child)> &callback)
{
    if (!this->isArray(node) && !this->isObject(node))
    {
        return;
    }
    auto childNode = this->readChild(node);
    auto idx = 0;
    while (childNode)
    {
        if (callback)
        {
            callback(idx++, childNode);
        }
        childNode = this->readNext(childNode);
    }
}

void *NGenXX::Json::Decoder::readNext(const void *const node)
{
    auto cj = this->parseNode(node);
    return cj ? cj->next : NULL;
}