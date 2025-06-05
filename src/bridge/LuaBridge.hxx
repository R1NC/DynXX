#ifndef NGENXX_SRC_BRIDGE_LUA_HXX_
#define NGENXX_SRC_BRIDGE_LUA_HXX_

#include <NGenXXInternal.h>

EXTERN_C_BEGIN
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
EXTERN_C_END

#if defined(__cplusplus)

#include <string>
#include <optional>

#include "BaseBridge.hxx"

#define DEF_LUA_FUNC_VOID(fL, fS)           \
    int fL(lua_State *L)                    \
    {                                       \
        auto json = luaL_checkstring(L, 1); \
        fS(json);                           \
        return 1;                           \
    }

#define DEF_LUA_FUNC_STRING(fL, fS)         \
    int fL(lua_State *L)                    \
    {                                       \
        auto json = luaL_checkstring(L, 1); \
        const std::string res = fS(json);   \
        lua_pushstring(L, res.c_str());     \
        return 1;                           \
    }

#define DEF_LUA_FUNC_INTEGER(fL, fS)        \
    int fL(lua_State *L)                    \
    {                                       \
        auto json = luaL_checkstring(L, 1); \
        auto res = fS(json);                \
        lua_pushinteger(L, res);            \
        return 1;                           \
    }

#define DEF_LUA_FUNC_BOOL(fL, fS)           \
    int fL(lua_State *L)                    \
    {                                       \
        auto json = luaL_checkstring(L, 1); \
        bool res = fS(json);                \
        lua_pushboolean(L, res);            \
        return 1;                           \
    }

#define DEF_LUA_FUNC_FLOAT(fL, fS)          \
    int fL(lua_State *L)                    \
    {                                       \
        auto json = luaL_checkstring(L, 1); \
        double res = fS(json);              \
        lua_pushnumber(L, res);             \
        return 1;                           \
    }

namespace NGenXX::Bridge
{
    class LuaBridge : public BaseBridge
    {
    public:
        /**
         * @brief Create Lua environment
         */
        LuaBridge();
        LuaBridge(const LuaBridge &) = delete;
        LuaBridge &operator=(const LuaBridge &) = delete;
        LuaBridge(LuaBridge &&) = delete;
        LuaBridge &operator=(LuaBridge &&) = delete;

        /**
         * @brief Load Lua file
         * @warning Will alert a prompt window in WebAssembly!
         * @param file Lua file path
         * @return success or not
         */
        [[nodiscard]] bool loadFile(const std::string &file) const;

        /**
         * @brief Load Lua script content
         * @param script Lua script content
         * @return success or not
         */
        [[nodiscard]] bool loadScript(const std::string &script) const;

        /**
         * @brief export C function to Lua environment
         * @param funcName the exported function name
         * @param funcPointer the C function pointer
         */
        void bindFunc(const std::string &funcName, int (*funcPointer)(lua_State *)) const;

        /**
         * @brief Call Lua function
         * @param func Lua function name
         * @param params Lua function params（wrap multiple params with json）
         * @return return value of Lua function
         */
        std::optional<std::string> callFunc(const std::string &func, const std::string &params) const;

        /**
         * @brief Release Lua environment
         */
        ~LuaBridge() override;

    private:
        lua_State *lstate{nullptr};
        std::unique_ptr<std::thread> threadTimer{nullptr};
    };
}

#endif

#endif // NGENXX_SRC_BRIDGE_LUA_HXX_
