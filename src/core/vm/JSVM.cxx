#if defined(USE_QJS)
#include "JSVM.hxx"

#include <fstream>
#include <sstream>
#include <utility>

#include <DynXX/CXX/Log.hxx>
#include "../util/MemUtil.hxx"

namespace
{
    constexpr auto IMPORT_STD_OS_JS = "import * as std from 'qjs:std';\n"
                                         "import * as os from 'qjs:os';\n"
                                         "globalThis.std = std;\n"
                                         "globalThis.os = os;\n";

    constexpr auto JSLoopTimeoutMicroSecs = 1 * 1000uz;
    constexpr auto JSCallRetryCount = 10uz;
    constexpr auto JSCallSleepMicroSecs = 100 * 1000uz;

    using enum DynXXLogLevelX;
    using namespace DynXX::Core::Util;
    using namespace DynXX::Core::Concurrent;

// JSVM dump error

    void printJsErr(JSContext *ctx, const JSValueConst val)
    {
        if (const auto str = JS_ToCString(ctx, val))
        {
            dynxxLogPrint(Error, str);
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

// JSVM Internal

    bool _loadScript(JSContext *ctx, const std::string &script, const std::string &name, const bool isModule)
    {
        const auto flags = isModule ? (JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY) : JS_EVAL_TYPE_GLOBAL;
        const auto jEvalRet = JS_Eval(ctx, script.c_str(), script.length(), name.c_str(), flags);
        if (JS_IsException(jEvalRet)) [[unlikely]]
        {
            dynxxLogPrint(Error, "JS_Eval failed ->");
            dumpJsErr(ctx);
            return false;
        }

        if (isModule)
        {
            if (JS_VALUE_GET_TAG(jEvalRet) != JS_TAG_MODULE) [[unlikely]]
            { // Check whether it's a JS Module or not，or QJS may crash
                dynxxLogPrint(Error, "JS try to load invalid module");
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

// JS Promise wrapper

    class JSPromise
    {
    private:
        std::weak_ptr<JSContext> ctx{};
        JSValue p{JS_UNDEFINED};
        JSValue f[2]{JS_UNDEFINED, JS_UNDEFINED};
    
    public:
        JSPromise() = delete;
        JSPromise(const JSPromise &) = delete;
        JSPromise(JSPromise &&) = delete;
        JSPromise &operator=(const JSPromise &) = delete;
        JSPromise &operator=(JSPromise &&) = delete;

        explicit JSPromise(const std::shared_ptr<JSContext> &ctx) : ctx(ctx) {
            const auto weakCtx = this->ctx.lock();
            if (!weakCtx) [[unlikely]] {
                return;
            }
            this->p = JS_NewPromiseCapability(weakCtx.get(), this->f);
            if (JS_IsException(this->p)) [[unlikely]] {
                dynxxLogPrint(Error, "JSVM_NewPromise failed ->");
                dumpJsErr(weakCtx.get());
                JS_FreeValue(weakCtx.get(), this->p);
                this->p = JS_UNDEFINED;
            }
        }

        void callbackJS(JSValue &ret) {
            const auto weakCtx = this->ctx.lock();
            if (!weakCtx) [[unlikely]] {
                return;
            }
            const auto jCallRet = JS_Call(weakCtx.get(), this->f[0], JS_UNDEFINED, 1, &ret);
            if (JS_IsException(jCallRet)) [[unlikely]] {
                dynxxLogPrint(Error, "JSVM_CallPromise failed ->");
                dumpJsErr(weakCtx.get());
            }
            JS_FreeValue(weakCtx.get(), ret);
            JS_FreeValue(weakCtx.get(), jCallRet);
        }

        JSValue& jsObj() {
            return this->p;
        }

        ~JSPromise()
        {
            const auto weakCtx = this->ctx.lock();
            if (!weakCtx) [[unlikely]] {
                return;
            }
            JS_FreeValue(weakCtx.get(), this->f[0]);
            JS_FreeValue(weakCtx.get(), this->f[1]);
            //JS_FreeValue(weakCtx.get(), this->p);
        }
    };

    std::unique_ptr<Mem::PtrCache<JSPromise>> promiseCache{nullptr};
}

// JSVM Internal

JSValue DynXX::Core::VM::JSVM::jAwait(const JSValue obj)
{
    if (JS_VALUE_GET_TAG(obj) != JS_TAG_OBJECT) [[unlikely]] {
        return obj;
    }

    auto ret = JS_UNDEFINED;
    auto done = false;

    do {
        /// Do not force to acquire the lock, to avoid blocking the JS event loop.
        const auto ctx = this->context.get();
        if (tryLockUntil(JSLoopTimeoutMicroSecs)) [[unlikely]]
        {
            const auto state = JS_PromiseState(ctx, obj);
            done = state != JS_PROMISE_PENDING;
            switch (state)
            {
            case JS_PROMISE_FULFILLED:
                ret = JS_PromiseResult(ctx, obj);
                JS_FreeValue(ctx, obj);
                break;
            case JS_PROMISE_REJECTED:
                ret = JS_Throw(ctx, JS_PromiseResult(ctx, obj));
                JS_FreeValue(ctx, obj);
                break;
            default:
                break;
            }
            unlock();
        }

        if (!done) {
            sleep();
        }
    } while (!done);

    return ret;
}

// JSValueHash and JSValueEqual

std::size_t DynXX::Core::VM::JSVM::JSValueHash::operator()(const JSValue &jv) const 
{
    return std::hash<void *>()(JS_VALUE_GET_PTR(jv));
}

bool DynXX::Core::VM::JSVM::JSValueEqual::operator()(const JSValue &left, const JSValue &right) const 
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

// JSVM API

DynXX::Core::VM::JSVM::JSVM() : runtime(JS_NewRuntime(), JS_FreeRuntime)
{
    js_std_init_handlers(this->runtime.get());
    JS_SetModuleLoaderFunc(this->runtime.get(), nullptr, js_module_loader, nullptr);
    js_std_set_worker_new_context_func(_newContext);

    this->context = std::shared_ptr<JSContext>(_newContext(this->runtime.get()), JS_FreeContext);
    this->jGlobal = JS_GetGlobalObject(this->context.get());// Can not free here, will be called in future

    promiseCache = std::make_unique<Mem::PtrCache<JSPromise>>();

    this->timerLooperTask = std::make_unique<TimerTask>([this]() {
        if (tryLockUntil(JSLoopTimeoutMicroSecs))
        {
            js_std_loop_timer(context.get());
            unlock();
        }
    }, JSLoopTimeoutMicroSecs);

    this->promiseLooperTask = std::make_unique<TimerTask>([this]() {
        if (tryLockUntil(JSLoopTimeoutMicroSecs))
        {
            js_std_loop_promise(context.get());
            unlock();
        }
    }, JSLoopTimeoutMicroSecs);
}

bool DynXX::Core::VM::JSVM::bindFunc(const std::string &funcJ, JSCFunction *funcC)
{
    auto res = true;
    const auto ctx = this->context.get();
    const auto jFunc = JS_NewCFunction(ctx, funcC, funcJ.c_str(), 1);
    if (JS_IsException(jFunc)) [[unlikely]]
    {
        dynxxLogPrint(Error, "JS_NewCFunction failed ->");
        dumpJsErr(ctx);
        res = false;
    }
    else [[likely]]
    {
        if (!JS_DefinePropertyValueStr(ctx, this->jGlobal, funcJ.c_str(), jFunc, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE)) [[unlikely]]
        {
            dynxxLogPrint(Error, "JS_DefinePropertyValueStr failed ->");
            dumpJsErr(ctx);
            res = false;
        }
    }

    this->jValueCache.insert(jFunc); // Can not free here, will be called in future

    return res;
}

bool DynXX::Core::VM::JSVM::loadFile(const std::string &file, bool isModule)
{
    try {
        std::ifstream ifs(file.c_str());
        ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        if (!ifs.is_open()) [[unlikely]]
        {
            dynxxLogPrintF(Error, "JSVM::loadFile {} open failed", file.c_str());
            return false;
        }
        std::ostringstream ss;
        ss << ifs.rdbuf();
        return this->loadScript(ss.str(), file, isModule);
    }
    catch (const std::ios_base::failure& e) {
        dynxxLogPrintF(Error, "JSVM::loadFile {} IO error: {}", file.c_str(), e.what());
        return false;
    }
    catch (const std::exception& e) {
        dynxxLogPrintF(Error, "JSVM::loadFile {} error: {}", file.c_str(), e.what());
        return false;
    }
}

bool DynXX::Core::VM::JSVM::loadScript(const std::string &script, const std::string &name, bool isModule) {
    auto lock = std::scoped_lock(this->vmMutex);
    return _loadScript(this->context.get(), script, name, isModule);
}

bool DynXX::Core::VM::JSVM::loadBinary(const Bytes &bytes, bool isModule) {
    auto lock = std::scoped_lock(this->vmMutex);
    return js_std_eval_binary(this->context.get(), bytes.data(), bytes.size(), 0);
}

/// WARNING: Nested call between native and JS requires a reenterable `recursive_mutex` here!
std::optional<std::string> DynXX::Core::VM::JSVM::callFunc(std::string_view func, std::string_view params, bool await) {
    if (!lockAutoRetry(JSCallRetryCount, JSCallSleepMicroSecs)) [[unlikely]]
    {
        dynxxLogPrint(Error, "JSVM::callFunc failed to lock");
        return std::nullopt;
    }
    std::string s;
    auto success = false;

    const auto ctx = this->context.get();

    if (const auto jFunc = JS_GetPropertyStr(ctx, this->jGlobal, func.data()); JS_IsFunction(ctx, jFunc)) [[likely]]
    {
        const auto jParams = JS_NewString(ctx, params.data());
        JSValue argv[] = {jParams};

        auto jRes = JS_Call(ctx, jFunc, this->jGlobal, sizeof(argv), argv);

        if (JS_IsException(jRes)) [[unlikely]]
        {
            dynxxLogPrint(Error, "JS_Call failed ->");
            dumpJsErr(ctx);
        }
        else [[likely]]
        {
            success = true;
            if (await)
            {/// WARNING: Do not use built-in `js_std_await()`, since it will triger the Promise Event Loop once again.
                jRes = this->jAwait(jRes); // Handle promise if needed
            }
            const auto cS = JS_ToCString(ctx, jRes);
            s = std::move(makeStr(cS));
            JS_FreeCString(ctx, cS);
        }
        JS_FreeValue(ctx, jRes);
        JS_FreeValue(ctx, jParams);
    }
    else [[unlikely]]
    {
        dynxxLogPrintF(Error, "Can not find JS func:{}", func);
    }

    this->unlock();

    return success? std::make_optional(s): std::nullopt;
}

JSValue DynXX::Core::VM::JSVM::newPromise(std::function<JSValue()> &&jf)
{
    if (!this->lockAutoRetry(JSCallRetryCount, JSCallSleepMicroSecs)) [[unlikely]]
    {
        dynxxLogPrint(Error, "JSVM::newPromise create failed to lock");
        return JS_UNDEFINED;
    }
    const auto handle = promiseCache->add(std::make_unique<JSPromise>(this->context));
    auto result = promiseCache->get(handle)->jsObj();
    this->unlock();

    this->executor >> [handle, cbk = std::move(jf), this] {
        auto ret = cbk();

        if (!this->lockAutoRetry(JSCallRetryCount, JSCallSleepMicroSecs)) [[unlikely]]
        {
            dynxxLogPrint(Error, "JSVM::newPromise callback failed to lock");
            promiseCache->remove(handle);
            return;
        }
        promiseCache->get(handle)->callbackJS(ret);
        promiseCache->remove(handle);

        this->unlock();
    };

    return result;
}

JSValue DynXX::Core::VM::JSVM::newPromiseVoid(std::function<void()> &&vf)
{
    return this->newPromise([cbk = std::move(vf)]() {
        cbk();
        return JS_UNDEFINED;
    });
}

JSValue DynXX::Core::VM::JSVM::newPromiseBool(std::function<bool()> &&bf)
{
    return this->newPromise([&ctx = this->context, cbk = std::move(bf)]{
        const auto ret = cbk();
        return JS_NewBool(ctx.get(), ret);
    });
}

JSValue DynXX::Core::VM::JSVM::newPromiseInt32(std::function<int32_t()> &&i32f)
{
    return this->newPromise([&ctx = this->context, cbk = std::move(i32f)]{
        const auto ret = cbk();
        return JS_NewInt32(ctx.get(), ret);
    });
}

JSValue DynXX::Core::VM::JSVM::newPromiseInt64(std::function<int64_t()> &&i64f)
{
    return this->newPromise([&ctx = this->context, cbk = std::move(i64f)]{
        const auto ret = cbk();
        return JS_NewInt64(ctx.get(), ret);
    });
}

JSValue DynXX::Core::VM::JSVM::newPromiseFloat(std::function<double()> &&ff)
{
    return this->newPromise([&ctx = this->context, cbk = std::move(ff)]{
        const auto ret = cbk();
        return JS_NewFloat64(ctx.get(), ret);
    });
}

JSValue DynXX::Core::VM::JSVM::newPromiseString(std::function<const std::string()> &&sf)
{
    return this->newPromise([&ctx = this->context, cbk = std::move(sf)]{
        const auto ret = cbk();
        return JS_NewString(ctx.get(), ret.c_str() != nullptr ? ret.c_str() : "");
    });
}

DynXX::Core::VM::JSVM::~JSVM()
{
    promiseCache.reset();
    this->timerLooperTask.reset();
    this->promiseLooperTask.reset();

    this->active = false;
    js_std_loop_cancel(this->runtime.get());

    js_std_set_worker_new_context_func(nullptr);

    for (const auto &jv : this->jValueCache)
    {
        if (auto tag = JS_VALUE_GET_TAG(jv); tag == static_cast<decltype(tag)>(JS_TAG_MODULE)) [[unlikely]]
        {
            // Free a module will cause crash in QJS
            continue;
        }
        JS_FreeValue(this->context.get(), jv);
    }
    this->jValueCache.clear();

    JS_FreeValue(this->context.get(), jGlobal);
    this->context.reset();

    js_std_free_handlers(this->runtime.get());
    this->runtime.reset();
}
#endif
