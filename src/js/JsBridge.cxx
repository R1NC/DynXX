#include "JsBridge.hxx"
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"
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
        NGenXX::Log::print(NGenXXLogLevelError, str);
        JS_FreeCString(ctx, str);
    }
}

void _ngenxx_js_dump_err(JSContext *ctx)
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
    this->context = JS_NewContext(this->runtime);

    js_std_add_helpers(this->context, 0, NULL);
    js_std_init_handlers(this->runtime);
    js_init_module_std(this->context, "qjs:std");
    js_init_module_os(this->context, "qjs:os");
    JSValue std_val = JS_Eval(this->context, IMPORT_STD_OS_JS, strlen(IMPORT_STD_OS_JS), "import-std-os.js", JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
    if (!JS_IsException(std_val))
    {
        js_module_set_import_meta(this->context, std_val, 1, 1);
        std_val = JS_EvalFunction(this->context, std_val);
    }
    else
    {
        Log::print(NGenXXLogLevelError, "JS Import std/os failed ->");
        _ngenxx_js_dump_err(this->context);
    }
    std_val = js_std_await(this->context, std_val);
    JS_FreeValue(this->context, std_val);

    this->global = JS_GetGlobalObject(this->context);
}

bool NGenXX::JsBridge::bindFunc(const std::string &funcJ, JSCFunction *funcC)
{
    bool res = true;
    JSValue jFunc = JS_NewCFunction(this->context, funcC, funcJ.c_str(), 1);
    if (JS_IsException(jFunc))
    {
        Log::print(NGenXXLogLevelError, "JS_NewCFunction failed ->");
        _ngenxx_js_dump_err(this->context);
        res = false;
    }
    else
    {
        if (!JS_DefinePropertyValueStr(this->context, this->global, funcJ.c_str(), jFunc, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE))
        {
            Log::print(NGenXXLogLevelError, "JS_DefinePropertyValueStr failed ->");
            _ngenxx_js_dump_err(this->context);
            res = false;
        }
    }

    this->cFuncs.push_back(jFunc);

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
        Log::print(NGenXXLogLevelError, "JS_Eval failed ->");
        _ngenxx_js_dump_err(this->context);
        res = false;
    }

    JS_FreeValue(this->context, jRes);
    return res;
}

bool NGenXX::JsBridge::loadBinary(NGenXX::Bytes bytes)
{
    auto [data, len] = bytes;
    js_std_eval_binary(this->context, data, len, 0);
    return true;
}

std::string NGenXX::JsBridge::callFunc(const std::string &func, const std::string &params)
{
    std::string s;

    JSValue jFunc = JS_GetPropertyStr(this->context, this->global, func.c_str());
    if (JS_IsFunction(this->context, jFunc))
    {
        JSValue jParams = JS_NewString(this->context, params.c_str());
        JSValue argv[] = {jParams};

        JSValue jRes = JS_Call(this->context, jFunc, this->global, sizeof(argv), argv);
        if (JS_IsException(jRes))
        {
            Log::print(NGenXXLogLevelError, "JS_Call failed ->");
            _ngenxx_js_dump_err(this->context);
        }
        else
        {
            js_std_loop(this->context); // Wating for async tasks
            s = std::string(JS_ToCString(this->context, jRes));
        }
        JS_FreeValue(this->context, jRes);
        JS_FreeValue(this->context, jParams);
    }
    else
    {
        Log::print(NGenXXLogLevelError, ("Can not find JS func:" + func).c_str());
    }

    return s;
}

NGenXX::JsBridge::~JsBridge()
{
    for (auto &f : this->cFuncs)
        JS_FreeValue(this->context, f);
    JS_FreeValue(this->context, this->global);
    JS_FreeContext(this->context);
    JS_FreeRuntime(this->runtime);
}
