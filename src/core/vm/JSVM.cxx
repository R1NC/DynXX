#if defined(USE_QJS)
#include "JSVM.hxx"

#include <fstream>
#include <sstream>
#include <utility>

#include <NGenXXLog.hxx>

namespace
{
    constexpr auto IMPORT_STD_OS_JS = "import * as std from 'qjs:std';\n"
                                         "import * as os from 'qjs:os';\n"
                                         "globalThis.std = std;\n"
                                         "globalThis.os = os;\n";

#pragma mark JSVM dump error

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

#pragma mark JSVM Internal

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

#pragma mark JS Promise wrapper

    struct Promise
    {
        JSValue p{JS_UNDEFINED};
        JSValue f[2]{JS_UNDEFINED, JS_UNDEFINED};
    };

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
}

#pragma mark JSVM Internal

JSValue NGenXX::Core::VM::JSVM::jAwait(const JSValue obj)
{
    auto ret = JS_UNDEFINED;
    for (;;)
    {
        /// Do not force to acquire the lock, to avoid blocking the JS event loop.
        if (!tryLockMutex(this->vmMutex)) [[unlikely]]
        {
            sleep();
            continue;
        }
        if (const auto state = JS_PromiseState(this->context, obj); state == JS_PROMISE_FULFILLED)
        {
            ret = JS_PromiseResult(this->context, obj);
            JS_FreeValue(this->context, obj);
            break;
        }
        else if (state == JS_PROMISE_REJECTED)
        {
            ret = JS_Throw(this->context, JS_PromiseResult(this->context, obj));
            JS_FreeValue(this->context, obj);
            break;
        }
        else if (state == JS_PROMISE_PENDING)
        {
            /// Promise is executing: release the lock, sleep for a while. To avoid blocking the js event loop, or overloading CPU.
            unlockMutex(this->vmMutex);
            sleep();
            continue;
        }
        else
        {
            /// Not a Promise: release the lock, return the result immediately.
            ret = obj;
            break;
        }
    }
    unlockMutex(this->vmMutex);
    return ret;
}

#pragma mark JSValueHash & JSValueEqual

std::size_t NGenXX::Core::VM::JSVM::JSValueHash::operator()(const JSValue &jv) const 
{
    return std::hash<void *>()(JS_VALUE_GET_PTR(jv));
}

bool NGenXX::Core::VM::JSVM::JSValueEqual::operator()(const JSValue &left, const JSValue &right) const 
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

#pragma mark JSVM API

NGenXX::Core::VM::JSVM::JSVM()
{
    this->runtime = JS_NewRuntime();
    js_std_init_handlers(this->runtime);
    JS_SetModuleLoaderFunc(this->runtime, nullptr, js_module_loader, nullptr);
    js_std_set_worker_new_context_func(_newContext);

    this->context = _newContext(this->runtime);
    this->jGlobal = JS_GetGlobalObject(this->context);// Can not free here, will be called in future

    this->executor.add([ctx = this->context, &mtx = this->vmMutex]() {
        if (tryLockMutex(mtx))
        {
            js_std_loop_promise(ctx);
            unlockMutex(mtx);
        }
    });
    this->executor.add([ctx = this->context, &mtx = this->vmMutex]() {
        if (tryLockMutex(mtx))
        {
            js_std_loop_timer(ctx);
            unlockMutex(mtx);
        }
    });
}

bool NGenXX::Core::VM::JSVM::bindFunc(const std::string &funcJ, JSCFunction *funcC)
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

bool NGenXX::Core::VM::JSVM::loadFile(const std::string &file, bool isModule)
{
    std::ifstream ifs(file.c_str());
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return this->loadScript(ss.str(), file, isModule);
}

bool NGenXX::Core::VM::JSVM::loadScript(const std::string &script, const std::string &name, bool isModule) {
    auto lock = std::lock_guard(this->vmMutex);
    return _loadScript(this->context, script, name, isModule);
}

