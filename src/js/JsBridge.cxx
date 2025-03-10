#if defined(USE_QJS)
#include "JsBridge.hxx"

#include <cstdint>
#include <cstdlib>

#include <fstream>
#include <sstream>
#include <streambuf>
#include <utility>
#include <mutex>
#include <thread>

#include <uv.h>

#include <NGenXXLog.hxx>
#include "../util/TimeUtil.hxx"

constexpr auto IMPORT_STD_OS_JS = "import * as std from 'qjs:std';\n"
                                         "import * as os from 'qjs:os';\n"
                                         "globalThis.std = std;\n"
                                         "globalThis.os = os;\n";

constexpr auto NGenXXJsSleepMilliSecs = 1uz;

typedef struct NgenXXJSPromise
{
    JSValue p{JS_UNDEFINED};
    JSValue f[2]{JS_UNDEFINED, JS_UNDEFINED};
} NgenXXJSPromise;

static uv_loop_t *_ngenxx_js_uv_loop_p = nullptr;
static uv_loop_t *_ngenxx_js_uv_loop_t = nullptr;
static uv_timer_t *_ngenxx_js_uv_timer_p = nullptr;
static uv_timer_t *_ngenxx_js_uv_timer_t = nullptr;
static std::unique_ptr<std::recursive_timed_mutex> _ngenxx_js_mutex = nullptr;

static bool _ngenxx_js_try_acquire_lock()
{
    if (!_ngenxx_js_mutex) [[unlikely]]
    {
        return false;
    }
    auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(NGenXXJsSleepMilliSecs);
    return _ngenxx_js_mutex->try_lock_until(timeout);
}

static void _ngenxx_js_uv_timer_cb_p(uv_timer_t *timer)
{
    auto ctx = static_cast<JSContext *>(timer->data);
    /// Do not force to acquire the lock, to avoid blocking the JS event loop.
    if (_ngenxx_js_try_acquire_lock())
    {
        js_std_loop_promise(ctx);
        _ngenxx_js_mutex->unlock();
    }
}

static void _ngenxx_js_uv_timer_cb_t(uv_timer_t *timer)
{
    auto ctx = static_cast<JSContext *>(timer->data);
    /// Do not force to acquire the lock, to avoid blocking the JS event loop.
    if (_ngenxx_js_try_acquire_lock()) [[likely]]
    {
        js_std_loop_timer(ctx);
        _ngenxx_js_mutex->unlock();
    }
}

static void _ngenxx_js_uv_loop_start(JSContext *ctx, uv_loop_t *uv_loop, uv_timer_t *uv_timer, uv_timer_cb cb)
{
    if (uv_loop == nullptr) [[likely]]
    {
        uv_loop = mallocX<uv_loop_t>();
        uv_loop_init(uv_loop);
    }
    else [[unlikely]]
    {
        if (uv_loop_alive(uv_loop)) [[unlikely]]
        {
            return;
        }
    }

    if (uv_timer == nullptr) [[likely]]
    {
        uv_timer = mallocX<uv_timer_t>();
        uv_timer_init(uv_loop, uv_timer);
        uv_timer->data = ctx;
        uv_timer_start(uv_timer, cb, NGenXXJsSleepMilliSecs, NGenXXJsSleepMilliSecs);
    }
    else [[unlikely]]
    {
        uv_timer_stop(uv_timer);
        uv_timer_again(uv_timer);
    }

    uv_run(uv_loop, UV_RUN_DEFAULT);
}

static void _ngenxx_js_uv_loop_stop(uv_loop_t *uv_loop, uv_timer_t *uv_timer)
{
    if (uv_loop == nullptr || uv_timer == nullptr || !uv_loop_alive(uv_loop)) [[unlikely]]
    {
        return;
    }

    uv_timer_stop(uv_timer);
    freeX(uv_timer);

    uv_stop(uv_loop);
    uv_loop_close(uv_loop);
    freeX(uv_loop);
}

static void _ngenxx_js_loop_startP(JSContext *ctx)
{
    _ngenxx_js_uv_loop_start(ctx, _ngenxx_js_uv_loop_p, _ngenxx_js_uv_timer_p, _ngenxx_js_uv_timer_cb_p);
}

