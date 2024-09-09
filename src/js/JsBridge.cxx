#include "JsBridge.hxx"

NGenXX::JsBridge::JsBridge()
{
    this->runtime = JS_NewRuntime();
    this->context = JS_NewContext(this->runtime);
}

bool NGenXX::JsBridge::loadFile(const std::string &file)
{
    js_load_file(this->context, 0, file.c_str());
}

std::string NGenXX::JsBridge::callFunc(const std::string &func, const std::string &params)
{
    JSValue jFunc = JS_NewString(this->context, func.c_str());
    JSValue jParams = JS_NewString(this->context, func.c_str());
    JSValue global = JS_GetGlobalObject(this->context);
    JSValue argv[] = {jParams};
    JS_Call(this->context, jFunc, global, sizeof(argv), argv);
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