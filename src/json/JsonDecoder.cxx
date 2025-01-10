#include "JsonDecoder.hxx"

#include <NGenXXLog.hxx>

#include <stdexcept>

const cJSON *NGenXX::Json::Decoder::parseNode(const void *const node)
{
    if (node == NULL)
    {
        return this->cjson;
    }
    return reinterpret_cast<cJSON *>(const_cast<void *>(node));
}

NGenXX::Json::Decoder::Decoder(const std::string &json)
{
    this->cjson = cJSON_Parse(json.c_str());
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
    auto cj = this->parseNode(node);
    if (cj != NULL && cJSON_IsString(cj))
    {
        return std::string(cj->valuestring ?: "");
    }
    return "";
}

double NGenXX::Json::Decoder::readNumber(const void *const node)
{
    double num = 0;
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
    void *childNode = this->readChild(node);
    int idx = 0;
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

NGenXX::Json::Decoder::~Decoder()
{
    if (this->cjson != NULL)
    {
        cJSON_Delete(this->cjson);
    }
}