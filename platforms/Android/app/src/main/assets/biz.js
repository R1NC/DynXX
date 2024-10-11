function jNetHttpRequest(params)
{
    var url = JSON.parse(params).url;
    var pLog = `{"level":1,"content":"Send Net Req from JS, url: ${url}"}`;
    ngenxx_log_printJ(pLog);
    return ngenxx_net_http_requestJ(params);
}
