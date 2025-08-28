#if defined(USE_LUA)
#include "LuaVM.hxx"

#if defined(USE_LIBUV)
#include <uv.h>
#endif

#include <DynXX/CXX/Log.hxx>
#include <DynXX/CXX/Types.hxx>

namespace {
    using enum DynXXLogLevelX;
    
#if defined(USE_LIBUV)
    struct Timer
    {
        lua_State *lState{nullptr};
        int lFuncRef{0};
        lua_Integer timeout{1};
        bool repeat{false};
        bool finished{false};
    };

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

    void _timer_cb(uv_timer_t *timer)
    {
        const auto timer_data = static_cast<Timer *>(timer->data);
        lua_rawgeti(timer_data->lState, LUA_REGISTRYINDEX, timer_data->lFuncRef);
        lua_pcall(timer_data->lState, 0, 0, 0);
    }

    uv_timer_t *_timer_start(Timer *timer_data)
    {
        auto timerP = mallocX<uv_timer_t>();
        timerP->data = timer_data;

        std::thread([timerP] {
            uv_timer_init(uv_default_loop(), timerP);
            const auto data = static_cast<Timer *>(timerP->data);
            uv_timer_start(timerP, _timer_cb, data->timeout, data->repeat ? data->timeout : 0);
            _loop_prepare();
        }).detach();

        return timerP;
    }

    void _timer_stop(uv_timer_t *timer, bool release)
    {
        const auto timer_data = static_cast<Timer *>(timer->data);
        luaL_unref(timer_data->lState, LUA_REGISTRYINDEX, timer_data->lFuncRef);
        if (!timer_data->finished)
        {
            uv_timer_set_repeat(timer, 0);
            uv_timer_stop(timer);
            timer_data->finished = true;
        }
        if (release)
        {
            freeX(timer->data);
            freeX(timer);
        }
    }

    int _util_timer_add(lua_State *L)
    {
        const auto timer_data = mallocX<Timer>();
        *timer_data = {
            .lState = L,
            .lFuncRef = luaL_ref(L, LUA_REGISTRYINDEX),
            .timeout = lua_tointeger(L, 1),
            .repeat = static_cast<bool>(lua_toboolean(L, 2))
        };

        const auto timer = _timer_start(timer_data);

        lua_pushlightuserdata(L, timer);
        return LUA_OK;
    }

    int _util_timer_remove(lua_State *L)
    {
        if (const auto timer = static_cast<uv_timer_t *>(lua_touserdata(L, 1)); timer != nullptr) [[likely]]
        {
            _timer_stop(timer, true);
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

DynXX::Core::VM::LuaVM::LuaVM()
{
    this->lstate = luaL_newstate();
    luaL_openlibs(this->lstate);
#if defined(USE_LIBUV)
    lua_register_lib(this->lstate, "Timer", lib_timer_funcs);
    _loop_init();
#endif
}

DynXX::Core::VM::LuaVM::~LuaVM()
{
    this->active = false;
#if defined(USE_LIBUV)
    _loop_stop();
#endif    
    lua_close(this->lstate);
}

void DynXX::Core::VM::LuaVM::bindFunc(const std::string &funcName, int (*funcPointer)(lua_State *)) const {
    lua_register(this->lstate, funcName.c_str(), funcPointer);
}

#if !defined(__EMSCRIPTEN__)
bool DynXX::Core::VM::LuaVM::loadFile(const std::string &file)
{
    auto lock = std::scoped_lock(this->vmMutex);
    if (const auto ret = luaL_dofile(this->lstate, file.c_str()); ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(this->lstate, "`luaL_dofile` error:");
        return false;
    }
    return true;
}
#endif

bool DynXX::Core::VM::LuaVM::loadScript(const std::string &script)
{
    auto lock = std::scoped_lock(this->vmMutex);
    if (const auto ret = luaL_dostring(this->lstate, script.c_str()); ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(this->lstate, "`luaL_dostring` error:");
        return false;
    }
    return true;
}

/// WARNING: Nested call between native and Lua requires a reenterable `recursive_mutex` here!
std::optional<std::string> DynXX::Core::VM::LuaVM::callFunc(std::string_view func, std::string_view params)
{
    auto lock = std::scoped_lock(this->vmMutex);
    lua_getglobal(this->lstate, func.data());
    lua_pushstring(this->lstate, params.data());
    if (const auto ret = lua_pcall(this->lstate, 1, 1, 0); ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(this->lstate, "`lua_pcall` error:");
        return std::nullopt;
    }
    const auto s = makeStr(lua_tostring(this->lstate, -1));

    lua_pop(this->lstate, 1);
    return {s};
}
#endif
