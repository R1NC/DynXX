#include "JsonDecoder.hxx"

cJSON *NGenXX::Json::Decoder::parseNode(void *node)
{
    if (node == NULL)
        return this->cjson;
    return (cJSON *)node;
}

NGenXX::Json::Decoder::Decoder(const std::string &json)
{
    this->cjson = cJSON_Parse(json.c_str());
}

bool NGenXX::Json::Decoder::isArray(void *node)
{
    cJSON *cj = this->parseNode(node);
    return cj ? cJSON_IsArray(cj) : false;
}

bool NGenXX::Json::Decoder::isObject(void *node)
{
    cJSON *cj = this->parseNode(node);
    return cj ? cJSON_IsObject(cj) : false;
}

void *NGenXX::Json::Decoder::readNode(void *node, const std::string &k)
{
    cJSON *cj = this->parseNode(node);
    if (cj != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(cj, k.c_str());
    }
    return NULL;
}

std::string NGenXX::Json::Decoder::readString(void *node)
{
    cJSON *cj = this->parseNode(node);
    if (cj != NULL && cJSON_IsString(cj))
    {                                                               \
        return std::string(cj->valuestring ?: "");
    }
    return "";
}

double NGenXX::Json::Decoder::readNumber(void *node)
{
    cJSON *cj = this->parseNode(node);
    if (cj != NULL && cJSON_IsNumber(cj))
    {
        return cj->valuedouble;
    }
    return 0;
}

void *NGenXX::Json::Decoder::readChild(void *node)
{
    cJSON *cj = this->parseNode(node);
    if (cj != NULL && (this->isArray(cj) || this->isObject(cj))) {
        return cj->child;
    }
    return NULL;
}

void *NGenXX::Json::Decoder::readNext(void *node)
{
    cJSON *cj = this->parseNode(node);
    return cj ? cj->next : NULL;
}

NGenXX::Json::Decoder::~Decoder()
{
    if (this->cjson != NULL)
    {
        cJSON_Delete(this->cjson);
    }
}