static void _ngenxx_js_loop_startT(JSContext *ctx)
{
    _ngenxx_js_uv_loop_start(ctx, _ngenxx_js_uv_loop_t, _ngenxx_js_uv_timer_t, _ngenxx_js_uv_timer_cb_t);
}

static void _ngenxx_js_loop_stop(JSRuntime *rt)
{
    _ngenxx_js_uv_loop_stop(_ngenxx_js_uv_loop_p, _ngenxx_js_uv_timer_p);
    _ngenxx_js_uv_loop_stop(_ngenxx_js_uv_loop_t, _ngenxx_js_uv_timer_t);

    js_std_loop_cancel(rt);
}

static void _ngenxx_js_print_err(JSContext *ctx, JSValueConst val)
{
    auto str = JS_ToCString(ctx, val);
    if (str)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, str);
        JS_FreeCString(ctx, str);
    }
}

static void _ngenxx_js_dump_err(JSContext *ctx)
{
    auto exception_val = JS_GetException(ctx);

    _ngenxx_js_print_err(ctx, exception_val);
    if (JS_IsError(ctx, exception_val))
    {
        auto val = JS_GetPropertyStr(ctx, exception_val, "stack");
        if (!JS_IsUndefined(val))
        {
            _ngenxx_js_print_err(ctx, val);
        }
        JS_FreeValue(ctx, val);
    }

    JS_FreeValue(ctx, exception_val);
}

bool _ngenxx_js_loadScript(JSContext *ctx, const std::string &script, const std::string &name, bool isModule)
{
    auto res = true;
    auto flags = isModule ? (JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY) : JS_EVAL_TYPE_GLOBAL;
    auto jEvalRet = JS_Eval(ctx, script.c_str(), script.length(), name.c_str(), flags);
    if (JS_IsException(jEvalRet)) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_Eval failed ->");
        _ngenxx_js_dump_err(ctx);
        return false;
    }

    if (isModule)
    {
        if (JS_VALUE_GET_TAG(jEvalRet) != JS_TAG_MODULE)
        { // Check whether it's a JS Module or not，or QJS may crash
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS try to load invalid module");
            JS_FreeValue(ctx, jEvalRet);
            return false;
        }
        js_module_set_import_meta(ctx, jEvalRet, false, true);
        auto jEvalFuncRet = JS_EvalFunction(ctx, jEvalRet);
        JS_FreeValue(ctx, jEvalFuncRet);
        // this->jValueCache.insert(std::move(jEvalRet));//Can not free here, or QJS may crash
    }
    else
    {
        JS_FreeValue(ctx, jEvalRet);
    }

    return res;
}

/// A JS Worker created a all new independent `JSContext`，so we should load the js files and modules again.
/// By default, we just load the built-in modules.
static JSContext *_ngenxx_js_newContext(JSRuntime *rt)
{
    auto ctx = JS_NewContext(rt);
    if (!ctx) [[unlikely]]
    {
        return nullptr;
    }

    js_std_add_helpers(ctx, 0, nullptr);
    js_init_module_std(ctx, "qjs:std");
    js_init_module_os(ctx, "qjs:os");

    _ngenxx_js_loadScript(ctx, IMPORT_STD_OS_JS, "import-std-os.js", true);

    return ctx;
}

NGenXX::JsBridge::JsBridge()
{
    _ngenxx_js_mutex = std::make_unique<std::recursive_timed_mutex>();

    this->runtime = JS_NewRuntime();
    js_std_init_handlers(this->runtime);
    JS_SetModuleLoaderFunc(this->runtime, nullptr, js_module_loader, nullptr);
    js_std_set_worker_new_context_func(_ngenxx_js_newContext);

    this->context = _ngenxx_js_newContext(this->runtime);
    this->jGlobal = JS_GetGlobalObject(this->context);// Can not free here, will be called in future

    std::thread([&ctx = this->context]
    {
        _ngenxx_js_loop_startP(ctx); 
    }).detach();
    std::thread([&ctx = this->context]
    {
        _ngenxx_js_loop_startT(ctx); 
    }).detach();
}

