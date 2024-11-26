#include "JsBridge.hxx"
#include "../../include/NGenXXLog.hxx"
#include "../../external/libuv/include/uv.h"

#include <fstream>
#include <sstream>
#include <streambuf>
#include <utility>
#include <mutex>

constexpr const char *IMPORT_STD_OS_JS = "import * as std from 'qjs:std';\n"
                                         "import * as os from 'qjs:os';\n"
                                         "globalThis.std = std;\n"
                                         "globalThis.os = os;\n";

typedef struct JS_Promise
{
    JSValue p;
    JSValue f[2];
} JS_Promise;

static uv_loop_t *_ngenxx_js_uv_loop_p = nullptr;
static uv_loop_t *_ngenxx_js_uv_loop_t = nullptr;
static uv_timer_t *_ngenxx_js_uv_timer_p = nullptr;
static uv_timer_t *_ngenxx_js_uv_timer_t = nullptr;
static std::mutex *_ngenxx_js_mutex = nullptr;

static void _ngenxx_js_uv_timer_cb_p(uv_timer_t *timer)
{
    JSContext *ctx = reinterpret_cast<JSContext *>(timer->data);
    /// Do not force to acquire the lock, to avoid blocking the JS event loop.
    if (_ngenxx_js_mutex->try_lock())
    {
        js_std_loop_promise(ctx);
        _ngenxx_js_mutex->unlock();
    }
}

static void _ngenxx_js_uv_timer_cb_t(uv_timer_t *timer)
{
    JSContext *ctx = reinterpret_cast<JSContext *>(timer->data);
    /// Do not force to acquire the lock, to avoid blocking the JS event loop.
    if (_ngenxx_js_mutex->try_lock())
    {
        js_std_loop_timer(ctx);
        _ngenxx_js_mutex->unlock();
    }
}

static void _ngenxx_js_uv_loop_start(JSContext *ctx, uv_loop_t *uv_loop, uv_timer_t *uv_timer, uv_timer_cb cb)
{
    if (uv_loop == nullptr)
    {
        uv_loop = reinterpret_cast<uv_loop_t *>(malloc(sizeof(uv_loop_t)));
        uv_loop_init(uv_loop);
    }
    else
    {
        if (uv_loop_alive(uv_loop))
        {
            return;
        }
    }

    if (uv_timer == nullptr)
    {
        uv_timer = reinterpret_cast<uv_timer_t *>(malloc(sizeof(uv_timer_t)));
        uv_timer_init(uv_loop, uv_timer);
        uv_timer->data = ctx;
        uv_timer_start(uv_timer, cb, 1, 1);
    }
    else
    {
        uv_timer_stop(uv_timer);
        uv_timer_again(uv_timer);
    }

    uv_run(uv_loop, UV_RUN_DEFAULT);
}

static void _ngenxx_js_uv_loop_stop(uv_loop_t *uv_loop, uv_timer_t *uv_timer)
{
    if (uv_loop == nullptr || uv_timer == nullptr || !uv_loop_alive(uv_loop))
    {
        return;
    }

    uv_timer_stop(uv_timer);
    free(uv_timer);
    uv_timer = nullptr;

    uv_stop(uv_loop);
    uv_loop_close(uv_loop);
    free(uv_loop);
    uv_loop = nullptr;
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
    const char *str = JS_ToCString(ctx, val);
    if (str)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, str);
        JS_FreeCString(ctx, str);
    }
}

static void _ngenxx_js_dump_err(JSContext *ctx)
{
    JSValue exception_val = JS_GetException(ctx);

    _ngenxx_js_print_err(ctx, exception_val);
    if (JS_IsError(ctx, exception_val))
    {
        JSValue val = JS_GetPropertyStr(ctx, exception_val, "stack");
        if (!JS_IsUndefined(val))
        {
            _ngenxx_js_print_err(ctx, val);
        }
        JS_FreeValue(ctx, val);
    }

    JS_FreeValue(ctx, exception_val);
}

bool _ngenxx_js_loadScript(JSContext *ctx, const std::string &script, const std::string &name, const bool isModule)
{
    bool res = true;
    int flags = isModule ? (JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY) : JS_EVAL_TYPE_GLOBAL;
    JSValue jEvalRet = JS_Eval(ctx, script.c_str(), script.length(), name.c_str(), flags);
    if (JS_IsException(jEvalRet))
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
        JSValue jEvalFuncRet = JS_EvalFunction(ctx, jEvalRet);
        JS_FreeValue(ctx, jEvalFuncRet);
        // this->jValues.push_back(jEvalRet);//Can not free here, or QJS may crash
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
    JSContext *ctx;
    ctx = JS_NewContext(rt);
    if (!ctx)
    {
        return NULL;
    }

    js_std_add_helpers(ctx, 0, NULL);
    js_init_module_std(ctx, "qjs:std");
    js_init_module_os(ctx, "qjs:os");

    _ngenxx_js_loadScript(ctx, IMPORT_STD_OS_JS, "import-std-os.js", true);

    return ctx;
}

