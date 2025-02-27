#if defined(USE_LUA)
#include "LuaBridge.hxx"

#include <cstring>
#include <cstdlib>

#include <thread>
#include <functional>

#include <uv.h>

#include <NGenXXLog.hxx>
#include <NGenXXTypes.hxx>

typedef struct
{
    lua_State *lState;
    int lFuncRef;
    size_t timeout;
    bool repeat;
    bool finished;
} ngenxx_lua_timer_data;

void ngenxx_lua_uv_loop_init()
{
    uv_loop_init(uv_default_loop());
}

void ngenxx_lua_uv_loop_prepare()
{
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void ngenxx_lua_uv_loop_stop()
{
    if (!uv_loop_alive(uv_default_loop())) [[unlikely]]
    {
        return;
    }
    uv_stop(uv_default_loop());
    uv_loop_close(uv_default_loop());
}

void ngenxx_lua_uv_timer_cb(uv_timer_t *timer)
{
    auto timer_data = static_cast<ngenxx_lua_timer_data *>(timer->data);
    lua_rawgeti(timer_data->lState, LUA_REGISTRYINDEX, timer_data->lFuncRef);
    lua_pcall(timer_data->lState, 0, 0, 0);
}

uv_timer_t *ngenxx_lua_uv_timer_start(ngenxx_lua_timer_data *timer_data)
{
    auto timer = mallocPtr<uv_timer_t>(1);
    timer->data = timer_data;
    
    std::thread([tmr = timer] {
        uv_timer_init(uv_default_loop(), tmr);
        auto timer_data = static_cast<ngenxx_lua_timer_data *>(tmr->data);
        uv_timer_start(tmr, ngenxx_lua_uv_timer_cb, timer_data->timeout, timer_data->repeat ? timer_data->timeout : 0);
        ngenxx_lua_uv_loop_prepare();
    }).detach();
    
    return timer;
}

void ngenxx_lua_uv_timer_stop(uv_timer_t *timer, bool release)
{
    auto timer_data = static_cast<ngenxx_lua_timer_data *>(timer->data);
    luaL_unref(timer_data->lState, LUA_REGISTRYINDEX, timer_data->lFuncRef);
    if (!timer_data->finished)
    {
        uv_timer_set_repeat(timer, 0);
        uv_timer_stop(timer);
        timer_data->finished = true;
    }
    if (release)
    {
        freePtr(timer->data);
        freePtr(timer);
    }
}

static int ngenxx_lua_util_timer_add(lua_State *L)
{
    auto timeout = lua_tointeger(L, 1);
    auto repeat = lua_toboolean(L, 2);
    auto timer_data = mallocPtr<ngenxx_lua_timer_data>(1);
    timer_data->lFuncRef = luaL_ref(L, LUA_REGISTRYINDEX);
    timer_data->lState = L;
    timer_data->timeout = timeout;
    timer_data->repeat = repeat != 0;
    
    auto timer = ngenxx_lua_uv_timer_start(timer_data);
    
    lua_pushlightuserdata(L, timer);
    return 1;
}

static int ngenxx_lua_util_timer_remove(lua_State *L)
{
    auto timer = static_cast<uv_timer_t *>(lua_touserdata(L, 1));
    if (timer != NULL) [[likely]]
    {
        ngenxx_lua_uv_timer_stop(timer, true);
    }
    return 0;
}

static const luaL_Reg ngenxx_lua_lib_timer_funcs[] = {
    {"add", ngenxx_lua_util_timer_add},
    {"remove", ngenxx_lua_util_timer_remove},
    {NULL, NULL} /* sentinel */
};

#define ngenxx_lua_register_lib(L, lib, funcs) \
    {                                          \
        luaL_newlib(L, funcs);                 \
        lua_setglobal(L, lib);                 \
    }

#define PRINT_L_ERROR(L, prefix)                                                \
    do                                                                          \
    {                                                                           \
        const char *luaErrMsg = lua_tostring(L, -1);                            \
        if (luaErrMsg != NULL)                                                  \
        {                                                                       \
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "{}{}", prefix, luaErrMsg); \
        }                                                                       \
    } while (0);

NGenXX::LuaBridge::LuaBridge()
{
    this->lstate = luaL_newstate();
    luaL_openlibs(this->lstate);
    
    ngenxx_lua_register_lib(this->lstate, "Timer", ngenxx_lua_lib_timer_funcs);
    
    ngenxx_lua_uv_loop_init();
}

NGenXX::LuaBridge::~LuaBridge()
{
    ngenxx_lua_uv_loop_stop();
    
    lua_close(this->lstate);
}

void NGenXX::LuaBridge::bindFunc(const std::string &funcName, int (*funcPointer)(lua_State *))
{
    lua_register(this->lstate, funcName.c_str(), funcPointer);
}

#if !defined(__EMSCRIPTEN__)
bool NGenXX::LuaBridge::loadFile(const std::string &file)
{
    auto lock = std::lock_guard(this->mutex);
    auto ret = luaL_dofile(this->lstate, file.c_str());
    if (ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(this->lstate, "`luaL_dofile` error:");
        return false;
    }
    return true;
}
#endif

bool NGenXX::LuaBridge::loadScript(const std::string &script)
{
    auto lock = std::lock_guard(this->mutex);
    auto ret = luaL_dostring(this->lstate, script.c_str());
    if (ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(this->lstate, "`luaL_dostring` error:");
        return false;
    }
    return true;
}

/// WARNING: Nested call between native and Lua requires a reenterable `recursive_mutex` here!
std::string NGenXX::LuaBridge::callFunc(const std::string &func, const std::string &params)
{
    std::string s;
    auto lock = std::lock_guard(this->mutex);
    lua_getglobal(this->lstate, func.c_str());
    lua_pushstring(this->lstate, params.c_str());
    auto ret = lua_pcall(lstate, 1, 1, 0);
    if (ret != LUA_OK) [[unlikely]]
    {
        PRINT_L_ERROR(this->lstate, "`lua_pcall` error:");
        return s;
    }
    auto res = lua_tostring(this->lstate, -1);
    s = std::string(res ?: "");

    lua_pop(this->lstate, 1);
    return s;
}
#endif
