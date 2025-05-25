#include "quickjs.h"
#if defined(USE_QJS)
#include "JsBridge.hxx"

#include <fstream>
#include <sstream>
#include <utility>
#include <mutex>
#include <thread>

#include <uv.h>

#include <NGenXXLog.hxx>
#include "../util/TimeUtil.hxx"

namespace
{
    constexpr auto IMPORT_STD_OS_JS = "import * as std from 'qjs:std';\n"
                                         "import * as os from 'qjs:os';\n"
                                         "globalThis.std = std;\n"
                                         "globalThis.os = os;\n";

    constexpr auto SleepMilliSecs = 1uz;

#pragma mark JsBridge dump error

    void printJsErr(JSContext *ctx, const JSValueConst val)
    {
        if (const auto str = JS_ToCString(ctx, val))
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, str);
            JS_FreeCString(ctx, str);
        }
    }

    void dumpJsErr(JSContext *ctx)
    {
        const auto exception_val = JS_GetException(ctx);

        printJsErr(ctx, exception_val);
        if (JS_IsError(ctx, exception_val))
        {
            const auto val = JS_GetPropertyStr(ctx, exception_val, "stack");
            if (!JS_IsUndefined(val))
            {
                printJsErr(ctx, val);
            }
            JS_FreeValue(ctx, val);
        }

        JS_FreeValue(ctx, exception_val);
    }

#pragma mark JS Promise wrapper

    typedef struct Promise
    {
        JSValue p{JS_UNDEFINED};
        JSValue f[2]{JS_UNDEFINED, JS_UNDEFINED};
    } Promise;

    Promise *_newPromise(JSContext *ctx)
    {
        const auto jPromise = new Promise();
        jPromise->p = JS_NewPromiseCapability(ctx, jPromise->f);
        if (JS_IsException(jPromise->p)) [[unlikely]]
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_NewPromise failed ->");
            dumpJsErr(ctx);
            JS_FreeValue(ctx, jPromise->p);
            return nullptr;
        }
        return jPromise;
    }

    void _callbackPromise(JSContext *ctx, const Promise *jPromise, JSValue jRet)
    {
        const auto jCallRet = JS_Call(ctx, jPromise->f[0], JS_UNDEFINED, 1, &jRet);
        if (JS_IsException(jCallRet)) [[unlikely]]
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_CallPromise failed ->");
            dumpJsErr(ctx);
        }

        JS_FreeValue(ctx, jCallRet);
        JS_FreeValue(ctx, jRet);
        JS_FreeValue(ctx, jPromise->f[0]);
        JS_FreeValue(ctx, jPromise->f[1]);
        // JS_FreeValue(ctx, jPromise->p);
        delete jPromise;
    }

#pragma mark JsBridge Event Loop

    uv_loop_t *_uv_loop_p = nullptr;
    uv_loop_t *_uv_loop_t = nullptr;
    uv_timer_t *_uv_timer_p = nullptr;
    uv_timer_t *_uv_timer_t = nullptr;
    std::unique_ptr<std::recursive_timed_mutex> mutex = nullptr;

    bool tryLock()
    {
        if (!mutex) [[unlikely]]
        {
            return false;
        }
        const auto timeout = steady_clock::now() + milliseconds(SleepMilliSecs);
        return mutex->try_lock_until(timeout);
    }

    void _uv_timer_cb_p(uv_timer_t *timer)
    {
        const auto ctx = static_cast<JSContext *>(timer->data);
        /// Do not force to acquire the lock, to avoid blocking the JS event loop.
        if (tryLock())
        {
            js_std_loop_promise(ctx);
            mutex->unlock();
        }
    }

    void _uv_timer_cb_t(uv_timer_t *timer)
    {
        const auto ctx = static_cast<JSContext *>(timer->data);
        /// Do not force to acquire the lock, to avoid blocking the JS event loop.
        if (tryLock()) [[likely]]
        {
            js_std_loop_timer(ctx);
            mutex->unlock();
        }
    }

    void _uv_loop_start(JSContext *ctx, uv_loop_t *uv_loop, uv_timer_t *uv_timer, const uv_timer_cb cb)
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
            uv_timer_start(uv_timer, cb, SleepMilliSecs, SleepMilliSecs);
        }
        else [[unlikely]]
        {
            uv_timer_stop(uv_timer);
            uv_timer_again(uv_timer);
        }

        uv_run(uv_loop, UV_RUN_DEFAULT);
    }

    void _uv_loop_stop(uv_loop_t *uv_loop, uv_timer_t *uv_timer)
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

    void loop_startP(JSContext *ctx)
    {
        _uv_loop_start(ctx, _uv_loop_p, _uv_timer_p, _uv_timer_cb_p);
    }

    void loop_startT(JSContext *ctx)
    {
        _uv_loop_start(ctx, _uv_loop_t, _uv_timer_t, _uv_timer_cb_t);
    }

    void loop_stop(JSRuntime *rt)
    {
        _uv_loop_stop(_uv_loop_p, _uv_timer_p);
        _uv_loop_stop(_uv_loop_t, _uv_timer_t);

        js_std_loop_cancel(rt);
    }

