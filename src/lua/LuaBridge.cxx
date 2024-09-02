#ifdef __cplusplus

#include <string.h>
#include <stdlib.h>
#include <string>
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"

extern "C"
{
#include "../../../external/lua/lua.h"
#include "../../../external/lua/lualib.h"
#include "../../../external/lua/lauxlib.h"
}

#define PRINT_L_ERROR(L, prefix)                                                       \
    do                                                                                 \
    {                                                                                  \
        const char *luaErrMsg = lua_tostring(L, -1);                                   \
        if (luaErrMsg != NULL)                                                         \
        {                                                                              \
            Log::print(Error, (std::string(prefix) + std::string(luaErrMsg)).c_str()); \
        }                                                                              \
    } while (0);

namespace NGenXX
{
    namespace LuaBridge
    {
        lua_State *create(void)
        {
            lua_State *lstate = luaL_newstate();
            luaL_openlibs(lstate);
            return lstate;
        }

        void destroy(lua_State *lstate)
        {
            if (lstate == NULL)
            {
                Log::print(Error, "LuaBridge.destroy: lstate is NULL");
                return;
            }
            lua_close(lstate);
        }

        void bindFunc(lua_State *lstate, const char *funcName, int (*funcPointer)(lua_State *))
        {
            if (lstate == NULL)
            {
                Log::print(Error, "LuaBridge.bindFunc: lstate is NULL");
                return;
            }
            lua_pushcfunction(lstate, funcPointer);
            lua_setglobal(lstate, funcName);
        }

#ifndef __EMSCRIPTEN__
        int loadFile(lua_State *lstate, const char *file)
        {
            if (lstate == NULL)
            {
                Log::print(Error, "LuaBridge.loadFile: lstate is NULL");
                return LUA_ERRERR;
            }
            if (file == NULL)
            {
                Log::print(Error, "LuaBridge.loadFile: file is NULL");
                return LUA_ERRERR;
            }
            int ret = luaL_dofile(lstate, file);
            if (ret != LUA_OK)
            {
                PRINT_L_ERROR(lstate, "`luaL_dofile` error:");
            }
            return ret;
        }
#endif

        int loadScript(lua_State *lstate, const char *script)
        {
            if (lstate == NULL)
            {
                Log::print(Error, "LuaBridge.loadScript: lstate is NULL");
                return LUA_ERRERR;
            }
            if (script == NULL)
            {
                Log::print(Error, "LuaBridge.loadScript: script is NULL");
                return LUA_ERRERR;
            }
            int ret = luaL_dostring(lstate, script);
            if (ret != LUA_OK)
            {
                PRINT_L_ERROR(lstate, "`luaL_dostring` error:");
            }
            return ret;
        }

        const char *callFunc(lua_State *lstate, const char *func, const char *params)
        {
            if (lstate == NULL)
            {
                Log::print(Error, "LuaBridge.callFunc: lstate is NULL");
                return NULL;
            }
            if (func == NULL)
            {
                Log::print(Error, "LuaBridge.callFunc: func is NULL");
                return NULL;
            }
            lua_getglobal(lstate, func);
            lua_pushstring(lstate, params);
            int ret = lua_pcall(lstate, 1, 1, 0);
            if (ret != LUA_OK)
            {
                PRINT_L_ERROR(lstate, "`lua_pcall` error:");
                return NULL;
            }
            const char *res = lua_tostring(lstate, -1);

            // Or memory issues will occur while Lua VM freed but the pointer is referenced outside.
            char *cRes;
            if (res != NULL)
            {
                cRes = (char *)malloc(strlen(res) + 1);
                strcpy(cRes, res);
                // free((void *)res);
            }

            lua_pop(lstate, 1);
            return cRes;
        }
    }
}

#endif