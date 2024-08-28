--[[
Functions in `EngineXX.h` are accessible in Lua with a postfix of `L`；
The params and return value are (json) strings.
]]

function lNetHttpReq(pReq)
    pLog = '{"level":3,"content":"Send Net HTTP Req from Lua.."}';
    enginexx_log_printL(pLog);
    rsp = enginexx_net_http_reqL(pReq);
    return rsp;
end

