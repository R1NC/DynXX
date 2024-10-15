function _Array2Str(array) {
    var s = '';
    array = array || [];
    array.forEach((item, index, arr) => {
        s += `${item}`;
    });
    return s;
}

function _Map2UrlStr(map) {
    map = map || new Map();
    var s = '';
    map.forEach((v, k) => {
        s += s.length === 0 ? '?' : '&';
        s += `${k}=${v}`;
    });
    return s;
}

function _Map2StrArrayJson(map) {
    var arr = [];
    map = map || new Map();
    map.forEach((v, k) => {
        arr.push(`"${k}=${v}"`);
    });
    return arr;
}

function _json2Array(json) {
    return JSON.parse(json || `[]`);
}

function _printArray(arr) {
    arr.map((x) => {
        NGenXXLogPrint(NGenXXLogLevel.Debug, `${x}`);
    });
}

// Utils

function NGenXXStr2Bytes(str) {
    return Array.from(str, char => char.charCodeAt(0))
}

function NGenXXBytes2Str(bytes) {
    return bytes.map((b) => {
        return b > 0 ? String.fromCharCode(b) : '';
    }).join('');
}

// Log

const NGenXXLogLevel = Object.freeze({
    Info: 0,
    Debug: 1,
    Warn: 2,
    Error: 3,
    None: 4
});

function NGenXXLogPrint(level, content) {
    content = content || ``;
    let inJson = `{"level":${level}, "content":"${content}"}`;
    ngenxx_log_printJ(inJson);
}

// Net.Http

const NGenXXHttpMethod = Object.freeze({
    Get: 0,
    Post: 1,
    Put: 2
});

function NGenXXNetHttpRequest(url, paramMap, method, headerMap, formFieldNameArray, formFieldMimeArray, formFieldDataArray, timeout) {
    paramStr = _Map2UrlStr(paramMap);
    headerMap = headerMap || new Map();
    headerArrayStr = _Map2StrArrayJson(headerMap);
    
    formFieldNameArray = formFieldNameArray || [];
    formFieldMimeArray = formFieldMimeArray || [];
    formFieldDataArray = formFieldDataArray || [];
    formFieldNameArrayStr = _Array2Str(formFieldNameArray);
    formFieldMimeArrayStr = _Array2Str(formFieldMimeArray);
    formFieldDataArrayStr = _Array2Str(formFieldDataArray);
    
    let inJson = `{"url":"${url}", "params":"${paramStr}", "method":${method}, "header_v":[${headerArrayStr}], "header_c":${headerMap.size}, "form_field_name_v":[${formFieldNameArrayStr}], "form_field_mimeme_v":[${formFieldMimeArrayStr}], "form_field_data_v":[${formFieldDataArrayStr}], "form_field_count":${formFieldNameArray.length}, "timeout":${timeout}}`;
    
    console.log(inJson);
    
    return ngenxx_net_http_requestJ(inJson);
}

// Store.SQLite

function NGenXXStoreSQLiteOpen(_id) {
    let inJson = `{"_id":"${_id}"}`;
    return ngenxx_store_sqlite_openJ(inJson);
}

function NGenXXStoreSQLiteExecute(conn, sql) {
    let inJson = `{"conn":${conn}, "sql":"${sql}"}`;
    return ngenxx_store_sqlite_executeJ(inJson);
}

function NGenXXStoreSQLiteQueryDo(conn, sql) {
    let inJson = `{"conn":${conn}, "sql":"${sql}"}`;
    return ngenxx_store_sqlite_query_doJ(inJson);
}