#pragma mark JsBridge Internal

    bool _loadScript(JSContext *ctx, const std::string &script, const std::string &name, const bool isModule)
    {
        const auto flags = isModule ? (JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY) : JS_EVAL_TYPE_GLOBAL;
        const auto jEvalRet = JS_Eval(ctx, script.c_str(), script.length(), name.c_str(), flags);
        if (JS_IsException(jEvalRet)) [[unlikely]]
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_Eval failed ->");
            dumpJsErr(ctx);
            return false;
        }

        if (isModule)
        {
            if (JS_VALUE_GET_TAG(jEvalRet) != JS_TAG_MODULE) [[unlikely]]
            { // Check whether it's a JS Module or not，or QJS may crash
                ngenxxLogPrint(NGenXXLogLevelX::Error, "JS try to load invalid module");
                JS_FreeValue(ctx, jEvalRet);
                return false;
            }
            js_module_set_import_meta(ctx, jEvalRet, false, true);
            const auto jEvalFuncRet = JS_EvalFunction(ctx, jEvalRet);
            JS_FreeValue(ctx, jEvalFuncRet);
            // this->jValueCache.insert(std::move(jEvalRet));//Can not free here, or QJS may crash
        }
        else
        {
            JS_FreeValue(ctx, jEvalRet);
        }

        return true;
    }

    /// A JS Worker created a all new independent `JSContext`，so we should load the js files and modules again.
    /// By default, we just load the built-in modules.
    JSContext *_newContext(JSRuntime *rt)
    {
        const auto ctx = JS_NewContext(rt);
        if (!ctx) [[unlikely]]
        {
            return nullptr;
        }

        js_std_add_helpers(ctx, 0, nullptr);
        js_init_module_std(ctx, "qjs:std");
        js_init_module_os(ctx, "qjs:os");

        _loadScript(ctx, IMPORT_STD_OS_JS, "import-std-os.js", true);

        return ctx;
    }

    JSValue _await(JSContext *ctx, const JSValue obj)
    {
        if (!mutex) [[unlikely]]
        {
            return JS_UNDEFINED;
        }
        auto ret = JS_UNDEFINED;
        for (;;)
        {
            /// Do not force to acquire the lock, to avoid blocking the JS event loop.
            if (!tryLock()) [[unlikely]]
            {
                sleepForMilliSecs(SleepMilliSecs);
                continue;
            }
            if (const auto state = JS_PromiseState(ctx, obj); state == JS_PROMISE_FULFILLED)
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
                mutex->unlock();
                sleepForMilliSecs(SleepMilliSecs);
                continue;
            }
            else
            {
                /// Not a Promise: release the lock, return the result immediately.
                ret = obj;
                break;
            }
        }
        mutex->unlock();
        return ret;
    }
}

#pragma mark JSValueHash & JSValueEqual