NGenXX::JsBridge::JsBridge()
{
    _ngenxx_js_mutex = new std::mutex();

    this->runtime = JS_NewRuntime();
    js_std_init_handlers(this->runtime);
    JS_SetModuleLoaderFunc(this->runtime, NULL, js_module_loader, NULL);
    js_std_set_worker_new_context_func(_ngenxx_js_newContext);

    this->context = _ngenxx_js_newContext(this->runtime);
    this->jValues.push_back(JS_GetGlobalObject(this->context));

    this->loopThreadP = std::thread([&ctx = this->context]() 
    { 
        _ngenxx_js_loop_startP(ctx); 
    });
    this->loopThreadT = std::thread([&ctx = this->context]() 
    { 
        _ngenxx_js_loop_startT(ctx); 
    });
}

bool NGenXX::JsBridge::bindFunc(const std::string &funcJ, JSCFunction *funcC)
{
    bool res = true;
    JSValue jFunc = JS_NewCFunction(this->context, funcC, funcJ.c_str(), 1);
    if (JS_IsException(jFunc))
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_NewCFunction failed ->");
        _ngenxx_js_dump_err(this->context);
        res = false;
    }
    else
    {
        if (!JS_DefinePropertyValueStr(this->context, this->jValues[0], funcJ.c_str(), jFunc, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE))
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_DefinePropertyValueStr failed ->");
            _ngenxx_js_dump_err(this->context);
            res = false;
        }
    }

    this->jValues.push_back(jFunc); // Can not free here, will be called in future

    return res;
}

bool NGenXX::JsBridge::loadFile(const std::string &file, const bool isModule)
{
    std::ifstream ifs(file.c_str());
    std::stringstream ss;
    ss << ifs.rdbuf();
    return this->loadScript(ss.str(), file, isModule);
}

bool NGenXX::JsBridge::loadScript(const std::string &script, const std::string &name, const bool isModule)
{
    const std::lock_guard<std::mutex> lock(*_ngenxx_js_mutex);
    return _ngenxx_js_loadScript(this->context, script, name, isModule);
}

bool NGenXX::JsBridge::loadBinary(Bytes bytes, const bool isModule)
{
    const std::lock_guard<std::mutex> lock(*_ngenxx_js_mutex);
    return js_std_eval_binary(this->context, bytes.data(), bytes.size(), 0);
}

static inline const std::string _ngenxx_js_jstr2stdstr(JSContext *ctx, JSValue jstr)
{
    auto c = JS_ToCString(ctx, jstr);
    auto s = std::string(c ?: "");
    JS_FreeCString(ctx, c);
    return s;
}

JSValue _ngenxx_js_await(JSContext *ctx, JSValue obj)
{
    JSValue ret;
    for (;;)
    {
        /// Do not force to acquire the lock, to avoid blocking the JS event loop.
        if (!_ngenxx_js_mutex->try_lock())
        {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            continue;
        }
        int state = JS_PromiseState(ctx, obj);
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
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        else
        {
            /// Not a Promise: release the lock, return the result immediately.
            _ngenxx_js_mutex->unlock();
            ret = obj;
            break;
        }
    }
    return ret;
}

std::string NGenXX::JsBridge::callFunc(const std::string &func, const std::string &params)
{
    const std::lock_guard<std::mutex> lock(*_ngenxx_js_mutex);
    std::string s;

    JSValue jFunc = JS_GetPropertyStr(this->context, this->jValues[0], func.c_str());
    if (JS_IsFunction(this->context, jFunc))
    {
        JSValue jParams = JS_NewString(this->context, params.c_str());
        JSValue argv[] = {jParams};

        JSValue jRes = JS_Call(this->context, jFunc, this->jValues[0], sizeof(argv), argv);

        /// Release the lock imediately, to avoid blocking the JS event loop.
        _ngenxx_js_mutex->unlock();

        if (JS_IsException(jRes))
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_Call failed ->");
            _ngenxx_js_dump_err(this->context);
        }
        else
        {
            jRes = _ngenxx_js_await(this->context, jRes); // Handle promise if needed
            s = std::move(_ngenxx_js_jstr2stdstr(this->context, jRes));
        }
        JS_FreeValue(this->context, jRes);
        JS_FreeValue(this->context, jParams);
    }
    else
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, ("Can not find JS func:" + func).c_str());
    }

    return s;
}

JS_Promise* _ngenxx_js_promise_new(JSContext *ctx)
{
    auto jPromise = new JS_Promise();
    JSValue funcs[2];
    jPromise->p = JS_NewPromiseCapability(ctx, jPromise->f);
    if (JS_IsException(jPromise->p))
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_NewPromise failed ->");
        _ngenxx_js_dump_err(ctx);
        JS_FreeValue(ctx, jPromise->p);
        return nullptr;
    }
    return jPromise;
}

void _ngenxx_js_promise_callback(JSContext *ctx, JS_Promise* jPromise, JSValue jRet)
{
    JSValue jCallRet = JS_Call(ctx, jPromise->f[0], JS_UNDEFINED, 1, &jRet);
    if (JS_IsException(jCallRet))
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_CallPromise failed ->");
        _ngenxx_js_dump_err(ctx);
    }

    JS_FreeValue(ctx, jCallRet);
    JS_FreeValue(ctx, jRet);
    JS_FreeValue(ctx, jPromise->f[0]);
    JS_FreeValue(ctx, jPromise->f[1]);
    //JS_FreeValue(ctx, jPromise->p);
    delete (jPromise);
}

