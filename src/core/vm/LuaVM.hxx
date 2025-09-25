#pragma once

#include <DynXX/C/Macro.h>

DYNXX_EXTERN_C_BEGIN
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
DYNXX_EXTERN_C_END

#include <DynXX/CXX/Types.hxx>

#include "BaseVM.hxx"

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

namespace DynXX::Core::VM {
    class LuaVM final : public BaseVM {
    public:
        /**
         * @brief Create Lua environment
         */
        LuaVM();

        LuaVM(const LuaVM &) = delete;

        LuaVM &operator=(const LuaVM &) = delete;

        LuaVM(LuaVM &&) = delete;

        LuaVM &operator=(LuaVM &&) = delete;

        /**
         * @brief Load Lua file
         * @warning Will alert a prompt window in WebAssembly!
         * @param file Lua file path
         * @return success or not
         */
        [[nodiscard]] bool loadFile(const std::string &file);

        /**
         * @brief Load Lua script content
         * @param script Lua script content
         * @return success or not
         */
        [[nodiscard]] bool loadScript(const std::string &script);

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
        std::optional<std::string> callFunc(std::string_view func, std::string_view params);

        /**
         * @brief Release Lua environment
         */
        ~LuaVM() override;

    private:
        std::unique_ptr<lua_State, void(*)(lua_State*)> lstate;
    };
}
