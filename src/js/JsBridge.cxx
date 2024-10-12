#include "JsBridge.hxx"
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"
#include <fstream>
#include <sstream>
#include <streambuf>

static void _ngenxx_js_dump_obj(JSContext *ctx, JSValueConst val)
{
    const char *str;
    str = JS_ToCString(ctx, val);
    if (str)
    {
        NGenXX::Log::print(NGenXXLogLevelError, str);
        JS_FreeCString(ctx, str);
    }
}

static void _ngenxx_js_dump_error1(JSContext *ctx, JSValueConst exception_val)
{
    JSValue val;
    bool is_error;
    is_error = JS_IsError(ctx, exception_val);
    _ngenxx_js_dump_obj(ctx, exception_val);
    if (is_error)
    {
        val = JS_GetPropertyStr(ctx, exception_val, "stack");
        if (!JS_IsUndefined(val))
        {
            _ngenxx_js_dump_obj(ctx, val);
        }
        JS_FreeValue(ctx, val);
    }
}

void _ngenxx_js_dump_error(JSContext *ctx)
{
    JSValue exception_val;
    exception_val = JS_GetException(ctx);
    _ngenxx_js_dump_error1(ctx, exception_val);
    JS_FreeValue(ctx, exception_val);
}

NGenXX::JsBridge::JsBridge()
{
    this->runtime = JS_NewRuntime();
    this->context = JS_NewContext(this->runtime);
    js_std_add_helpers(this->context, 0, NULL);
    js_std_init_handlers(this->runtime);
    js_init_module_std(this->context, "std");
    js_init_module_os(this->context, "os");
    this->global = JS_GetGlobalObject(this->context);
}

bool NGenXX::JsBridge::bindFunc(const std::string &funcJ, JSCFunction *funcC)
{
    bool res = true;
    JSValue jFunc = JS_NewCFunction(this->context, funcC, funcJ.c_str(), 1);
    if (JS_IsException(jFunc))
    {
        Log::print(NGenXXLogLevelError, "JS_NewCFunction failed ->");
        _ngenxx_js_dump_error(this->context);
        res = false;
    }
    else
    {
        if (!JS_DefinePropertyValueStr(this->context, this->global, funcJ.c_str(), jFunc, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE))
        {
            Log::print(NGenXXLogLevelError, "JS_DefinePropertyValueStr failed ->");
            _ngenxx_js_dump_error(this->context);
            res = false;
        }
    }
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
        _ngenxx_js_dump_error(this->context);
        res = false;
    }

    JS_FreeValue(this->context, jRes);
    return res;
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
            _ngenxx_js_dump_error(this->context);
        }
        else
        {
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
    JS_FreeValue(this->context, this->global);
    JS_FreeContext(this->context);
    JS_FreeRuntime(this->runtime);
}
