#if defined(USE_LUA)
#include "LuaVM.hxx"

#include <memory>

#if defined(USE_LIBUV)
#include <uv.h>
#endif

#include <DynXX/CXX/Log.hxx>
#include <DynXX/CXX/Types.hxx>

namespace {
    using enum DynXXLogLevelX;
    using namespace DynXX::Core::Concurrent;
    
#if defined(USE_LIBUV)
    struct LuaTimer
    {
        lua_State *L{nullptr};
        int lFuncRef{0};
        lua_Integer timeout{1};
        bool repeat{false};
        bool finished{false};

        LuaTimer() = delete;
        LuaTimer(const LuaTimer &) = delete;
        LuaTimer(LuaTimer &&) = delete;
        LuaTimer &operator=(const LuaTimer &) = delete;
        LuaTimer &operator=(LuaTimer &&) = delete;

        explicit LuaTimer(lua_State *L) : L(L) {
            this->lFuncRef = luaL_ref(L, LUA_REGISTRYINDEX);
            this->timeout = lua_tointeger(L, 1);
            this->repeat = static_cast<bool>(lua_toboolean(L, 2));
        }

        ~LuaTimer()
        {
            luaL_unref(this->L, LUA_REGISTRYINDEX, this->lFuncRef);
        }
    };

    std::unique_ptr<Executor> timerExecutor = nullptr;

    void _loop_init()
    {
        uv_loop_init(uv_default_loop());
    }

    void _loop_prepare()
    {
        uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    }

    void _loop_stop()
    {
        if (!uv_loop_alive(uv_default_loop())) [[unlikely]]
        {
            return;
        }
        uv_stop(uv_default_loop());
        uv_loop_close(uv_default_loop());
    }

    void _timer_cb(uv_timer_t *uvTimer)
    {
        const auto lTimer = static_cast<LuaTimer *>(uvTimer->data);
        lua_rawgeti(lTimer->L, LUA_REGISTRYINDEX, lTimer->lFuncRef);
        lua_pcall(lTimer->L, 0, 0, 0);
    }

    uv_timer_t *_timer_start(LuaTimer *timer)
    {
        if (!timerExecutor) [[unlikely]]
        {
            return nullptr;
        }
        
        auto timerP = mallocX<uv_timer_t>();
        timerP->data = timer;

        (*timerExecutor) >> [timerP] {
            uv_timer_init(uv_default_loop(), timerP);
            const auto lTimer = static_cast<LuaTimer *>(timerP->data);
            uv_timer_start(timerP, _timer_cb, lTimer->timeout, lTimer->repeat ? lTimer->timeout : 0);
            _loop_prepare();
        };

        return timerP;
    }

    void _timer_stop(uv_timer_t *uvTimer, bool release)
    {
        const auto lTimer = static_cast<LuaTimer *>(uvTimer->data);
        if (!lTimer->finished)
        {
            uv_timer_set_repeat(uvTimer, 0);
            uv_timer_stop(uvTimer);
            lTimer->finished = true;
        }
        if (release)
        {
            freeX(uvTimer->data);
            freeX(uvTimer);
        }
    }

    int _util_timer_add(lua_State *L)
    {
        const auto lTimer = new LuaTimer(L);

        const auto uvTimer = _timer_start(lTimer);

        lua_pushlightuserdata(L, uvTimer);
        return LUA_OK;
    }

    int _util_timer_remove(lua_State *L)
    {
        if (const auto uvTimer = static_cast<uv_timer_t *>(lua_touserdata(L, 1)); uvTimer != nullptr) [[likely]]
        {
            _timer_stop(uvTimer, true);
        }
        return LUA_OK;
    }

    constexpr luaL_Reg lib_timer_funcs[] = {
        {"add", _util_timer_add},
        {"remove", _util_timer_remove},
        {nullptr, nullptr} /* sentinel */
    };
#endif

    #define lua_register_lib(L, lib, funcs)    \
    {                                          \
        luaL_newlib(L, funcs);                 \
        lua_setglobal(L, lib);                 \
    }

    #define PRINT_L_ERROR(L, prefix)                                            \
    do                                                                          \
    {                                                                           \
        const char *luaErrMsg = lua_tostring(L, -1);                            \
        if (luaErrMsg != nullptr)                                               \
        {                                                                       \
            dynxxLogPrintF(Error, "{}{}", prefix, luaErrMsg); \
        }                                                                       \
    } while (0)
}

DynXX::Core::VM::LuaVM::LuaVM() : lstate(luaL_newstate(), lua_close)
{
    const auto L = this->lstate.get();
    luaL_openlibs(L);
#if defined(USE_LIBUV)
    lua_register_lib(L, "Timer", lib_timer_funcs);
    _loop_init();
    timerExecutor = std::make_unique<Executor>(1, 1000uz);
#endif
}

DynXX::Core::VM::LuaVM::~LuaVM()
{
    this->active = false;
#if defined(USE_LIBUV)
    timerExecutor.reset();
    _loop_stop();
#endif
}

void DynXX::Core::VM::LuaVM::bindFunc(const std::string &funcName, int (*funcPointer)(lua_State *)) const {
    lua_register(this->lstate.get(), funcName.c_str(), funcPointer);
}

#if !defined(__EMSCRIPTEN__)
bool DynXX::Core::VM::LuaVM::loadFile(const std::string &file)
{
    auto lock = std::scoped_lock(this->vmMutex);
    const auto L = this->lstate.get();
    if (const auto ret = luaL_dofile(L, file.c_str()); ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(L, "`luaL_dofile` error:");
        return false;
    }
    return true;
}
#endif

bool DynXX::Core::VM::LuaVM::loadScript(const std::string &script)
{
    auto lock = std::scoped_lock(this->vmMutex);
    const auto L = this->lstate.get();
    if (const auto ret = luaL_dostring(L, script.c_str()); ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(L, "`luaL_dostring` error:");
        return false;
    }
    return true;
}

/// WARNING: Nested call between native and Lua requires a reenterable `recursive_mutex` here!
std::optional<std::string> DynXX::Core::VM::LuaVM::callFunc(std::string_view func, std::string_view params)
{
    auto lock = std::scoped_lock(this->vmMutex);
    const auto L = this->lstate.get();
    lua_getglobal(L, func.data());
    lua_pushstring(L, params.data());
    if (const auto ret = lua_pcall(L, 1, 1, 0); ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(L, "`lua_pcall` error:");
        return std::nullopt;
    }
    const auto s = makeStr(lua_tostring(L, -1));

    lua_pop(L, 1);
    return {s};
}
#endif
