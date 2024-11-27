--local json = require 'json.lua'

--[[local]] NGenXX = {}

function NGenXX.LogPrint(_level, _content)
    inJson = json.stringify({
        level = _level,
        content = _content
    })
    ngenxx_log_printL(inJson)
end

function NGenXX.NetHttpRequest(_url, _method, _timeout)
    NGenXX.LogPrint(3, 'Send Net HTTP Req from Lua..')
    inJson = json.stringify({
        url = _url,
        method = _method,
        timeout = _timeout
    })
    return ngenxx_net_http_requestL(inJson)
end

return NGenXX
