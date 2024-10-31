Module['onRuntimeInitialized'] = function () {
    FS.mkdir('/data');
    //FS.mount(IDBFS, {}, '/data');
    var _initRet = Module._ngenxx_init('/data');

    if (_initRet > 0) {
        /**
        * WARNING: C/C++ callback JS function in async thread is not supported:
        * https://github.com/emscripten-core/emscripten/issues/16567
        */
        var _func = Module.addFunction(function (level, content) {
            var sContent = Module.UTF8ToString(content);
            console.log('NGenXX<' + level + '>' + sContent);
        }, 'vii');//wasm type signature just support number
        Module._ngenxx_log_set_callback(_func);

        _luaScript = jstr2wasm("function lNetHttpRequest(pReq)\n" +
            "pLog = '{\"level\":3,\"content\":\"Send Net Req from Lua..\"}';\n" +
            "ngenxx_log_printL(pLog);\n" +
            "rsp = ngenxx_net_http_requestL(pReq);\n" +
            "return rsp;\n" +
            "end");
        _loadRet = Module._ngenxx_lua_loadS(_luaScript);
    }
}

Module['destroy'] = function () {
    Module._ngenxx_release();
    Module._free(_loadRet);
    Module._free(_luaScript);
}

function jstr2wasm(jstr) {
    var len = jstr.length + 1;
    var ptr = Module._malloc(len);
    stringToUTF8(jstr, ptr, len);
    return ptr;
}

function testGetVersion() {
    _version = Module._ngenxx_get_version();

    version = Module.UTF8ToString(_version);
    window.alert("version:\n" + version);

    Module._free(_version);
}

function testHttpReq() {
    _url = jstr2wasm("https://rinc.xyz");
    _params = jstr2wasm("");
    _rsp = Module._ngenxx_net_http_request(_url, _params, 0, 0, 0, 0);

    rsp = Module.UTF8ToString(_rsp);
    window.alert("http rsp:\n" + rsp);

    Module._free(_rsp);
    Module._free(_params);
    Module._free(_url);
}

function testCallLua() {
    _funcName = jstr2wasm("lNetHttpRequest");
    _funcParams = jstr2wasm('{"url":"https://rinc.xyz","params":""}');
    _callRes = Module._ngenxx_lua_call(_funcName, _funcParams);

    callRes = Module.UTF8ToString(_callRes);
    window.alert("Lua running result:\n" + callRes);

    Module._free(_callRes);
    Module._free(_funcParams);
    Module._free(_funcName);
}