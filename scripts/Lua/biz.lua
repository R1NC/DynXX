--local NGenXX = require 'NGenXX.lua'

function TestNetHttpRequest(url)
    method = NGenXX.Net.Http.Method.Get
    timeout = 10 * 1000
    return NGenXX.Net.Http.request(url, method, timeout)
end
