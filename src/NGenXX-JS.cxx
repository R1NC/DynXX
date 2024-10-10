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
#include "json/JsonDecoder.hxx"

static NGenXX::JsBridge *_ngenxx_js;

void registerJsModule();
#define BIND_JS_FUNC(f) _ngenxx_js->bindFunc(#f, f);

void _ngenxx_js_init(void)
{
    if (_ngenxx_js != NULL) return;
    _ngenxx_js = new NGenXX::JsBridge();
    registerJsModule();
}

void _ngenxx_js_release(void)
{
    if (_ngenxx_js == NULL)
        return;
    delete _ngenxx_js;
    _ngenxx_js = NULL;
}

#pragma mark Net.Http

static JSValue ngenxx_net_http_requestJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    if (json == NULL)
        return JS_EXCEPTION;

    NGenXX::Json::Decoder decoder(json);
    const char *url = str2charp(decoder.readString(decoder.readNode(NULL, "url")));
    const char *params = str2charp(decoder.readString(decoder.readNode(NULL, "params")));
    const int method = decoder.readNumber(decoder.readNode(NULL, "method"));
    const int header_c = decoder.readNumber(decoder.readNode(NULL, "header_c"));
    const int form_field_count = decoder.readNumber(decoder.readNode(NULL, "form_field_count"));
    const unsigned long timeout = decoder.readNumber(decoder.readNode(NULL, "timeout"));

    char **header_v = (char **)malloc(HTTP_HEADERS_MAX_COUNT * sizeof(char *));
    void *header_vNode = decoder.readNode(NULL, "header_v");
    if (header_vNode)
    {
        decoder.readChildren(header_vNode, [&](int idx, void *child) -> void {
            if (idx == HTTP_HEADERS_MAX_COUNT) return;
            header_v[idx] = (char *)malloc(HTTP_HEADER_MAX_LENGTH * sizeof(char) + 1);
            strcpy(header_v[idx], decoder.readString(child).c_str());
        });
    }

    if (method < 0 || url == NULL || header_c > HTTP_HEADERS_MAX_COUNT)
        return JS_UNDEFINED;

    //TODO
    const char *res = ngenxx_net_http_request(url, params, method, (const char **)header_v, header_c, NULL, NULL, NULL, 0, NULL, 0, timeout);

    free((void *)url);
    free((void *)params);
    for (int i = 0; i < header_c; i++)
    {
        free((void *)header_v[i]);
    }

    JS_FreeCString(ctx, json);

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