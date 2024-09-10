#include "JsBridge.hxx"
#include <fstream>
#include <sstream>
#include <streambuf>
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"

NGenXX::JsBridge::JsBridge()
{
    this->runtime = JS_NewRuntime();
    this->context = JS_NewContext(this->runtime);
}

bool NGenXX::JsBridge::bindFunc(const std::string &funcJ, JSCFunction *funcC)
{
    bool res = true;
    JSValue jFunc = JS_NewCFunction(this->context, funcC, funcJ.c_str(), 1);
    if (JS_IsException(jFunc))
    {
        Log::print(Error, "JS_NewCFunction failed");
        js_std_dump_error(this->context);
        res = false;
    }
    else
    {
        JSValue jGlobal = JS_GetGlobalObject(this->context);
        if (JS_DefinePropertyValueStr(this->context, jGlobal, funcJ.c_str(), jFunc, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE) < 0)
        {
            Log::print(Error, "JS_DefinePropertyValueStr failed");
            js_std_dump_error(this->context);
            res = false;
        }
        JS_FreeValue(this->context, jGlobal);
        JS_FreeValue(this->context, jFunc);
    }
    return res;
}

bool NGenXX::JsBridge::loadFile(const std::string &file)
{
    bool res = true;
    std::ifstream ifs(file.c_str());
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string script_text = ss.str();

    JSValue jRes = JS_Eval(this->context, script_text.c_str(), script_text.length(), file.c_str(), JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(jRes))
    {
        Log::print(Error, "JS_Eval failed");
        js_std_dump_error(this->context);
        res = false;
    }

    JS_FreeValue(this->context, jRes);
    return res;
}

std::string NGenXX::JsBridge::callFunc(const std::string &func, const std::string &params)
{
    std::string s;

    JSValue jGlobal = JS_GetGlobalObject(this->context);
    JSValue jFunc = JS_GetPropertyStr(this->context, jGlobal, func.c_str());
    if (JS_IsFunction(this->context, jFunc))
    {
        JSValue jParams = JS_NewString(this->context, params.c_str());
        JSValue argv[] = {jParams};

        JSValue jRes = JS_Call(this->context, jFunc, jGlobal, sizeof(argv), argv);
        if (JS_IsException(jRes))
        {
            Log::print(Error, "JS_Call failed");
            js_std_dump_error(this->context);
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
        Log::print(Error, ("Can not find JS func:" + func).c_str());
    }

    JS_FreeValue(this->context, jFunc);
    JS_FreeValue(this->context, jGlobal);

    return s;
}

NGenXX::JsBridge::~JsBridge()
{
    JS_FreeContext(this->context);
    JS_FreeRuntime(this->runtime);
}