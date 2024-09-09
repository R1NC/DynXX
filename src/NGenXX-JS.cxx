#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "NGenXX-JS.hxx"
#include "../include/NGenXX.h"
#include "../external/quickjs/quickjs.h"
#include "js/JsBridge.hxx"

void _ngenxx_js_init(NGenXXHandle *handle)
{
    if (handle == NULL)
        return;
    handle->qjs = new NGenXX::JsBridge();
}

void _ngenxx_js_release(NGenXXHandle *handle)
{
    if (handle == NULL || handle->qjs == NULL)
        return;
    delete handle->qjs;
}

#pragma mark Net.Http

static JSValue ngenxx_net_http_requestJ(JSContext *ctx, JSValueConst this_val,
                                        int argc, JSValueConst *argv)
{
    const char *url = JS_ToCString(ctx, argv[0]);
    if (url == NULL)
        return JS_EXCEPTION;

    const char *res = ngenxx_net_http_request(url, NULL, 0, NULL, 0, 0);

    JS_FreeCString(ctx, url);

    return JS_NewString(ctx, res);
}

#pragma mark JS

bool ngenxx_J_loadF(void *sdk, const char *file)
{
    if (sdk == NULL || file == NULL)
        return false;
    return ((NGenXX::JsBridge *)(((NGenXXHandle *)sdk)->qjs))->loadFile(std::string(file));
}

const char *ngenxx_J_call(void *sdk, const char *func, const char *params)
{
    if (sdk == NULL || func == NULL)
        return NULL;
    str2charp(((NGenXX::JsBridge *)(((NGenXXHandle *)sdk)->qjs))->callFunc(std::string(func), std::string(params)));
}

#pragma mark JS Module Register

static const JSCFunctionListEntry _ngenxx_js_func_list[] = {
    JS_CFUNC_DEF("ngenxx_net_http_requestJ", 1, ngenxx_net_http_requestJ),
};

static const int _ngenxx_js_func_count(const JSCFunctionListEntry *ngenxx_js_func_list)
{
    return sizeof(ngenxx_js_func_list) / sizeof((ngenxx_js_func_list)[0]);
}

static int _ngenxx_js_module_init(JSContext *ctx, JSModuleDef *m)
{
    return JS_SetModuleExportList(ctx, m, _ngenxx_js_func_list, _ngenxx_js_func_count(_ngenxx_js_func_list));
}

void registerJsModule(NGenXXHandle *handle)
{
    ((NGenXX::JsBridge *)(handle->qjs))->addModule("NGenXX", _ngenxx_js_module_init, _ngenxx_js_func_list);
}