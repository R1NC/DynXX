#if defined(USE_QJS)
#include "JSVM.hxx"

#include <fstream>
#include <sstream>
#include <utility>
#include <array>

#include "quickjs.h"

#include <DynXX/CXX/Log.hxx>
#include "../util/MemUtil.hxx"
#include "../util/TimeUtil.hxx"

namespace
{
    constexpr auto IMPORT_STD_OS_JS = "import * as std from 'qjs:std';\n"
                                         "import * as os from 'qjs:os';\n"
                                         "globalThis.std = std;\n"
                                         "globalThis.os = os;\n";

    constexpr auto JSLoopTimeoutMicroSecs = 1UZ * 1000UZ;
    constexpr auto JSCallRetryCount = 10UZ;
    constexpr auto JSCallSleepMicroSecs = 100UZ * 1000UZ;
    constexpr auto JSAwaitMaxTimeMicroSecs = 15UZ * 1000UZ * 1000UZ;

    using enum DynXXLogLevelX;
    using namespace DynXX::Core::Util;
    using namespace DynXX::Core::Concurrent;

// JSVM dump error

    void printJsErr(JSContext *ctx, const JSValueConst val)
    {
        if (const auto str = JS_ToCString(ctx, val); str != nullptr)
        {
            dynxxLogPrint(Error, str);
            JS_FreeCString(ctx, str);
        }
    }

    void dumpJsErr(JSContext *ctx)
    {
        const auto exception_val = JS_GetException(ctx);

        printJsErr(ctx, exception_val);
        if (JS_IsError(ctx, exception_val) == 1)
        {
            const auto val = JS_GetPropertyStr(ctx, exception_val, "stack");
            if (JS_IsUndefined(val) == 0)
            {
                printJsErr(ctx, val);
            }
            JS_FreeValue(ctx, val);
        }

        JS_FreeValue(ctx, exception_val);
    }

// JSVM Internal

    [[nodiscard]] bool _loadScript(JSContext *ctx, std::string_view script, std::string_view name, bool isModule)
    {
        if (ctx == nullptr || script.empty() || name.empty()) [[unlikely]] {
            return false;
        }
        const auto flags = isModule ? (JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY) : JS_EVAL_TYPE_GLOBAL;
        const auto nameS = std::string{name.data(), name.size()};
        const auto jEvalRet = JS_Eval(ctx, script.data(), script.size(), nameS.c_str(), flags);
        if (JS_IsException(jEvalRet) == 1) [[unlikely]]
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
            js_module_set_import_meta(ctx, jEvalRet, 0, 1);
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
        if (ctx == nullptr) [[unlikely]]
        {
            return nullptr;
        }

        js_std_add_helpers(ctx, 0, nullptr);
        js_init_module_std(ctx, "qjs:std");
        js_init_module_os(ctx, "qjs:os");
        return ctx;
    }

// JS Promise wrapper

    class JSPromise
    {
    private:
        std::weak_ptr<JSContext> ctx;
        JSValue p{JS_UNDEFINED};
        std::array<JSValue, 2> f{JS_UNDEFINED, JS_UNDEFINED};
    
    public:
        JSPromise() = delete;
        JSPromise(const JSPromise &) = delete;
        JSPromise(JSPromise &&) = delete;
        JSPromise &operator=(const JSPromise &) = delete;
        JSPromise &operator=(JSPromise &&) = delete;

        explicit JSPromise(const std::shared_ptr<JSContext> &ctx) : ctx(ctx) {
            const auto weakCtx = this->ctx.lock();
            if (weakCtx == nullptr) [[unlikely]] {
                return;
            }
            this->p = JS_NewPromiseCapability(weakCtx.get(), this->f.data());
            if (JS_IsException(this->p) == 1) [[unlikely]] {
                dynxxLogPrint(Error, "JSVM_NewPromise failed ->");
                dumpJsErr(weakCtx.get());
                JS_FreeValue(weakCtx.get(), this->p);
                this->p = JS_UNDEFINED;
            }
        }

        void callbackJS(JSValue &ret) {
            const auto weakCtx = this->ctx.lock();
            if (weakCtx == nullptr) [[unlikely]] {
                return;
            }
            const auto jCallRet = JS_Call(weakCtx.get(), this->f[0], JS_UNDEFINED, 1, &ret);
            if (JS_IsException(jCallRet) == 1) [[unlikely]] {
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
            if (weakCtx == nullptr) [[unlikely]] {
                return;
            }
            JS_FreeValue(weakCtx.get(), this->f[0]);
            JS_FreeValue(weakCtx.get(), this->f[1]);
            //JS_FreeValue(weakCtx.get(), this->p);
        }
    };

