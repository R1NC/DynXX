#ifndef NGENXX_SRC_LUA_BRIDGE_HXX_
#define NGENXX_SRC_LUA_BRIDGE_HXX_

extern "C"
{
#include "../../../external/lua/lua.h"
#include "../../../external/lua/lualib.h"
#include "../../../external/lua/lauxlib.h"
}

#ifdef __cplusplus

#include <string>
#include <mutex>

namespace NGenXX
{
    class LuaBridge
    {
    private:
        lua_State *lstate;
        std::mutex mutex;

    public:
        /**
         * @brief Create Lua environment
         */
        LuaBridge();

        /**
         * @brief Load Lua file
         * @warning Will alert a prompt window in WebAssembly!
         * @param file Lua file path
         * @return success or not
         */
        bool loadFile(const std::string &file);

        /**
         * @brief Load Lua script content
         * @param script Lua script content
         * @return success or not
         */
        bool loadScript(const std::string &script);

        /**
         * @brief export C function to Lua environment
         * @param funcName the exported function name
         * @param funcPointer the C function pointer
         */
        void bindFunc(const std::string &funcName, int (*funcPointer)(lua_State *));

        /**
         * @brief Call Lua function
         * @param func Lua function name
         * @param params Lua function params（wrap multiple params with json）
         * @return return value of Lua function
         */
        const std::string callFunc(const std::string &func, const std::string &params);

        /**
         * @brief Release Lua environment
         */
        ~LuaBridge();
    };
}

#endif

#endif // NGENXX_SRC_LUA_BRIDGE_HXX_
