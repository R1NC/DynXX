#include "JsonDecoder.hxx"

cJSON *NGenXX::Json::Decoder::parseNode(void *const node)
{
    if (node == NULL)
        return this->cjson;
    return reinterpret_cast<cJSON *>(const_cast<void *>(node));
}

NGenXX::Json::Decoder::Decoder(const std::string &json)
{
    this->cjson = cJSON_Parse(json.c_str());
}

bool NGenXX::Json::Decoder::isArray(void *const node)
{
    cJSON *cj = this->parseNode(node);
    return cj ? cJSON_IsArray(cj) : false;
}

bool NGenXX::Json::Decoder::isObject(void *const node)
{
    cJSON *cj = this->parseNode(node);
    return cj ? cJSON_IsObject(cj) : false;
}

void *const NGenXX::Json::Decoder::readNode(void *const node, const std::string &k)
{
    cJSON *cj = this->parseNode(node);
    if (cj != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(cj, k.c_str());
    }
    return NULL;
}

std::string NGenXX::Json::Decoder::readString(void *const node)
{
    cJSON *cj = this->parseNode(node);
    if (cj != NULL && cJSON_IsString(cj))
    {
        return std::string(cj->valuestring ?: "");
    }
    return "";
}

double NGenXX::Json::Decoder::readNumber(void *const node)
{
    cJSON *cj = this->parseNode(node);
    if (cj != NULL && cJSON_IsNumber(cj))
    {
        return cj->valuedouble;
    }
    return 0;
}

void *const NGenXX::Json::Decoder::readChild(void *const node)
{
    cJSON *cj = this->parseNode(node);
    if (cj != NULL && (this->isArray(cj) || this->isObject(cj)))
    {
        return cj->child;
    }
    return NULL;
}

void NGenXX::Json::Decoder::readChildren(void *const node, std::function<void(int idx, void *child)> callback)
{
    if (!this->isArray(node) && !this->isObject(node))
        return;
    void *childNode = this->readChild(node);
    int idx = 0;
    while (childNode)
    {
        if (callback)
            callback(idx++, childNode);
        childNode = this->readNext(childNode);
    }
}

void *const NGenXX::Json::Decoder::readNext(void *const node)
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