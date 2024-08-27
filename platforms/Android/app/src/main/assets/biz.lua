--[[
All functions in `EngineXX.h`（except the ones with `enginexx_L_` prefix）are accessible in Lua with a postfix of `L`；
The params and return value are (json) strings.
]]

function lTestGetVersion()
    v = enginexx_get_versionL()
    return 'EngineXX Version: ' .. v
end

function lTestHttpReq(p)
    rsp = enginexx_http_reqL(p)
    return 'HTTP response: ' .. rsp
end
