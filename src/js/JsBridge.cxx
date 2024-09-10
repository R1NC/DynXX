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

bool NGenXX::JsBridge::loadFile(const std::string &file)
{
    std::ifstream ifs(file.c_str());
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string script_text = ss.str();

    JSValue res = JS_Eval(this->context, script_text.c_str(), script_text.length(), file.c_str(), JS_EVAL_TYPE_GLOBAL);
    bool success = !JS_IsException(res);

    JS_FreeValue(this->context, res);
    return success;
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

        JSValue jRes = JS_Call(this->context, jFunc, JS_UNDEFINED, sizeof(argv), argv);
        if (!JS_IsException(jRes))
        {
            s = std::string(JS_ToCString(this->context, jRes));
        }
        else
        {
            Log::print(Error, "JS_Call failed");
            js_std_dump_error(this->context);
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

static const int _ngenxx_js_func_count(const JSCFunctionListEntry *ngenxx_js_func_list)
{
    return sizeof(ngenxx_js_func_list) / sizeof((ngenxx_js_func_list)[0]);
}

bool NGenXX::JsBridge::addModule(const std::string &module, JSModuleInitFunc *callback, const JSCFunctionListEntry *funcList)
{
    JSModuleDef *m;
    m = JS_NewCModule(this->context, module.c_str(), callback);
    if (!m)
        return false;
    JS_AddModuleExportList(this->context, m, funcList, _ngenxx_js_func_count(funcList));
}