    Mem::PtrCache<JSPromise> &ensurePromiseCache() {
        static auto *cache = new Mem::PtrCache<JSPromise>();
        return *cache;
    }
}

namespace DynXX::Core::VM {

// JSVM Internal

JSValue JSVM::jAwait(JSValue obj)
{
    if (JS_VALUE_GET_TAG(obj) != JS_TAG_OBJECT) [[unlikely]] {
        return obj;
    }

    const auto beginTime = Time::nowInMicroSecs();

    auto ret = JS_UNDEFINED;
    auto needWait = true;

    do {
        /// Do not force to acquire the lock, to avoid blocking the JS event loop.
        const auto ctx = this->context.get();
        if (tryLockUntil(JSLoopTimeoutMicroSecs)) [[unlikely]]
        {
            const auto state = JS_PromiseState(ctx, obj);
            needWait = state == JS_PROMISE_PENDING;
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

        if (needWait && Time::nowInMicroSecs() - beginTime > JSAwaitMaxTimeMicroSecs) [[unlikely]] {
            dynxxLogPrint(Error, "JSVM await timeout");
            needWait = false;
        }

        if (needWait) {
            sleep();
        }
    } while (needWait);

    return ret;
}

// JSValueHash and JSValueEqual

std::size_t JSVM::JSValueHash::operator()(const JSValue &jv) const noexcept
{
    return std::hash<void *>()(JS_VALUE_GET_PTR(jv));
}

bool JSVM::JSValueEqual::operator()(const JSValue &left, const JSValue &right) const noexcept
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

JSVM::JSVM() : context{std::shared_ptr<JSContext>(_newContext(this->runtime.get()), JSContextDeleter{})}
{
    if (this->runtime == nullptr || this->context == nullptr) [[unlikely]]
    {
        return;
    }
    js_std_init_handlers(this->runtime.get());
    JS_SetModuleLoaderFunc(this->runtime.get(), nullptr, js_module_loader, nullptr);
    js_std_set_worker_new_context_func(_newContext);

    if (!_loadScript(this->context.get(), IMPORT_STD_OS_JS, "import-std-os.js", true)) [[unlikely]]
    {
        return;
    }

    this->jGlobal = JS_GetGlobalObject(this->context.get());// Can not free here, will be called in future
    if (JS_IsException(this->jGlobal) == 1) [[unlikely]]
    {
        dumpJsErr(this->context.get());
        JS_FreeValue(this->context.get(), this->jGlobal);
        this->jGlobal = JS_UNDEFINED;
        return;
    }

}

bool JSVM::bindFunc(std::string_view funcJ, JSCFunction *funcC)
{
    if (this->context == nullptr || JS_IsUndefined(this->jGlobal) == 1) [[unlikely]]
    {
        return false;
    }
    const auto funcS = std::string{funcJ.data(), funcJ.size()}; 
    auto res = true;
    const auto ctx = this->context.get();
    const auto jFunc = JS_NewCFunction(ctx, funcC, funcS.c_str(), 1);
    if (JS_IsException(jFunc) == 1) [[unlikely]]
    {
        dynxxLogPrint(Error, "JS_NewCFunction failed ->");
        dumpJsErr(ctx);
        res = false;
    }
    else [[likely]]
    {
        if (JS_DefinePropertyValueStr(ctx, this->jGlobal, funcS.c_str(), jFunc, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE) == 0) [[unlikely]]
        {
            dynxxLogPrint(Error, "JS_DefinePropertyValueStr failed ->");
            dumpJsErr(ctx);
            res = false;
        }
    }

    return res;
}

void JSVM::beforeLoad()
{
    this->timerLooperTask = std::make_unique<TimerTask>([weakSelf = std::weak_ptr<BaseVM>(this->shared_from_this())]() {
        const auto self = std::dynamic_pointer_cast<JSVM>(weakSelf.lock());
        if (self == nullptr) [[unlikely]] {
            return;
        }
        if (self->tryLockUntil(JSLoopTimeoutMicroSecs))
        {
            js_std_loop_timer(self->context.get());
            self->unlock();
        }
    }, JSLoopTimeoutMicroSecs);

    this->promiseLooperTask = std::make_unique<TimerTask>([weakSelf = std::weak_ptr<BaseVM>(this->shared_from_this())]() {
        const auto self = std::dynamic_pointer_cast<JSVM>(weakSelf.lock());
        if (self == nullptr) [[unlikely]] {
            return;
        }
        if (self->tryLockUntil(JSLoopTimeoutMicroSecs))
        {
            js_std_loop_promise(self->context.get());
            self->unlock();
        }
    }, JSLoopTimeoutMicroSecs);
}

bool JSVM::loadFile(std::string_view file, bool isModule)
{
    const auto fileS = std::string{file.data(), file.size()};
    {
        const auto lock = std::scoped_lock(this->vmMutex);
#if defined(__cpp_lib_generic_unordered_lookup)
        if (this->loadedScriptNames.contains(file)) [[likely]]
#else
        if (this->loadedScriptNames.contains(fileS)) [[likely]]
#endif
        {
            return true;
        }
    }
    try {
        std::ifstream ifs(fileS.c_str());
        ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        if (!ifs.is_open()) [[unlikely]]
        {
            dynxxLogPrintF(Error, "JSVM::loadFile {} open failed", fileS.c_str());
            return false;
        }
        std::ostringstream ss;
        ss << ifs.rdbuf();
        return this->loadScript(ss.str(), file, isModule);
    }
    catch (const std::ios_base::failure& e) {
        dynxxLogPrintF(Error, "JSVM::loadFile {} IO error: {}", fileS.c_str(), e.what());
        return false;
    }
    catch (const std::exception& e) {
        dynxxLogPrintF(Error, "JSVM::loadFile {} error: {}", fileS.c_str(), e.what());
        return false;
    }
}

bool JSVM::loadScript(std::string_view script, std::string_view name, bool isModule) {
    if (this->context == nullptr) [[unlikely]]
    {
        return false;
    }
    const auto lock = std::scoped_lock(this->vmMutex);
#if defined(__cpp_lib_generic_unordered_lookup)
    if (this->loadedScriptNames.contains(name)) [[likely]]
#else
    const auto nameS = std::string{name.data(), name.size()};
    if (this->loadedScriptNames.contains(nameS)) [[likely]]
#endif
    {
        return true;
    }
    const auto loaded = _loadScript(this->context.get(), script, name, isModule);
    if (loaded) [[likely]]
    {
#if defined(__cpp_lib_generic_unordered_lookup)
        this->loadedScriptNames.emplace(name);
#else
        this->loadedScriptNames.insert(nameS);
#endif
    }
    return loaded;
}

bool JSVM::loadBinary(BytesView bytes, [[maybe_unused]] bool isModule) {
    if (this->context == nullptr) [[unlikely]]
    {
        return false;
    }
    const auto lock = std::scoped_lock(this->vmMutex);
    return js_std_eval_binary(this->context.get(), bytes.data(), bytes.size(), 0);
}

/// WARNING: Nested call between native and JS requires a reenterable `recursive_mutex` here!
std::optional<std::string> JSVM::callFunc(std::string_view func, std::string_view params, bool await) {
    if (this->context == nullptr || JS_IsUndefined(this->jGlobal) == 1) [[unlikely]]
    {
        return std::nullopt;
    }
    if (!lockAutoRetry(JSCallRetryCount, JSCallSleepMicroSecs)) [[unlikely]]
    {
        dynxxLogPrint(Error, "JSVM::callFunc failed to lock");
        return std::nullopt;
    }
    std::string s;
    auto success = false;

    const auto ctx = this->context.get();
    const auto funcS = std::string{func.data(), func.size()};
    const auto paramsS = std::string{params.data(), params.size()};
    if (const auto jFunc = JS_GetPropertyStr(ctx, this->jGlobal, funcS.c_str()); JS_IsFunction(ctx, jFunc)) [[likely]]
    {
        const auto jParams = JS_NewString(ctx, paramsS.c_str());
        std::array<JSValue, 1> argv{jParams};

        auto jRes = JS_Call(ctx, jFunc, this->jGlobal, argv.size(), argv.data());

        if (JS_IsException(jRes) == 1) [[unlikely]]
        {
            dynxxLogPrint(Error, "JS_Call failed ->");
            dumpJsErr(ctx);
        }
        else [[likely]]
        {
            success = true;
            if (await)
            {/// WARNING: Do not use built-in `js_std_await()`, since it will triger the Promise Event Loop once again.
                this->beforeLoad();
                jRes = this->jAwait(jRes); // Handle promise if needed
            }
            const auto cS = JS_ToCString(ctx, jRes);
            s = makeStr(cS);
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

JSValue JSVM::newPromise(std::function<JSValue(JSContext *)> &&jf)
{
    if (!this->lockAutoRetry(JSCallRetryCount, JSCallSleepMicroSecs)) [[unlikely]]
    {
        dynxxLogPrint(Error, "JSVM::newPromise create failed to lock");
        return JS_UNDEFINED;
    }
    auto *promiseCache = &ensurePromiseCache();
    const auto handle = promiseCache->add(std::make_unique<JSPromise>(this->context));
    const auto promise = promiseCache->get(handle);
    if (promise == nullptr) [[unlikely]]
    {
        this->unlock();
        return JS_UNDEFINED;
    }
    const auto result = promise->jsObj();
    this->unlock();

    this->submitTask([weakSelf = std::weak_ptr<BaseVM>(this->shared_from_this()), promiseCache, handle, cbk = std::move(jf)] {
        const auto self = std::dynamic_pointer_cast<JSVM>(weakSelf.lock());
        if (self == nullptr) [[unlikely]]
        {
            promiseCache->remove(handle);
            return;
        }
        if (!self->lockAutoRetry(JSCallRetryCount, JSCallSleepMicroSecs)) [[unlikely]]
        {
            dynxxLogPrint(Error, "JSVM::newPromise callback failed to lock");
            promiseCache->remove(handle);
            return;
        }
        const auto ctx = self->context.get();
        if (ctx == nullptr) [[unlikely]]
        {
            promiseCache->remove(handle);
            self->unlock();
            return;
        }
        auto ret = cbk(ctx);
        auto *promiseInCbk = promiseCache->get(handle);
        if (promiseInCbk == nullptr) [[unlikely]]
        {
            self->unlock();
            return;
        }
        promiseInCbk->callbackJS(ret);
        promiseCache->remove(handle);

        self->unlock();
    });

    return result;
}

JSValue JSVM::newPromiseVoid(std::function<void()> &&vf)
{
    return this->newPromise([cbk = std::move(vf)]([[maybe_unused]] JSContext *ctx) {
        cbk();
        return JS_UNDEFINED;
    });
}

JSValue JSVM::newPromiseBool(std::function<bool()> &&bf)
{
    return this->newPromise([cbk = std::move(bf)](JSContext *ctx) {
        const auto ret = cbk();
        return JS_NewBool(ctx, ret);
    });
}

JSValue JSVM::newPromiseInt32(std::function<int32_t()> &&i32f)
{
    return this->newPromise([cbk = std::move(i32f)](JSContext *ctx) {
        const auto ret = cbk();
        return JS_NewInt32(ctx, ret);
    });
}

JSValue JSVM::newPromiseInt64(std::function<int64_t()> &&i64f)
{
    return this->newPromise([cbk = std::move(i64f)](JSContext *ctx) {
        const auto ret = cbk();
        return JS_NewInt64(ctx, ret);
    });
}

JSValue JSVM::newPromiseFloat(std::function<double()> &&ff)
{
    return this->newPromise([cbk = std::move(ff)](JSContext *ctx) {
        const auto ret = cbk();
        return JS_NewFloat64(ctx, ret);
    });
}

JSValue JSVM::newPromiseString(std::function<const std::string()> &&sf)
{
    return this->newPromise([cbk = std::move(sf)](JSContext *ctx) {
        const auto ret = cbk();
        return JS_NewString(ctx, ret.c_str());
    });
}

JSVM::~JSVM()
{
    this->timerLooperTask.reset();
    this->promiseLooperTask.reset();
    js_std_set_worker_new_context_func(nullptr);
    this->jValueCache.clear();
    this->loadedScriptNames.clear();
    this->jGlobal = JS_UNDEFINED;
    this->context.reset();
    if (this->runtime != nullptr) [[likely]]
    {
        js_std_free_handlers(this->runtime.get());
    }
    this->runtime.reset();
}

} // namespace DynXX::Core::VM

#endif