std::size_t NGenXX::JsBridge::JSValueHash::operator()(const JSValue &jv) const 
{
    return std::hash<void *>()(JS_VALUE_GET_PTR(jv));
}

bool NGenXX::JsBridge::JSValueEqual::operator()(const JSValue &left, const JSValue &right) const 
{
    if (JS_VALUE_GET_TAG(left) != JS_VALUE_GET_TAG(right)) 
    {
        return false;
    }
    const auto tag = JS_VALUE_GET_TAG(left);
    if (tag == JS_TAG_BOOL && JS_VALUE_GET_BOOL(left) != JS_VALUE_GET_BOOL(right))
    {
        return false;
    }
    if (tag == JS_TAG_INT && JS_VALUE_GET_INT(left) != JS_VALUE_GET_INT(right))
    {
        return false;
    }
    if (tag == JS_TAG_FLOAT64 && JS_VALUE_GET_FLOAT64(left) != JS_VALUE_GET_FLOAT64(right))
    {
        return false;
    }
    return JS_VALUE_GET_PTR(left) == JS_VALUE_GET_PTR(right);
}

#pragma mark JsBridge API

NGenXX::JsBridge::JsBridge()
{
    mutex = std::make_unique<std::recursive_timed_mutex>();

    this->runtime = JS_NewRuntime();
    js_std_init_handlers(this->runtime);
    JS_SetModuleLoaderFunc(this->runtime, nullptr, js_module_loader, nullptr);
    js_std_set_worker_new_context_func(_newContext);

    this->context = _newContext(this->runtime);
    this->jGlobal = JS_GetGlobalObject(this->context);// Can not free here, will be called in future

    std::thread([ctx = this->context]
    {
        loop_startP(ctx); 
    }).detach();
    std::thread([ctx = this->context]
    {
        loop_startT(ctx); 
    }).detach();
}

bool NGenXX::JsBridge::bindFunc(const std::string &funcJ, JSCFunction *funcC)
{
    auto res = true;
    const auto jFunc = JS_NewCFunction(this->context, funcC, funcJ.c_str(), 1);
    if (JS_IsException(jFunc)) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_NewCFunction failed ->");
        dumpJsErr(this->context);
        res = false;
    }
    else [[likely]]
    {
        if (!JS_DefinePropertyValueStr(this->context, this->jGlobal, funcJ.c_str(), jFunc, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE)) [[unlikely]]
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_DefinePropertyValueStr failed ->");
            dumpJsErr(this->context);
            res = false;
        }
    }

    this->jValueCache.insert(jFunc); // Can not free here, will be called in future

    return res;
}

bool NGenXX::JsBridge::loadFile(const std::string &file, bool isModule) const
{
    std::ifstream ifs(file.c_str());
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return this->loadScript(ss.str(), file, isModule);
}

bool NGenXX::JsBridge::loadScript(const std::string &script, const std::string &name, bool isModule) const {
    if (!mutex) [[unlikely]]
    {
        return false;
    }
    auto lock = std::lock_guard(*mutex);
    return _loadScript(this->context, script, name, isModule);
}

bool NGenXX::JsBridge::loadBinary(const Bytes &bytes, bool isModule) const {
    if (!mutex) [[unlikely]]
    {
        return false;
    }
    auto lock = std::lock_guard(*mutex);
    return js_std_eval_binary(this->context, bytes.data(), bytes.size(), 0);
}

