#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "NGenXX-JS.hxx"
#include "../include/NGenXX.h"
#include "../external/quickjs/quickjs.h"
#include "js/JsBridge.hxx"

void registerJsModule(NGenXXHandle *handle);
#define BIND_JS_FUNC(h, f) ((NGenXX::JsBridge *)(handle->qjs))->bindFunc(#f, f);

void _ngenxx_js_init(NGenXXHandle *handle)
{
    if (handle == NULL)
        return;
    handle->qjs = new NGenXX::JsBridge();
    registerJsModule(handle);
}

void _ngenxx_js_release(NGenXXHandle *handle)
{
    if (handle == NULL || handle->qjs == NULL)
        return;
    delete handle->qjs;
}

#pragma mark Net.Http

JSValue ngenxx_net_http_requestJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
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
    return str2charp(((NGenXX::JsBridge *)(((NGenXXHandle *)sdk)->qjs))->callFunc(std::string(func), std::string(params)));
}

#pragma mark JS Module Register

void registerJsModule(NGenXXHandle *handle)
{
    BIND_JS_FUNC(handle, ngenxx_net_http_requestJ);
}