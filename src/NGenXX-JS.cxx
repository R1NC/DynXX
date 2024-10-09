#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "NGenXX-JS.hxx"
#include "../include/NGenXX.h"
#include "../external/quickjs/quickjs.h"
#include "NGenXX-inner.hxx"
#include "util/TypeUtil.hxx"
#include "js/JsBridge.hxx"

static NGenXX::JsBridge *_ngenxx_js;

void registerJsModule();
#define BIND_JS_FUNC(f) _ngenxx_js->bindFunc(#f, f);

bool _ngenxx_js_init(void)
{
    if (_ngenxx_js != NULL) return true;
    _ngenxx_js = new NGenXX::JsBridge();
    registerJsModule();
    return true;
}

void _ngenxx_js_release(void)
{
    if (_ngenxx_js == NULL)
        return;
    delete _ngenxx_js;
    _ngenxx_js = NULL;
}

#pragma mark Net.Http

JSValue ngenxx_net_http_requestJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *url = JS_ToCString(ctx, argv[0]);
    if (url == NULL)
        return JS_EXCEPTION;

    const char *res = ngenxx_net_http_request(url, NULL, 0, NULL, 0, NULL, NULL, NULL, 0, NULL, 0, 5000);

    JS_FreeCString(ctx, url);

    return JS_NewString(ctx, res);
}

#pragma mark JS

bool ngenxx_J_loadF(const char *file)
{
    if (_ngenxx_js == NULL || file == NULL)
        return false;
    return _ngenxx_js->loadFile(std::string(file));
}

bool ngenxx_J_loadS(const char *script, const char *name)
{
    if (_ngenxx_js == NULL || script == NULL || name == NULL)
        return false;
    return _ngenxx_js->loadScript(std::string(script), std::string(name));
}

const char *ngenxx_J_call(const char *func, const char *params)
{
    if (_ngenxx_js == NULL || func == NULL)
        return NULL;
    return str2charp(_ngenxx_js->callFunc(std::string(func), std::string(params)));
}

#pragma mark JS Module Register

void registerJsModule()
{
    BIND_JS_FUNC(ngenxx_net_http_requestJ);
}