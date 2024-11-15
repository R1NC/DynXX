#include "JsBridge.hxx"
#include "../../include/NGenXXLog.hxx"

#include <fstream>
#include <sstream>
#include <streambuf>
#include <utility>
#include <thread>

constexpr const char *IMPORT_STD_OS_JS = "import * as std from 'qjs:std';\n"
                                         "import * as os from 'qjs:os';\n"
                                         "globalThis.std = std;\n"
                                         "globalThis.os = os;\n";

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

bool _ngenxx_js_loadScript(JSContext * ctx, const std::string &script, const std::string &name, const bool isModule)
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
        {// Check whether it's a JS Module or not，or QJS may crash
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS try to load invalid module");
            JS_FreeValue(ctx, jEvalRet);
            return false;
        }
        js_module_set_import_meta(ctx, jEvalRet, false, true);
        JSValue jEvalFuncRet = JS_EvalFunction(ctx, jEvalRet);
        JS_FreeValue(ctx, jEvalFuncRet);
        //this->jValues.push_back(jEvalRet);//Can not free here, or QJS may crash
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
        return NULL;
    
    js_std_add_helpers(ctx, 0, NULL);
    js_init_module_std(ctx, "qjs:std");
    js_init_module_os(ctx, "qjs:os");
    
    _ngenxx_js_loadScript(ctx, IMPORT_STD_OS_JS, "import-std-os.js", true);

    return ctx;
}

/// About JS event loop：
/// 1. Normal JS function does not depend on the event loop，but `promise` & `setTimeout()`/`setInterval()` do；
/// 2. The event loop should be created only once in a daemon thread to handle the coming events without blocking；
/// 3. `js_std_loop()` will check pending jobs triggered by `promise` and timer events triggered by `select` system call.
static JSValue _ngenxx_js_loop(JSContext *ctx)
{
    static JSValue _ngenxx_js_loop_jsv = js_std_loop(ctx);
}

static std::thread *_ngenxx_js_loopThread = nullptr;

static void _ngenxx_js_checkMainLoop(JSContext *ctx)
{
    if (!_ngenxx_js_loopThread)
    {
        _ngenxx_js_loopThread = new std::thread(_ngenxx_js_loop, ctx);
    }
}


NGenXX::JsBridge::JsBridge()
{
    this->runtime = JS_NewRuntime();
    js_std_init_handlers(this->runtime);
    JS_SetModuleLoaderFunc(this->runtime, NULL, js_module_loader, NULL);
    js_std_set_worker_new_context_func(_ngenxx_js_newContext);

    this->context = _ngenxx_js_newContext(this->runtime);
    this->jValues.push_back(JS_GetGlobalObject(this->context));

    this->loadScript(IMPORT_STD_OS_JS, "import-std-os.js", true);
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

    this->jValues.push_back(jFunc);//Can not free here, will be called in future

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
    const std::lock_guard<std::mutex> lock(this->mutex);
    return _ngenxx_js_loadScript(this->context, script, name, isModule);
}

bool NGenXX::JsBridge::loadBinary(Bytes bytes, const bool isModule)
{
    const std::lock_guard<std::mutex> lock(this->mutex);
    return js_std_eval_binary(this->context, bytes.data(), bytes.size(), 0);
}

static inline const std::string _ngenxx_j_jstr2stdstr(JSContext *ctx, JSValue jstr)
{
    auto c = JS_ToCString(ctx, jstr);
    auto s = std::string(c ?: "");
    JS_FreeCString(ctx, c);
    return s;
}

std::string NGenXX::JsBridge::callFunc(const std::string &func, const std::string &params, const bool await)
{
    const std::lock_guard<std::mutex> lock(this->mutex);
    std::string s;

    JSValue jFunc = JS_GetPropertyStr(this->context, this->jValues[0], func.c_str());
    if (JS_IsFunction(this->context, jFunc))
    {
        JSValue jParams = JS_NewString(this->context, params.c_str());
        JSValue argv[] = {jParams};

        JSValue jRes = JS_Call(this->context, jFunc, this->jValues[0], sizeof(argv), argv);
        if (JS_IsException(jRes))
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_Call failed ->");
            _ngenxx_js_dump_err(this->context);
        }
        else
        {
            _ngenxx_js_checkMainLoop(this->context);
            if (await)
            {
                jRes = js_std_await(this->context, jRes);   // Handle promise if needed
                s = std::move(_ngenxx_j_jstr2stdstr(this->context, jRes));
            }
            else
            {
                s = std::move(_ngenxx_j_jstr2stdstr(this->context, jRes));
            }
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

NGenXX::JsBridge::~JsBridge()
{
    js_std_set_worker_new_context_func(NULL);

    for (auto &jv : this->jValues)
    {
        uint32_t tag = JS_VALUE_GET_TAG(jv);
        if (tag != JS_TAG_MODULE)
            JS_FreeValue(this->context, jv);
    }
    JS_FreeContext(this->context);

    js_std_free_handlers(this->runtime);
    JS_FreeRuntime(this->runtime);
}