function NGenXXStoreSQLiteQueryReadRow(query_result) {
    let inJson = `{"query_result":${query_result}}`;
    return ngenxx_store_sqlite_query_read_rowJ(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnText(query_result, column) {
    let inJson = `{"query_result":${query_result}, "column":"${column}"}`;
    return ngenxx_store_sqlite_query_read_column_textJ(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnInteger(query_result, column) {
    let inJson = `{"query_result":${query_result}, "column":"${column}"}`;
    return ngenxx_store_sqlite_query_read_column_integerJ(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnFloat(query_result, column) {
    let inJson = `{"query_result":${query_result}, "column":"${column}"}`;
    return ngenxx_store_sqlite_query_read_column_floatJ(inJson);
}

function NGenXXStoreSQLiteQueryDrop(query_result) {
    let inJson = `{"query_result":${query_result}}`;
    ngenxx_store_sqlite_query_dropJ(inJson);
}

function NGenXXStoreSQLiteClose(conn) {
    let inJson = `{"conn":${conn}}`;
    ngenxx_store_sqlite_closeJ(inJson);
}

// Store.KV

function NGenXXStoreKVOpen(_id) {
    let inJson = `{"_id":"${_id}"}`;
    return ngenxx_store_kv_openJ(inJson);
}

function NGenXXStoreKVReadString(conn, k) {
    let inJson = `{"conn":${conn}, "k":"${k}"}`;
    return ngenxx_store_kv_read_stringJ(inJson);
}

function NGenXXStoreKVWriteString(conn, k, s) {
    let inJson = `{"conn":${conn}, "k":"${k}", "v":"${s}"}`;
    return ngenxx_store_kv_write_stringJ(inJson);
}

function NGenXXStoreKVReadInteger(conn, k) {
    let inJson = `{"conn":${conn}, "k":"${k}"}`;
    return ngenxx_store_kv_read_integerJ(inJson);
}

function NGenXXStoreKVWriteInteger(conn, k, i) {
    let inJson = `{"conn":${conn}, "k":"${k}", "v":${i}}`;
    return ngenxx_store_kv_write_integerJ(inJson);
}

function NGenXXStoreKVReadFloat(conn, k) {
    let inJson = `{"conn":${conn}, "k":"${k}"}`;
    return ngenxx_store_kv_read_floatJ(inJson);
}

function NGenXXStoreKVWriteFloat(conn, k, f) {
    let inJson = `{"conn":${conn}, "k":"${k}", "v":${f}}`;
    return ngenxx_store_kv_write_floatJ(inJson);
}

function NGenXXStoreKVContains(conn, k) {
    let inJson = `{"conn":${conn}, "k":"${k}"}`;
    return ngenxx_store_kv_containsJ(inJson);
}

function NGenXXStoreKVRemove(conn, k) {
    let inJson = `{"conn":${conn}, "k":"${k}"}`;
    ngenxx_store_kv_removeJ(inJson);
}

function NGenXXStoreKVClear(conn) {
    let inJson = `{"conn":${conn}}`;
    ngenxx_store_kv_clearJ(inJson);
}

function NGenXXStoreKVClose(conn) {
    let inJson = `{"conn":${conn}}`;
    ngenxx_store_kv_closeJ(inJson);
}

// Coding

function NGenXXCodingHexBytes2Str(bytes) {
    let inJson = `{"inBytes":[${bytes}], "inLen":${bytes.length}}`;
    return ngenxx_coding_hex_bytes2strJ(inJson);
}

function NGenXXCodingHexStr2Bytes(hexStr) {
    let inJson = `{"str":${hexStr}}`;
    let outJson = ngenxx_coding_hex_str2bytesJ(inJson);
    return _json2Array(outJson);
}

// Crypto

function NGenXXCryptoRand(len) {
    let inJson = `{"len":${len}}`;
    let outJson = ngenxx_crypto_randJ(inJson);
    return _json2Array(outJson);
}

// Crypto.AES

function NGenXXCryptoAesEncrypt(inBytes, keyBytes) {
    let inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}, "keyBytes":[${keyBytes}], "keyLen":${keyBytes.length}}`;
    let outJson = ngenxx_crypto_aes_encryptJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesDecrypt(inBytes, keyBytes) {
    let inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}, "keyBytes":[${keyBytes}], "keyLen":${keyBytes.length}}`;
    let outJson = ngenxx_crypto_aes_decryptJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesGcmEncrypt(inBytes, keyBytes, ivBytes, aadBytes, tagBits) {
    aadBytes = aadBytes || [];
    let inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}, "keyBytes":[${keyBytes}], "keyLen":${keyBytes.length}, "initVectorBytes":[${ivBytes}], "initVectorLen":${ivBytes.length}, "aadBytes":[${aadBytes}], "aadLen":${aadBytes.length}, "tagBits":${tagBits}}`;
    let outJson = ngenxx_crypto_aes_gcm_encryptJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesGcmDecrypt(inBytes, keyBytes, ivBytes, aadBytes, tagBits) {
    aadBytes = aadBytes || [];
    let inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}, "keyBytes":[${keyBytes}], "keyLen":${keyBytes.length}, "initVectorBytes":[${ivBytes}], "initVectorLen":${ivBytes.length}, "aadBytes":[${aadBytes}], "aadLen":${aadBytes.length}, "tagBits":${tagBits}}`;
    let outJson = ngenxx_crypto_aes_gcm_decryptJ(inJson);
    return _json2Array(outJson);
}

// Crypto.Hash

function NGenXXCryptoHashMD5(inBytes) {
    let inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    let outJson = ngenxx_crypto_hash_md5J(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoHashSHA256(inBytes) {
    let inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    let outJson = ngenxx_crypto_hash_sha256J(inJson);
    return _json2Array(outJson);
}

// Crypto.Base64

function NGenXXCryptoBase64Encode(inBytes) {
    let inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    let outJson = ngenxx_crypto_base64_encodeJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoBase64Decode(inBytes) {
    let inJson = `{"inBytes":[${inBytes}], "inLen":${inBytes.length}}`;
    let outJson = ngenxx_crypto_base64_decodeJ(inJson);
    return _json2Array(outJson);
}
