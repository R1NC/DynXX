#ifndef NGENXX_LUA_BRIDGE_H_
#define NGENXX_LUA_BRIDGE_H_

#ifdef __cplusplus

#include "../../../external/lua/lua.h"

#include <string>

namespace NGenXX
{
    namespace LuaBridge
    {
        /**
         * @brief Create Lua environment
         * @return `lua_State*`
         */
        lua_State *create(void);

        /**
         * @brief Load Lua file
         * @warning Will alert a prompt window in WebAssembly!
         * @param lstate `lua_State*`
         * @param file Lua file path
         * @return success or not
         */
        int loadFile(lua_State *lstate, const char *file);

        /**
         * @brief Load Lua script content
         * @param lstate `lua_State*`
         * @param script Lua script content
         * @return success or not
         */
        int loadScript(lua_State *lstate, const char *script);

        /**
         * @brief export C function to Lua environment
         * @param lstate `lua_State*`
         * @param funcName the exported function name
         * @param funcPointer the C function pointer
         */
        void bindFunc(lua_State *lstate, const char *funcName, int (*funcPointer)(lua_State *));

        /**
         * @brief Call Lua function
         * @param lstate `lua_State*`
         * @param func Lua function name
         * @param params Lua function params（wrap multiple params with json）
         * @return return value of Lua function
         */
        const char *callFunc(lua_State *lstate, const char *func, const char *params);

        /**
         * @brief Release Lua environment
         * @param lstate `lua_State*`
         */
        void destroy(lua_State *lstate);
    }
}

#endif

#endif // NGENXX_LUA_BRIDGE_H_