bool NGenXX::Core::VM::JSVM::loadBinary(const Bytes &bytes, bool isModule) {
    auto lock = std::lock_guard(this->vmMutex);
    return js_std_eval_binary(this->context, bytes.data(), bytes.size(), 0);
}

/// WARNING: Nested call between native and JS requires a reenterable `recursive_mutex` here!
std::optional<std::string> NGenXX::Core::VM::JSVM::callFunc(const std::string &func, const std::string &params, bool await) {
    auto lock = std::unique_lock(this->vmMutex);
    std::string s;
    auto success = false;

    if (const auto jFunc = JS_GetPropertyStr(this->context, this->jGlobal, func.c_str()); JS_IsFunction(this->context, jFunc)) [[likely]]
    {
        const auto jParams = JS_NewString(this->context, params.c_str());
        JSValue argv[] = {jParams};

        auto jRes = JS_Call(this->context, jFunc, this->jGlobal, sizeof(argv), argv);

        /// Release the lock imediately, to avoid blocking the JS event loop.
        lock.unlock();

        if (JS_IsException(jRes)) [[unlikely]]
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_Call failed ->");
            dumpJsErr(this->context);
        }
        else [[likely]]
        {
            success = true;
            if (await)
            {/// WARNING: Do not use built-in `js_std_await()`, since it will triger the Promise Event Loop once again.
                jRes = this->jAwait(jRes); // Handle promise if needed
            }
            const auto cS = JS_ToCString(this->context, jRes);
            s = std::move(wrapStr(cS));
            JS_FreeCString(this->context, cS);
        }
        JS_FreeValue(this->context, jRes);
        JS_FreeValue(this->context, jParams);
    }
    else [[unlikely]]
    {
        lock.unlock();
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Can not find JS func:{}", func);
    }

    return success? std::make_optional(s): std::nullopt;
}

JSValue NGenXX::Core::VM::JSVM::newPromise(std::function<JSValue()> &&jf)
{
    auto jPromise = _newPromise(this->context);
    if (jPromise == nullptr) [[unlikely]]
    {
        return JS_EXCEPTION;
    }
    
    this->executor.add([&mtx = this->vmMutex, ctx = this->context, jPromise, jf = std::move(jf)] {
        auto lock = std::lock_guard(mtx);

        const auto jRet = jf();

        _callbackPromise(ctx, jPromise, jRet);
    });

    return jPromise->p;
}

JSValue NGenXX::Core::VM::JSVM::newPromiseVoid(std::function<void()> &&f)
{
    return this->newPromise([f = std::move(f)]() {
        f();
        return JS_UNDEFINED;
    });
}

JSValue NGenXX::Core::VM::JSVM::newPromiseBool(std::function<bool()> &&f)
{
    return this->newPromise([ctx = this->context, f = std::move(f)]{
        const auto ret = f();
        return JS_NewBool(ctx, ret);
    });
}

JSValue NGenXX::Core::VM::JSVM::newPromiseInt32(std::function<int32_t()> &&f)
{
    return this->newPromise([ctx = this->context, f = std::move(f)]{
        const auto ret = f();
        return JS_NewInt32(ctx, ret);
    });
}

JSValue NGenXX::Core::VM::JSVM::newPromiseInt64(std::function<int64_t()> &&f)
{
    return this->newPromise([ctx = this->context, f = std::move(f)]{
        const auto ret = f();
        return JS_NewInt64(ctx, ret);
    });
}

JSValue NGenXX::Core::VM::JSVM::newPromiseFloat(std::function<double()> &&f)
{
    return this->newPromise([ctx = this->context, f = std::move(f)]{
        const auto ret = f();
        return JS_NewFloat64(ctx, ret);
    });
}

JSValue NGenXX::Core::VM::JSVM::newPromiseString(std::function<const std::string()> &&f)
{
    return this->newPromise([ctx = this->context, f = std::move(f)]{
        const auto ret = f();
        return JS_NewString(ctx, ret.c_str() != nullptr ? ret.c_str() : "");
    });
}

NGenXX::Core::VM::JSVM::~JSVM()
{
    this->active = false;
    js_std_loop_cancel(this->runtime);

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
}
#endif
