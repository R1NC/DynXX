#if defined(USE_LUA)
#include "LuaBridge.hxx"

#if defined(USE_LIBUV)
#include <uv.h>
#endif

#include <NGenXXLog.hxx>
#include <NGenXXTypes.hxx>
namespace
{
    std::recursive_timed_mutex mutex;

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
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "{}{}", prefix, luaErrMsg); \
        }                                                                       \
    } while (0);
}

NGenXX::Bridge::LuaBridge::LuaBridge()
{
    this->lstate = luaL_newstate();
    luaL_openlibs(this->lstate);
#if defined(USE_LIBUV)
    lua_register_lib(this->lstate, "Timer", lib_timer_funcs);
    _loop_init();
#endif
}

NGenXX::Bridge::LuaBridge::~LuaBridge()
{
    this->shouldStop = true;
#if defined(USE_LIBUV)
    _loop_stop();
#endif    
    lua_close(this->lstate);
}

void NGenXX::Bridge::LuaBridge::bindFunc(const std::string &funcName, int (*funcPointer)(lua_State *)) const {
    lua_register(this->lstate, funcName.c_str(), funcPointer);
}

#if !defined(__EMSCRIPTEN__)
bool NGenXX::Bridge::LuaBridge::loadFile(const std::string &file) const
{
    auto lock = std::lock_guard(mutex);
    if (const auto ret = luaL_dofile(this->lstate, file.c_str()); ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(this->lstate, "`luaL_dofile` error:");
        return false;
    }
    return true;
}
#endif

bool NGenXX::Bridge::LuaBridge::loadScript(const std::string &script) const
{
    auto lock = std::lock_guard(mutex);
    if (const auto ret = luaL_dostring(this->lstate, script.c_str()); ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(this->lstate, "`luaL_dostring` error:");
        return false;
    }
    return true;
}

/// WARNING: Nested call between native and Lua requires a reenterable `recursive_mutex` here!
std::optional<std::string> NGenXX::Bridge::LuaBridge::callFunc(const std::string &func, const std::string &params) const
{
    auto lock = std::lock_guard(mutex);
    lua_getglobal(this->lstate, func.c_str());
    lua_pushstring(this->lstate, params.c_str());
    if (const auto ret = lua_pcall(lstate, 1, 1, 0); ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(this->lstate, "`lua_pcall` error:");
        return std::nullopt;
    }
    const auto &s = wrapStr(lua_tostring(this->lstate, -1));

    lua_pop(this->lstate, 1);
    return std::make_optional(std::move(s));
}
#endif
