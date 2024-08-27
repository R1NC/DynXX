#ifdef __cplusplus

extern "C"
{
#include "../../../external/lua/lua.h"
#include "../../../external/lua/lualib.h"
#include "../../../external/lua/lauxlib.h"
}

#define PRINT_L_ERROR(L, prefix) fprintf(stderr, "%s %s\n", prefix, lua_tostring(L, -1))

namespace EngineXX
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
            lua_close(lstate);
        }

        void bindFunc(lua_State *lstate, const char *funcName, int (*funcPointer)(lua_State *))
        {
            lua_pushcfunction(lstate, funcPointer);
            lua_setglobal(lstate, funcName);
        }

        int loadFile(lua_State *lstate, const char *file)
        {
            int ret = luaL_dofile(lstate, file);
            if (ret != LUA_OK)
            {
                PRINT_L_ERROR(lstate, "`luaL_dofile` error:");
            }
            return ret;
        }

        int loadScript(lua_State *lstate, const char *script)
        {
            int ret = luaL_dostring(lstate, script);
            if (ret != LUA_OK)
            {
                PRINT_L_ERROR(lstate, "`luaL_dostring` error:");
            }
            return ret;
        }

        const char *callFunc(lua_State *lstate, const char *func, const char *params)
        {
            lua_getglobal(lstate, func);
            lua_pushstring(lstate, params);
            int ret = lua_pcall(lstate, 1, 1, 0);
            if (ret != LUA_OK)
            {
                PRINT_L_ERROR(lstate, "`lua_pcall` error:");
                return NULL;
            }
            const char *res = lua_tostring(lstate, -1);
            lua_pop(lstate, 1);
            return res;
        }
    }
}

#endif