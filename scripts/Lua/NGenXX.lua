NGenXX = {}

NGenXX.Log = {}

NGenXX.Log.Level = {
    Debug = 3,
    Info = 4,
    Warn = 5,
    Error = 6,
    Fatal = 7,
    None = 8
}

function NGenXX.Log.print(_level, _content)
    inJson = JSON.stringify({
        level = _level,
        content = _content
    })
    ngenxx_log_printL(inJson)
end

NGenXX.Net = {}
NGenXX.Net.Http = {}

NGenXX.Net.Http.Method = {
    Get = 0,
    Post = 1,
    Put = 2
}

function NGenXX.Net.Http.request(_url, _method, _timeout)
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'Send Net HTTP Req from Lua..')
    inJson = JSON.stringify({
        url = _url,
        method = _method,
        timeout = _timeout
    })
    return ngenxx_net_http_requestL(inJson)
end
