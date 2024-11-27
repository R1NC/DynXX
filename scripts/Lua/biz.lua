--local NGenXX = require 'NGenXX.lua'

function TestNetHttpRequest(url)
    return NGenXX.NetHttpRequest(url, 0, 10000)
end
