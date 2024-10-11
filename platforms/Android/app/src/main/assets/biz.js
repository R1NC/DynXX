function log_print(l, c)
{
    ngenxx_log_printJ(`{"level":${l},"content":"${c}"}`);
}

function jNetHttpRequest(params)
{
    var url = JSON.parse(params).url;
    log_print(1, `Send Net Req from JS, url: ${url}`);
    return ngenxx_net_http_requestJ(params);
}

function str2bytes(s) {
    return Array.from(s, char => char.charCodeAt(0))
}

function bytes2str(b) {
    return b.map((byte) => {
        return String.fromCharCode(byte);
    }).join("");
}

function jBase64Encode(s)
{
    var bytes = str2bytes(s);
    var encodedJson = ngenxx_crypto_base64_encodeJ(`{"inBytes":[${bytes}], "inLen":${bytes.length}}`);
    var encodedBytes = JSON.parse(encodedJson);
    return bytes2str(encodedBytes);
}

function jBase64Decode(s)
{
    var bytes = str2bytes(s);
    var decodedJson = ngenxx_crypto_base64_decodeJ(`{"inBytes":[${bytes}], "inLen":${bytes.length}}`);
    var decodedBytes = JSON.parse(decodedJson);
    return bytes2str(decodedBytes);
}
