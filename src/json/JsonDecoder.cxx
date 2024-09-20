#include "JsonDecoder.hxx"

cJSON *NGenXX::Json::Decoder::parseObj(void *obj)
{
    if (obj == NULL)
        return this->cjson;
    return (cJSON *)obj;
}

NGenXX::Json::Decoder::Decoder(const std::string &json)
{
    this->cjson = cJSON_Parse(json.c_str());
}

bool NGenXX::Json::Decoder::isArray(void *obj)
{
    cJSON *cj = this->parseObj(obj);
    return cj ? cJSON_IsArray(cj) : false;
}

void *NGenXX::Json::Decoder::readItem(void *obj, const std::string &k)
{
    cJSON *cj = this->parseObj(obj);
    if (cj != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(cj, k.c_str());
    }
    return NULL;
}

std::string NGenXX::Json::Decoder::readString(void *obj)
{
    cJSON *cj = this->parseObj(obj);
    if (cj != NULL && cJSON_IsString(cj))
    {
        return cj->valuestring;
    }
    return "";
}

double NGenXX::Json::Decoder::readNumber(void *obj)
{
    cJSON *cj = this->parseObj(obj);
    if (cj != NULL && cJSON_IsNumber(cj))
    {
        return cj->valuedouble;
    }
    return 0;
}

void *NGenXX::Json::Decoder::readArray(void *obj)
{
    cJSON *cj = this->parseObj(obj);
    return cj ? cj->child : NULL;
}

void *NGenXX::Json::Decoder::readArrayNext(void *obj)
{
    if (!this->isArray(obj))
    {
        return NULL;
    }
    cJSON *cj = this->parseObj(obj);
    return cj ? cj->next : NULL;
}

NGenXX::Json::Decoder::~Decoder()
{
    if (this->cjson != NULL)
    {
        cJSON_Delete(this->cjson);
    }
}