/// WARNING: Nested call between native and JS requires a reenterable `recursive_mutex` here!
std::string NGenXX::JsBridge::callFunc(const std::string &func, const std::string &params, bool await) const {
    if (!mutex) [[unlikely]]
    {
        return {};
    }
    mutex->lock();
    std::string s;

    if (const auto jFunc = JS_GetPropertyStr(this->context, this->jGlobal, func.c_str()); JS_IsFunction(this->context, jFunc)) [[likely]]
    {
        const auto jParams = JS_NewString(this->context, params.c_str());
        JSValue argv[] = {jParams};

        auto jRes = JS_Call(this->context, jFunc, this->jGlobal, sizeof(argv), argv);

        /// Release the lock imediately, to avoid blocking the JS event loop.
        mutex->unlock();

        if (JS_IsException(jRes)) [[unlikely]]
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_Call failed ->");
            dumpJsErr(this->context);
        }
        else [[likely]]
        {
            if (await)
            {/// WARNING: Do not use built-in `js_std_await()`, since it will triger the Promise Event Loop once again.
                jRes = _await(this->context, jRes); // Handle promise if needed
            }
            const auto cS = JS_ToCString(this->context, jRes);
            s = wrapStr(cS);
            JS_FreeCString(this->context, cS);
        }
        JS_FreeValue(this->context, jRes);
        JS_FreeValue(this->context, jParams);
    }
    else [[unlikely]]
    {
        mutex->unlock();
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Can not find JS func:{}", func);
    }

    return s;
}

JSValue NGenXX::JsBridge::newPromise(std::function<JSValue()> &&jf)
{
    if (!mutex) [[unlikely]]
    {
        return JS_UNDEFINED;
    }
    auto jPromise = _newPromise(this->context);
    if (jPromise == nullptr) [[unlikely]]
    {
        return JS_EXCEPTION;
    }
    
    std::thread([ctx = this->context, jPromise, jf = std::move(jf)] {
        if (!mutex) [[unlikely]]
        {
            return;
        }
        auto lock = std::lock_guard(*mutex);

        const auto jRet = jf();

        _callbackPromise(ctx, jPromise, jRet);
    }).detach();

    return jPromise->p;
}

JSValue NGenXX::JsBridge::newPromiseVoid(std::function<void()> &&f)
{
    return this->newPromise([f = std::move(f)]() {
        f();
        return JS_UNDEFINED;
    });
}

JSValue NGenXX::JsBridge::newPromiseBool(std::function<bool()> &&f)
{
    return this->newPromise([ctx = this->context, f = std::move(f)]{
        const auto ret = f();
        return JS_NewBool(ctx, ret);
    });
}

JSValue NGenXX::JsBridge::newPromiseInt32(std::function<int32_t()> &&f)
{
    return this->newPromise([ctx = this->context, f = std::move(f)]{
        const auto ret = f();
        return JS_NewInt32(ctx, ret);
    });
}

JSValue NGenXX::JsBridge::newPromiseInt64(std::function<int64_t()> &&f)
{
    return this->newPromise([ctx = this->context, f = std::move(f)]{
        const auto ret = f();
        return JS_NewInt64(ctx, ret);
    });
}

JSValue NGenXX::JsBridge::newPromiseFloat(std::function<double()> &&f)
{
    return this->newPromise([ctx = this->context, f = std::move(f)]{
        const auto ret = f();
        return JS_NewFloat64(ctx, ret);
    });
}

JSValue NGenXX::JsBridge::newPromiseString(std::function<const std::string()> &&f)
{
    return this->newPromise([ctx = this->context, f = std::move(f)]{
        const auto ret = f();
        return JS_NewString(ctx, ret.c_str() != nullptr ? ret.c_str() : "");
    });
}

NGenXX::JsBridge::~JsBridge()
{
    loop_stop(this->runtime);

    js_std_set_worker_new_context_func(nullptr);

    for (const auto &jv : this->jValueCache)
    {
        if (auto tag = JS_VALUE_GET_TAG(jv); tag == static_cast<decltype(tag)>(JS_TAG_MODULE)) [[unlikely]]
        {
            // Free a module will cause crash in QJS
            continue;
        }
        JS_FreeValue(this->context, jv);
    }
    JS_FreeValue(this->context, jGlobal);
    JS_FreeContext(this->context);
    this->context = nullptr;

    js_std_free_handlers(this->runtime);
    JS_FreeRuntime(this->runtime);
    this->runtime = nullptr;

    mutex.reset();

    _uv_loop_p = nullptr;
    _uv_loop_t = nullptr;
    _uv_timer_p = nullptr;
    _uv_timer_t = nullptr;
}
#endif