JSValue NGenXX::JsBridge::newPromiseVoid(std::function<void()> f)
{
    auto jPromise = _ngenxx_js_promise_new(this->context);
    if (jPromise == nullptr)
    {
        return JS_EXCEPTION;
    }
    
    this->promiseThreadV.emplace_back([&ctx = this->context, jPromise = jPromise, cb = f]() {
        cb();
        
        const std::lock_guard<std::mutex> lock(*_ngenxx_js_mutex);

        JSValue jRet = JS_UNDEFINED;

        _ngenxx_js_promise_callback(ctx, jPromise, jRet);
    });

    return jPromise->p;
}

JSValue NGenXX::JsBridge::newPromiseBool(std::function<const bool()> f)
{
    auto jPromise = _ngenxx_js_promise_new(this->context);
    if (jPromise == nullptr)
    {
        return JS_EXCEPTION;
    }
    
    this->promiseThreadV.emplace_back([&ctx = this->context, jPromise = jPromise, cb = f]() {
        auto ret = cb();
        
        const std::lock_guard<std::mutex> lock(*_ngenxx_js_mutex);

        JSValue jRet = JS_NewBool(ctx, ret);

        _ngenxx_js_promise_callback(ctx, jPromise, jRet);
    });

    return jPromise->p;
}

JSValue NGenXX::JsBridge::newPromiseInt32(std::function<const int()> f)
{
    auto jPromise = _ngenxx_js_promise_new(this->context);
    if (jPromise == nullptr)
    {
        return JS_EXCEPTION;
    }
    
    this->promiseThreadV.emplace_back([&ctx = this->context, jPromise = jPromise, cb = f]() {
        auto ret = cb();
        
        const std::lock_guard<std::mutex> lock(*_ngenxx_js_mutex);

        JSValue jRet = JS_NewInt32(ctx, ret);

        _ngenxx_js_promise_callback(ctx, jPromise, jRet);
    });

    return jPromise->p;
}

JSValue NGenXX::JsBridge::newPromiseInt64(std::function<const long long()> f)
{
    auto jPromise = _ngenxx_js_promise_new(this->context);
    if (jPromise == nullptr)
    {
        return JS_EXCEPTION;
    }
    
    this->promiseThreadV.emplace_back([&ctx = this->context, jPromise = jPromise, cb = f]() {
        auto ret = cb();
        
        const std::lock_guard<std::mutex> lock(*_ngenxx_js_mutex);

        JSValue jRet = JS_NewInt64(ctx, ret);

        _ngenxx_js_promise_callback(ctx, jPromise, jRet);
    });

    return jPromise->p;
}

JSValue NGenXX::JsBridge::newPromiseFloat(std::function<const double()> f)
{
    auto jPromise = _ngenxx_js_promise_new(this->context);
    if (jPromise == nullptr)
    {
        return JS_EXCEPTION;
    }
    
    this->promiseThreadV.emplace_back([&ctx = this->context, jPromise = jPromise, cb = f]() {
        auto ret = cb();
        
        const std::lock_guard<std::mutex> lock(*_ngenxx_js_mutex);

        JSValue jRet = JS_NewFloat64(ctx, ret);

        _ngenxx_js_promise_callback(ctx, jPromise, jRet);
    });

    return jPromise->p;
}

JSValue NGenXX::JsBridge::newPromiseString(std::function<const std::string()> f)
{
    auto jPromise = _ngenxx_js_promise_new(this->context);
    if (jPromise == nullptr)
    {
        return JS_EXCEPTION;
    }
    
    this->promiseThreadV.emplace_back([&ctx = this->context, jPromise = jPromise, cb = f]() {
        auto ret = cb();

        const std::lock_guard<std::mutex> lock(*_ngenxx_js_mutex);

        JSValue jRet = JS_NewString(ctx, ret.c_str() ? : "");

        _ngenxx_js_promise_callback(ctx, jPromise, jRet);
    });

    return jPromise->p;
}

NGenXX::JsBridge::~JsBridge()
{
    _ngenxx_js_loop_stop(this->runtime);
    if (this->loopThreadP.joinable())
    {
        this->loopThreadP.join();
    }
    if (this->loopThreadT.joinable())
    {
        this->loopThreadT.join();
    }
    for (auto &thread : this->promiseThreadV)
    {
        thread.join();
    }

    js_std_set_worker_new_context_func(NULL);

    for (auto &jv : this->jValues)
    {
        uint32_t tag = JS_VALUE_GET_TAG(jv);
        if (tag != JS_TAG_MODULE)
        {
            JS_FreeValue(this->context, jv);
        }
    }
    JS_FreeContext(this->context);

    js_std_free_handlers(this->runtime);
    JS_FreeRuntime(this->runtime);

    delete _ngenxx_js_mutex;
}
