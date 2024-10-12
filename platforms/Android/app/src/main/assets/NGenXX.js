// Utils

function str2bytes(s) {
    return Array.from(s, char => char.charCodeAt(0))
}

function bytes2str(bs) {
    return bs.map((b) => {
        return String.fromCharCode(b);
    }).join("");
}

// Log

function NGenXXLogPrint(l, c) {
    var inJson = `{"level":${l},"content":"${c}"}`;
    ngenxx_log_printJ(inJson);
}

// Net.Http

function NGenXXHttpReq(params) {
    var url = JSON.parse(params).url;
    NGenXXLogPrint(1, `Send Net Req from JS, url: ${url}`);
    return ngenxx_net_http_requestJ(params);
}

// Coding

function NGenXXCodingHexBytes2Str(bs) {
    var inJson = `{"inBytes":[${bs}], "inLen":${bs.length}}`;
    return ngenxx_coding_hex_bytes2strJ(inJson);
}

function NGenXXCodingHexStr2Bytes(s) {
    var inJson = `{"str":${s}}`;
    var outBytes = ngenxx_coding_hex_str2bytesJ(inJson);
    return JSON.parse(outBytes);
}

// Crypto

function NGenXXCryptoRand(n) {
    var inJson = `{"len":${n}}`;
    var outBytes = ngenxx_crypto_randJ(inJson);
    return JSON.parse(outBytes);
}

// Crypto.Hash

function NGenXXCryptoHashMD5(s) {
    var inBytes = str2bytes(s);
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    var outJson = ngenxx_crypto_hash_md5J(inJson);
    return JSON.parse(outJson);
}

function NGenXXCryptoHashSHA256(s) {
    var inBytes = str2bytes(s);
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    var outJson = ngenxx_crypto_hash_sha256J(inJson);
    return JSON.parse(outJson);
}

// Crypto.Base64

function NGenXXCryptoBase64Encode(s) {
    var inBytes = str2bytes(s);
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    var outJson = ngenxx_crypto_base64_encodeJ(inJson);
    var outBytes = JSON.parse(outJson);
    return bytes2str(outBytes);
}

function NGenXXCryptoBase64Decode(s) {
    var inBytes = str2bytes(s);
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    var outJson = ngenxx_crypto_base64_decodeJ(inJson);
    var outBytes = JSON.parse(outJson);
    return bytes2str(outBytes);
}
