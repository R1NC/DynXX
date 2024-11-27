#include "LuaBridge.hxx"

#include <string.h>
#include <stdlib.h>
#include "../../include/NGenXXLog.hxx"

#define PRINT_L_ERROR(L, prefix)                                                                  \
    do                                                                                            \
    {                                                                                             \
        const char *luaErrMsg = lua_tostring(L, -1);                                              \
        if (luaErrMsg != NULL)                                                                    \
        {                                                                                         \
            ngenxxLogPrint(NGenXXLogLevelX::Error, std::string(prefix) + std::string(luaErrMsg)); \
        }                                                                                         \
    } while (0);

NGenXX::LuaBridge::LuaBridge()
{
    this->lstate = luaL_newstate();
    luaL_openlibs(this->lstate);
}

NGenXX::LuaBridge::~LuaBridge()
{
    lua_close(this->lstate);
}

void NGenXX::LuaBridge::bindFunc(const std::string &funcName, int (*funcPointer)(lua_State *))
{
    lua_pushcfunction(this->lstate, funcPointer);
    lua_setglobal(this->lstate, funcName.c_str());
}

#ifndef __EMSCRIPTEN__
bool NGenXX::LuaBridge::loadFile(const std::string &file)
{
    const std::lock_guard<std::mutex> lock(this->mutex);
    int ret = luaL_dofile(this->lstate, file.c_str());
    if (ret != LUA_OK)
    {
        PRINT_L_ERROR(this->lstate, "`luaL_dofile` error:");
        return false;
    }
    return true;
}
#endif

bool NGenXX::LuaBridge::loadScript(const std::string &script)
{
    const std::lock_guard<std::mutex> lock(this->mutex);
    int ret = luaL_dostring(this->lstate, script.c_str());
    if (ret != LUA_OK)
    {
        PRINT_L_ERROR(this->lstate, "`luaL_dostring` error:");
        return false;
    }
    return true;
}

const std::string NGenXX::LuaBridge::callFunc(const std::string &func, const std::string &params)
{
    std::string s;
    const std::lock_guard<std::mutex> lock(this->mutex);
    lua_getglobal(this->lstate, func.c_str());
    lua_pushstring(this->lstate, params.c_str());
    int ret = lua_pcall(lstate, 1, 1, 0);
    if (ret != LUA_OK)
    {
        PRINT_L_ERROR(this->lstate, "`lua_pcall` error:");
        return s;
    }
    const char *res = lua_tostring(this->lstate, -1);
    s = std::move(std::string(res ?: ""));

    lua_pop(this->lstate, 1);
    return s;
}
