function strArray2Json(strArray) {
    var json = '';
    strArray = strArray || [];
    json += '[';
    strArray.forEach((item, index, arr) => {
        json += `"${item}"`;
    });
    json += ']';
    return json;
}

// Utils

function NGenXXStr2Bytes(str) {
    return Array.from(str, char => char.charCodeAt(0))
}

function NGenXXBytes2Str(bytes) {
    return bytes.map((b) => {
        return String.fromCharCode(b);
    }).join("");
}

// Log

function NGenXXLogPrint(level, content) {
    var inJson = `{"level":${level},"content":"${content}"}`;
    ngenxx_log_printJ(inJson);
}

// Net.Http

function NGenXXNetHttpReq(url, params, method, headerArray, formFieldNameArray, formFieldMimeArray, formFieldDataArray, timeout) {
    params = params || ''
    headerArray = headerArray || []
    formFieldNameArray = formFieldNameArray || []
    formFieldMimeArray = formFieldMimeArray || []
    formFieldDataArray = formFieldDataArray || []
    headerArrayJson = strArray2Json(headerArray)
    formFieldNameArrayJson = strArray2Json(formFieldNameArray)
    formFieldMimeArrayJson = strArray2Json(formFieldMimeArray)
    formFieldDataArrayJson = strArray2Json(formFieldDataArray)
    var inJson = `{"url":"${url}", "params":"${params}", "method":${method}, "header_v":${headerArrayJson}, "header_c":${headerArray.length}, "form_field_name_v":${formFieldNameArrayJson}, "form_field_mimeme_v":${formFieldMimeArrayJson}, "form_field_data_v":${formFieldDataArrayJson}, "form_field_count":${formFieldNameArray.length}, "timeout":${timeout}}`;
    return ngenxx_net_http_requestJ(inJson);
}

// Coding

function NGenXXCodingHexBytes2Str(bytes) {
    var inJson = `{"inBytes":[${bytes}], "inLen":${bytes.length}}`;
    return ngenxx_coding_hex_bytes2strJ(inJson);
}

function NGenXXCodingHexStr2Bytes(hexStr) {
    var inJson = `{"str":${hexStr}}`;
    var outBytes = ngenxx_coding_hex_str2bytesJ(inJson);
    return JSON.parse(outBytes);
}

// Crypto

function NGenXXCryptoRand(len) {
    var inJson = `{"len":${len}}`;
    var outBytes = ngenxx_crypto_randJ(inJson);
    return JSON.parse(outBytes);
}

// Crypto.AES

function NGenXXCryptoAesEncrypt(str, key) {
    var inBytes = NGenXXStr2Bytes(str);
    var keyBytes = NGenXXStr2Bytes(key);
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}, "keyBytes":[${keyBytes}], "keyLen":${keyBytes.length}}`;
    var outJson = ngenxx_crypto_aes_encryptJ(inJson);
    var outBytes = JSON.parse(outJson);
    return NGenXXCodingHexBytes2Str(outBytes);
}

function NGenXXCryptoAesDecrypt(hexStr, key) {
    var inBytes = NGenXXCodingHexStr2Bytes(hexStr);
    var keyBytes = NGenXXStr2Bytes(key);
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}, "keyBytes":[${keyBytes}], "keyLen":${keyBytes.length}}`;
    var outJson = ngenxx_crypto_aes_decryptJ(inJson);
    var outBytes = JSON.parse(outJson);
    return NGenXXStr2Bytes(outBytes);
}

function NGenXXCryptoAesGcmEncrypt(str, key, iv, aad, tagBits) {
    var inBytes = NGenXXStr2Bytes(str);
    var keyBytes = NGenXXStr2Bytes(key);
    var ivBytes = NGenXXStr2Bytes(iv);
    var aadBytes = NGenXXStr2Bytes(aad) || [];
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}, "keyBytes":[${keyBytes}], "keyLen":${keyBytes.length}, "initVectorBytes":[${ivBytes}], "initVectorLen":${ivBytes.length}, "aadBytes":[${aadBytes}], "aadLen":${aadBytes.length}, "tagBits":${tagBits}}`;
    var outJson = ngenxx_crypto_aes_gcm_encryptJ(inJson);
    var outBytes = JSON.parse(outJson);
    return NGenXXCodingHexBytes2Str(outBytes);
}

function NGenXXCryptoAesGcmDecrypt(hexStr, key, iv, aad, tagBits) {
    var inBytes = NGenXXCodingHexStr2Bytes(hexStr);
    var keyBytes = NGenXXStr2Bytes(key);
    var ivBytes = NGenXXStr2Bytes(iv);
    var aadBytes = NGenXXStr2Bytes(aad) || [];
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}, "keyBytes":[${keyBytes}], "keyLen":${keyBytes.length}, "initVectorBytes":[${ivBytes}], "initVectorLen":${ivBytes.length}, "aadBytes":[${aadBytes}], "aadLen":${aadBytes.length}, "tagBits":${tagBits}}`;
    var outJson = ngenxx_crypto_aes_gcm_decryptJ(inJson);
    var outBytes = JSON.parse(outJson);
    return NGenXXStr2Bytes(outBytes);
}

// Crypto.Hash

function NGenXXCryptoHashMD5(str) {
    var inBytes = NGenXXStr2Bytes(str);
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    var outJson = ngenxx_crypto_hash_md5J(inJson);
    var outBytes = JSON.parse(outJson);
    return NGenXXCodingHexBytes2Str(outBytes);
}

function NGenXXCryptoHashSHA256(str) {
    var inBytes = NGenXXStr2Bytes(str);
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    var outJson = ngenxx_crypto_hash_sha256J(inJson);
    var outBytes = JSON.parse(outJson);
    return NGenXXCodingHexBytes2Str(outBytes);
}

// Crypto.Base64

function NGenXXCryptoBase64Encode(str) {
    var inBytes = NGenXXStr2Bytes(s);
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    var outJson = ngenxx_crypto_base64_encodeJ(inJson);
    var outBytes = JSON.parse(outJson);
    return NGenXXStr2Bytes(outBytes);
}

function NGenXXCryptoBase64Decode(str) {
    var inBytes = NGenXXStr2Bytes(str);
    var inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    var outJson = ngenxx_crypto_base64_decodeJ(inJson);
    var outBytes = JSON.parse(outJson);
    return NGenXXStr2Bytes(outBytes);
}
