#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "NGenXX-JS.hxx"
#include "../include/NGenXX.h"
#include "../external/quickjs/quickjs.h"
#include "NGenXX-inner.hxx"
#include "js/JsBridge.hxx"

void registerJsModule();
#define BIND_JS_FUNC(f) ((NGenXX::JsBridge *)(_ngenxx_handle->qjs))->bindFunc(#f, f);

bool _ngenxx_js_init(void)
{
    if (_ngenxx_handle == NULL) return false;
    if (_ngenxx_handle->qjs != NULL) return true;
    _ngenxx_handle->qjs = new NGenXX::JsBridge();
    registerJsModule();
    return true;
}

void _ngenxx_js_release(void)
{
    if (_ngenxx_handle == NULL || _ngenxx_handle->qjs == NULL)
        return;
    delete _ngenxx_handle->qjs;
    _ngenxx_handle->qjs = NULL;
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

void registerJsModule()
{
    BIND_JS_FUNC(ngenxx_net_http_requestJ);
}