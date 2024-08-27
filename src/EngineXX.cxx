#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../external/cjson/cJSON.h"

extern "C"
{
#include "../../../external/lua/lauxlib.h"
}

#ifdef __cplusplus

#include <string>
#include "net/HttpClient.hxx"
#include "lua/LuaBridge.hxx"

namespace EngineXX
{

    static inline const std::string GetVersion(void)
    {
        static const std::string s("0.0.1");
        return s;
    }

}

#endif

const char *enginexx_get_version(void)
{
    auto s = EngineXX::GetVersion();
    char *c = (char *)malloc(s.size());
    strcpy(c, s.c_str());
    return c;
}

int enginexx_get_versionL(lua_State *L)
{
    const char *res = enginexx_get_version();
    lua_pushstring(L, res);
    return 1;
}

const char *enginexx_http_req(const char *url, const char *params)
{
    const std::string sUrl(url);
    const std::string sParams(params);
    auto s = EngineXX::HttpClient::Request(sUrl, sParams);
    char *c = (char *)malloc(s.size());
    strcpy(c, s.c_str());
    return c;
}

int enginexx_http_reqL(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    char *url, *params;
    cJSON *json = cJSON_Parse(s);
    if (json)
    {
        cJSON *cj = json->child;
        while (cj)
        {
            if (cj->string && strcmp(cj->string, "url") == 0 && cj->type == cJSON_String)
            {
                url = cj->valuestring;
            }
            if (cj->string && strcmp(cj->string, "params") == 0 && cj->type == cJSON_String)
            {
                params = cj->valuestring;
            }
            cj = cj->next;
        }
        cJSON_free(json);
    }
    const char *res = enginexx_http_req(url, params);
    lua_pushstring(L, res);
    return 1;
}

void *enginexx_L_create(void)
{
    lua_State *lstate = EngineXX::LuaBridge::create();

    EngineXX::LuaBridge::bindFunc(lstate, "enginexx_get_versionL", enginexx_get_versionL);
    EngineXX::LuaBridge::bindFunc(lstate, "enginexx_http_reqL", enginexx_http_reqL);

    return lstate;
}

void enginexx_L_destroy(void *lstate)
{
    EngineXX::LuaBridge::destroy((lua_State *)lstate);
}

int enginexx_L_loadF(void *lstate, const char *file)
{
    return EngineXX::LuaBridge::loadFile((lua_State *)lstate, file);
}

int enginexx_L_loadS(void *lstate, const char *file)
{
    return EngineXX::LuaBridge::loadScript((lua_State *)lstate, file);
}

const char *enginexx_L_call(void *lstate, const char *func, const char *params)
{
    return EngineXX::LuaBridge::callFunc((lua_State *)lstate, func, params);
}