bool NGenXX::JsBridge::bindFunc(const std::string &funcJ, JSCFunction *funcC)
{
    auto res = true;
    auto jFunc = JS_NewCFunction(this->context, funcC, funcJ.c_str(), 1);
    if (JS_IsException(jFunc)) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_NewCFunction failed ->");
        _ngenxx_js_dump_err(this->context);
        res = false;
    }
    else [[likely]]
    {
        if (!JS_DefinePropertyValueStr(this->context, this->jGlobal, funcJ.c_str(), jFunc, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE)) [[unlikely]]
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_DefinePropertyValueStr failed ->");
            _ngenxx_js_dump_err(this->context);
            res = false;
        }
    }

    this->jValueCache.insert(std::move(jFunc)); // Can not free here, will be called in future

    return res;
}

bool NGenXX::JsBridge::loadFile(const std::string &file, bool isModule)
{
    std::ifstream ifs(file.c_str());
    std::stringstream ss;
    ss << ifs.rdbuf();
    return this->loadScript(ss.str(), file, isModule);
}

bool NGenXX::JsBridge::loadScript(const std::string &script, const std::string &name, bool isModule)
{
    auto lock = std::lock_guard(*_ngenxx_js_mutex);
    return _ngenxx_js_loadScript(this->context, script, name, isModule);
}

bool NGenXX::JsBridge::loadBinary(const Bytes &bytes, bool isModule)
{
    auto lock = std::lock_guard(*_ngenxx_js_mutex);
    return js_std_eval_binary(this->context, bytes.data(), bytes.size(), 0);
}

static inline std::string _ngenxx_js_jstr2stdstr(JSContext *ctx, JSValue jstr)
{
    auto c = JS_ToCString(ctx, jstr);
    auto s = std::string(c ?: "");
    JS_FreeCString(ctx, c);
    return s;
}

JSValue _ngenxx_js_await(JSContext *ctx, JSValue obj)
{
    auto ret = JS_UNDEFINED;
    for (;;)
    {
        /// Do not force to acquire the lock, to avoid blocking the JS event loop.
        if (!_ngenxx_js_try_acquire_lock()) [[unlikely]]
        {
            sleepForMilliSecs(NGenXXJsSleepMilliSecs);
            continue;
        }
        auto state = JS_PromiseState(ctx, obj);
        if (state == JS_PROMISE_FULFILLED)
        {
            ret = JS_PromiseResult(ctx, obj);
            JS_FreeValue(ctx, obj);
            break;
        }
        else if (state == JS_PROMISE_REJECTED)
        {
            ret = JS_Throw(ctx, JS_PromiseResult(ctx, obj));
            JS_FreeValue(ctx, obj);
            break;
        }
        else if (state == JS_PROMISE_PENDING)
        {
            /// Promise is executing: release the lock, sleep for a while. To avoid blocking the js event loop, or overloading CPU.
            _ngenxx_js_mutex->unlock();
            sleepForMilliSecs(NGenXXJsSleepMilliSecs);
            continue;
        }
        else
        {
            /// Not a Promise: release the lock, return the result immediately.
            ret = obj;
            break;
        }
    }
    _ngenxx_js_mutex->unlock();
    return ret;
}

/// WARNING: Nested call between native and JS requires a reenterable `recursive_mutex` here!
std::string NGenXX::JsBridge::callFunc(const std::string &func, const std::string &params, bool await)
{
    _ngenxx_js_mutex->lock();
    std::string s;

    auto jFunc = JS_GetPropertyStr(this->context, this->jGlobal, func.c_str());
    if (JS_IsFunction(this->context, jFunc)) [[likely]]
    {
        auto jParams = JS_NewString(this->context, params.c_str());
        JSValue argv[] = {jParams};

        auto jRes = JS_Call(this->context, jFunc, this->jGlobal, sizeof(argv), argv);

        /// Release the lock imediately, to avoid blocking the JS event loop.
        _ngenxx_js_mutex->unlock();

        if (JS_IsException(jRes)) [[unlikely]]
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_Call failed ->");
            _ngenxx_js_dump_err(this->context);
        }
        else [[likely]]
        {
            if (await)
            {/// WARNING: Do not use built-in `js_std_await()`, since it will triger the Promise Event Loop once again.
                jRes = _ngenxx_js_await(this->context, jRes); // Handle promise if needed
            }
            s = _ngenxx_js_jstr2stdstr(this->context, jRes);
        }
        JS_FreeValue(this->context, jRes);
        JS_FreeValue(this->context, jParams);
    }
    else [[unlikely]]
    {
        _ngenxx_js_mutex->unlock();
        ngenxxLogPrint(NGenXXLogLevelX::Error, ("Can not find JS func:" + func).c_str());
    }

    return s;
}

