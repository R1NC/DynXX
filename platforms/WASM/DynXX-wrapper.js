let _luaScript = 0;
let _loadRet = 0;

Module['onRuntimeInitialized'] = function () {
    FS.mkdir('/data');
    //FS.mount(IDBFS, {}, '/data');
    const _initRet = Module._dynxx_init('/data');

    if (_initRet > 0) {
        /**
        * WARNING: C/C++ callback JS function in async thread is not supported:
        * https://github.com/emscripten-core/emscripten/issues/16567
        */
        const _func = Module.addFunction(function (level, content) {
            const sContent = Module.UTF8ToString(content);
            console.log('DynXX<' + level + '>' + sContent);
        }, 'vii');//wasm type signature just support number
        Module._dynxx_log_set_callback(_func);

        _luaScript = jstr2wasm("function lNetHttpRequest(pReq)\n" +
            "pLog = '{\"level\":3,\"content\":\"Send Net Req from Lua..\"}';\n" +
            "dynxx_log_printL(pLog);\n" +
            "rsp = dynxx_net_http_requestL(pReq);\n" +
            "return rsp;\n" +
            "end");
        _loadRet = Module._dynxx_lua_loadS(_luaScript);
    }
}

Module['destroy'] = function () {
    Module._dynxx_release();
    Module._free(_loadRet);
    Module._free(_luaScript);
}

function jstr2wasm(jstr) {
    const len = jstr.length + 1;
    const ptr = Module._malloc(len);
    stringToUTF8(jstr, ptr, len);
    return ptr;
}

function testHttpReq() {
    const cRrl = jstr2wasm("https://rinc.xyz");
    const cParams = jstr2wasm("");

    new Promise((resolve, reject) => {
        const cRsp = Module._dynxx_net_http_request(cRrl, cParams, 0, 0, 0, 0);
        const sRsp = Module.UTF8ToString(cRsp);
        Module._free(cRsp);
        Module._free(cParams);
        Module._free(cRrl);
        resolve(sRsp);
    }).then((sRsp) => {
        console.log("http rsp:\n" + sRsp);
    });
}

function testCallLua() {
    const cFuncName = jstr2wasm("lNetHttpRequest");
    const cFuncParams = jstr2wasm('{"url":"https://rinc.xyz","params":""}');
    const cCallRes = Module._dynxx_lua_call(cFuncName, cFuncParams);

    const sCallRes = Module.UTF8ToString(cCallRes);
    window.alert("Lua running result:\n" + sCallRes);

    Module._free(cCallRes);
    Module._free(cFuncParams);
    Module._free(cFuncName);
}
