#include "JsBridge.hxx"
#include "../../include/NGenXXLog.hxx"
#include <fstream>
#include <sstream>
#include <streambuf>

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

NGenXX::JsBridge::JsBridge()
{
    this->runtime = JS_NewRuntime();
    JS_SetModuleLoaderFunc(this->runtime, NULL, js_module_loader, NULL);

    this->context = JS_NewContext(this->runtime);
    this->jValues.push_back(JS_GetGlobalObject(this->context));

    js_std_add_helpers(this->context, 0, NULL);
    js_std_init_handlers(this->runtime);
    js_init_module_std(this->context, "qjs:std");
    js_init_module_os(this->context, "qjs:os");
    JSValue jEvalRet = JS_Eval(this->context, IMPORT_STD_OS_JS, strlen(IMPORT_STD_OS_JS), "import-std-os.js", JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
    if (!JS_IsException(jEvalRet))
    {
        js_module_set_import_meta(this->context, jEvalRet, false, true);
        JSValue jEvalFuncRet = JS_EvalFunction(this->context, jEvalRet);
        JSValue jAwaitRet = js_std_await(this->context, jEvalFuncRet);
        JS_FreeValue(this->context, jAwaitRet);
        JS_FreeValue(this->context, jEvalFuncRet);
    }
    else
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "J Import std/os failed ->");
        _ngenxx_js_dump_err(this->context);
    }
    this->jValues.push_back(jEvalRet);
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

    this->jValues.push_back(jFunc);

    return res;
}

bool NGenXX::JsBridge::loadFile(const std::string &file)
{
    std::ifstream ifs(file.c_str());
    std::stringstream ss;
    ss << ifs.rdbuf();
    return this->loadScript(ss.str(), file);
}

bool NGenXX::JsBridge::loadScript(const std::string &script, const std::string &name)
{
    bool res = true;
    JSValue jRes = JS_Eval(this->context, script.c_str(), script.length(), name.c_str(), JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(jRes))
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "JS_Eval failed ->");
        _ngenxx_js_dump_err(this->context);
        res = false;
    }

    JS_FreeValue(this->context, jRes);
    return res;
}

bool NGenXX::JsBridge::loadBinary(Bytes bytes)
{
    auto [data, len] = bytes;
    return js_std_eval_binary(this->context, data, len, 0);
}

std::string NGenXX::JsBridge::callFunc(const std::string &func, const std::string &params)
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
            JSValue jLoop = js_std_loop(this->context); // Wating for async tasks
            jRes = js_std_await(this->context, jRes);   // Handle promise if needed
            auto c = JS_ToCString(this->context, jRes);
            s = std::string(c ?: "");
            JS_FreeCString(this->context, c);
            JS_FreeValue(this->context, jLoop);
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