NgenXXJSPromise *_NgenXXJSPromise_new(JSContext *ctx)
{
    auto jPromise = new NgenXXJSPromise();
    jPromise->p = JS_NewPromiseCapability(ctx, jPromise->f);
    if (JS_IsException(jPromise->p)) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_NewPromise failed ->");
        _ngenxx_js_dump_err(ctx);
        JS_FreeValue(ctx, jPromise->p);
        return nullptr;
    }
    return jPromise;
}

void _NgenXXJSPromise_callback(JSContext *ctx, NgenXXJSPromise *jPromise, JSValue jRet)
{
    auto jCallRet = JS_Call(ctx, jPromise->f[0], JS_UNDEFINED, 1, &jRet);
    if (JS_IsException(jCallRet)) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_CallPromise failed ->");
        _ngenxx_js_dump_err(ctx);
    }

    JS_FreeValue(ctx, jCallRet);
    JS_FreeValue(ctx, jRet);
    JS_FreeValue(ctx, jPromise->f[0]);
    JS_FreeValue(ctx, jPromise->f[1]);
    // JS_FreeValue(ctx, jPromise->p);
    delete jPromise;
}

JSValue NGenXX::JsBridge::newPromise(const std::function<JSValue()> &jf)
{
    auto jPromise = _NgenXXJSPromise_new(this->context);
    if (jPromise == nullptr) [[unlikely]]
    {
        return JS_EXCEPTION;
    }
    
    std::thread([&ctx = this->context, jPro = jPromise, cb = jf] {
        if (!_ngenxx_js_mutex) [[unlikely]]
        {
            return;
        }
        auto lock = std::lock_guard(*_ngenxx_js_mutex);

        auto jRet = cb();

        _NgenXXJSPromise_callback(ctx, jPro, jRet);
    }).detach();

    return jPromise->p;
}

JSValue NGenXX::JsBridge::newPromiseVoid(const std::function<void()> &f)
{
    return this->newPromise([cb = f]() {
        cb();
        return JS_UNDEFINED;
    });
}

JSValue NGenXX::JsBridge::newPromiseBool(const std::function<bool()> &f)
{
    return this->newPromise([&ctx = this->context, cb = f]{
        auto ret = cb();
        return JS_NewBool(ctx, ret);
    });
}

JSValue NGenXX::JsBridge::newPromiseInt32(const std::function<int32_t()> &f)
{
    return this->newPromise([&ctx = this->context, cb = f]{
        auto ret = cb();
        return JS_NewInt32(ctx, ret);
    });
}

JSValue NGenXX::JsBridge::newPromiseInt64(const std::function<int64_t()> &f)
{
    return this->newPromise([&ctx = this->context, cb = f]{
        auto ret = cb();
        return JS_NewInt64(ctx, ret);
    });
}

JSValue NGenXX::JsBridge::newPromiseFloat(const std::function<double()> &f)
{
    return this->newPromise([&ctx = this->context, cb = f]{
        auto ret = cb();
        return JS_NewFloat64(ctx, ret);
    });
}

JSValue NGenXX::JsBridge::newPromiseString(const std::function<const std::string()> &f)
{
    return this->newPromise([&ctx = this->context, cb = f]{
        auto ret = cb();
        return JS_NewString(ctx, ret.c_str() ? : "");
    });
}

NGenXX::JsBridge::~JsBridge()
{
    _ngenxx_js_loop_stop(this->runtime);

    js_std_set_worker_new_context_func(nullptr);

    for (const auto &jv : this->jValueCache)
    {
        auto tag = JS_VALUE_GET_TAG(jv);
        if (tag == static_cast<decltype(tag)>(JS_TAG_MODULE)) [[unlikely]]
        {
            // Free a module will cause crash in QJS
            continue;
        }
        JS_FreeValue(this->context, jv);
    }
    JS_FreeValue(this->context, jGlobal);
    JS_FreeContext(this->context);

    js_std_free_handlers(this->runtime);
    JS_FreeRuntime(this->runtime);

    _ngenxx_js_mutex.reset();
}
#endif
