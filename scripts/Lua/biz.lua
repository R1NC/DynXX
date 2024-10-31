--[[
Functions in `NGenXX.h` are accessible in Lua with a postfix of `L`；
The params and return value are (json) strings.
]]

function lNetHttpRequest(pReq)
    pLog = '{"level":3,"content":"Send Net HTTP Req from Lua.."}';
   ngenxx_log_printL(pLog);
    rsp = ngenxx_net_http_requestL(pReq);
    return rsp;
end

