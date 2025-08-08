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

function testHttpReq() {
    var cRrl = jstr2wasm("https://rinc.xyz");
    var cParams = jstr2wasm("");

    new Promise((resolve, reject) => {
        var cRsp = Module._ngenxx_net_http_request(cRrl, cParams, 0, 0, 0, 0);
        var sRsp = Module.UTF8ToString(cRsp);
        Module._free(cRsp);
        Module._free(cParams);
        Module._free(cRrl);
        resolve(sRsp);
    }).then((sRsp) => {
        console.log("http rsp:\n" + sRsp);
    });
}

function testCallLua() {
    var cFuncName = jstr2wasm("lNetHttpRequest");
    var cFuncParams = jstr2wasm('{"url":"https://rinc.xyz","params":""}');
    var cCallRes = Module._ngenxx_lua_call(cFuncName, cFuncParams);

    var sCallRes = Module.UTF8ToString(cCallRes);
    window.alert("Lua running result:\n" + sCallRes);

    Module._free(cCallRes);
    Module._free(cFuncParams);
    Module._free(cFuncName);
}