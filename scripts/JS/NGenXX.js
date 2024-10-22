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

function _Map2StrArray(map) {
    var arr = [];
    map = map || new Map();
    map.forEach((v, k) => {
        arr.push(`${k}=${v}`);
    });
    return arr;
}

function _json2Array(json) {
    return JSON.parse(json || '[]');
}

function _buffer2Bytes(buffer) {
    var bytes = [];
    new Uint8Array(buffer).forEach((x) => {
        if (x > 0) bytes.push(x);
    });
    return bytes;
}

function _bytes2Buffer(bytes) {
    return new Uint8Array(bytes).buffer;
}

function _printArray(arr) {
    arr.map((x) => {
        NGenXXLogPrint(NGenXXLogLevel.Debug, `${x}`);
    });
}

function NGenXXGetVersion() {
    return ngenxx_get_versionJ();
}

function NGenXXRootPath() {
    return ngenxx_root_pathJ();
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
    content = content || '';
    let inJson = JSON.stringify({
        "level": level,
        "content": content
    });
    ngenxx_log_printJ(inJson);
}

// DeviceInfo

const NGenXXDeviceType = Object.freeze({
    Unknown: 0,
    Android: 1,
    ApplePhone: 2,
    ApplePad: 3,
    AppleMac: 4,
    AppleWatch: 5,
    AppleTV: 6,
    HarmonyOS: 7,
    Windows: 8,
    Linux: 9,
    Web: 10
});

const NGenXXDeviceCpuArch = Object.freeze({
    Unknown: 0,
    X86: 1,
    X86_64: 2,
    IA64: 3,
    ARM: 4,
    ARM_64: 5
});

function NGenXXDevicePlatform() {
    return ngenxx_device_typeJ();
}

function NGenXXDeviceName() {
    return ngenxx_device_nameJ();
}

function NGenXXDeviceManufacturer() {
    return ngenxx_device_manufacturerJ();
}

function NGenXXDeviceOSVersion() {
    return ngenxx_device_os_versionJ();
}

function NGenXXDeviceCPUArch() {
    return ngenxx_device_cpu_archJ();
}

// Net.Http

const NGenXXHttpMethod = Object.freeze({
    Get: 0,
    Post: 1,
    Put: 2
});

function NGenXXNetHttpRequest(url, paramMap, method, headerMap, formFieldNameArray, formFieldMimeArray, formFieldDataArray, timeout) {
    paramStr = _Map2UrlStr(paramMap);
    headerArray = _Map2StrArray(headerMap);
    formFieldNameArray = formFieldNameArray || [];
    formFieldMimeArray = formFieldMimeArray || [];
    formFieldDataArray = formFieldDataArray || [];

    let inJson = JSON.stringify({
        "url": url,
        "method": method,
        "params": paramStr,
        "header_v": headerArray,
        "header_c": headerArray.length,
        "form_field_name_v": formFieldNameArray,
        "form_field_mime_v": formFieldMimeArray,
        "form_field_data_v": formFieldDataArray,
        "form_field_count": formFieldNameArray.length,
        "timeout": timeout
    });

    return ngenxx_net_http_requestJ(inJson);
}

// Store.SQLite

function NGenXXStoreSQLiteOpen(_id) {
    let inJson = JSON.stringify({
        "_id": _id
    });
    return ngenxx_store_sqlite_openJ(inJson);
}

function NGenXXStoreSQLiteExecute(conn, sql) {
    let inJson = JSON.stringify({
        "conn": conn,
        "sql": sql
    });
    return ngenxx_store_sqlite_executeJ(inJson);
}

function NGenXXStoreSQLiteQueryDo(conn, sql) {
    let inJson = JSON.stringify({
        "conn": conn,
        "sql": sql
    });
    return ngenxx_store_sqlite_query_doJ(inJson);
}

function NGenXXStoreSQLiteQueryReadRow(query_result) {
    let inJson = JSON.stringify({
        "query_result": query_result
    });
    return ngenxx_store_sqlite_query_read_rowJ(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnText(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return ngenxx_store_sqlite_query_read_column_textJ(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnInteger(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return ngenxx_store_sqlite_query_read_column_integerJ(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnFloat(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return ngenxx_store_sqlite_query_read_column_floatJ(inJson);
}

function NGenXXStoreSQLiteQueryDrop(query_result) {
    let inJson = JSON.stringify({
        "query_result": query_result
    });
    ngenxx_store_sqlite_query_dropJ(inJson);
}

function NGenXXStoreSQLiteClose(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    ngenxx_store_sqlite_closeJ(inJson);
}

// Store.KV

function NGenXXStoreKVOpen(_id) {
    let inJson = JSON.stringify({
        "_id": _id
    });
    return ngenxx_store_kv_openJ(inJson);
}

function NGenXXStoreKVReadString(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_read_stringJ(inJson);
}

function NGenXXStoreKVWriteString(conn, k, s) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": s
    });
    return ngenxx_store_kv_write_stringJ(inJson);
}

function NGenXXStoreKVReadInteger(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_read_integerJ(inJson);
}

function NGenXXStoreKVWriteInteger(conn, k, i) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": i
    });
    return ngenxx_store_kv_write_integerJ(inJson);
}

function NGenXXStoreKVReadFloat(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_read_floatJ(inJson);
}

function NGenXXStoreKVWriteFloat(conn, k, f) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": f
    });
    return ngenxx_store_kv_write_floatJ(inJson);
}

function NGenXXStoreKVContains(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_containsJ(inJson);
}

function NGenXXStoreKVRemove(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    ngenxx_store_kv_removeJ(inJson);
}

function NGenXXStoreKVClear(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    ngenxx_store_kv_clearJ(inJson);
}

function NGenXXStoreKVClose(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    ngenxx_store_kv_closeJ(inJson);
}

// Coding

function NGenXXStr2Bytes(str) {
    return Array.from(str, char => char.charCodeAt(0))
}

function NGenXXBytes2Str(bytes) {
    return bytes.map((b) => {
        return b > 0 ? String.fromCharCode(b) : '';
    }).join('');
}

function NGenXXCodingHexBytes2Str(bytes) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "inBytes": bytes,
        "inLen": bytes.length
    });
    return ngenxx_coding_hex_bytes2strJ(inJson);
}

function NGenXXCodingHexStr2Bytes(hexStr) {
    hexStr = hexStr || '';
    let inJson = JSON.stringify({
        "str": hexStr
    });
    let outJson = ngenxx_coding_hex_str2bytesJ(inJson);
    return _json2Array(outJson);
}

// Crypto

function NGenXXCryptoRand(len) {
    let inJson = JSON.stringify({
        "len": len
    });
    let outJson = ngenxx_crypto_randJ(inJson);
    return _json2Array(outJson);
}

// Crypto.AES

function NGenXXCryptoAesEncrypt(inBytes, keyBytes) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length,
        "keyBytes": keyBytes,
        "keyLen": keyBytes.length
    });
    let outJson = ngenxx_crypto_aes_encryptJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesDecrypt(inBytes, keyBytes) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length,
        "keyBytes": keyBytes,
        "keyLen": keyBytes.length
    });
    let outJson = ngenxx_crypto_aes_decryptJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesGcmEncrypt(inBytes, keyBytes, ivBytes, aadBytes, tagBits) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    ivBytes = ivBytes || [];
    aadBytes = aadBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length,
        "keyBytes": keyBytes,
        "keyLen": keyBytes.length,
        "initVectorBytes": ivBytes,
        "initVectorLen": ivBytes.length,
        "aadBytes": aadBytes,
        "aadLen": aadBytes.length,
        "tagBits": tagBits
    });
    let outJson = ngenxx_crypto_aes_gcm_encryptJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesGcmDecrypt(inBytes, keyBytes, ivBytes, aadBytes, tagBits) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    ivBytes = ivBytes || [];
    aadBytes = aadBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length,
        "keyBytes": keyBytes,
        "keyLen": keyBytes.length,
        "initVectorBytes": ivBytes,
        "initVectorLen": ivBytes.length,
        "aadBytes": aadBytes,
        "aadLen": aadBytes.length,
        "tagBits": tagBits
    });
    let outJson = ngenxx_crypto_aes_gcm_decryptJ(inJson);
    return _json2Array(outJson);
}

// Crypto.Hash

function NGenXXCryptoHashMD5(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length
    });
    let outJson = ngenxx_crypto_hash_md5J(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoHashSHA256(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length
    });
    let outJson = ngenxx_crypto_hash_sha256J(inJson);
    return _json2Array(outJson);
}

// Crypto.Base64

function NGenXXCryptoBase64Encode(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length
    });
    let outJson = ngenxx_crypto_base64_encodeJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoBase64Decode(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length
    });
    let outJson = ngenxx_crypto_base64_decodeJ(inJson);
    return _json2Array(outJson);
}

// Z.Atomic

const NGenXXZZipMode = Object.freeze({
    Default: -1,
    PreferSpeed: 1,
    PreferSize: 9
});

const NGenXXZFormat = Object.freeze({
    ZLib: 0,
    GZip: 1,
    Raw: 2
});

function NGenXXZZipInit(mode, bufferSize, format) {
    let inJson = JSON.stringify({
        "mode": mode,
        "bufferSize": bufferSize,
        "format": format
    });
    return ngenxx_z_zip_initJ(inJson);
}

function NGenXXZZipInput(zip, bytes, finish) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "zip": zip,
        "inBytes": bytes,
        "inLen": bytes.length,
        "inFinish": finish ? 1 : 0
    });
    return ngenxx_z_zip_inputJ(inJson);
}

function NGenXXZZipProcessDo(zip) {
    let inJson = JSON.stringify({
        "zip": zip
    });
    let outJson = ngenxx_z_zip_process_doJ(inJson);
    return _json2Array(outJson);
}

function NGenXXZZipProcessFinished(zip) {
    let inJson = JSON.stringify({
        "zip": zip
    });
    return ngenxx_z_zip_process_finishedJ(inJson);
}

function NGenXXZZipRelease(zip) {
    let inJson = JSON.stringify({
        "zip": zip
    });
    ngenxx_z_zip_releaseJ(inJson);
}

function NGenXXZUnZipInit(bufferSize, format) {
    let inJson = JSON.stringify({
        "bufferSize": bufferSize,
        "format": format
    });
    return ngenxx_z_unzip_initJ(inJson);
}

function NGenXXZUnZipInput(unzip, bytes, finish) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "unzip": unzip,
        "inBytes": bytes,
        "inLen": bytes.length,
        "inFinish": finish ? 1 : 0
    });
    return ngenxx_z_unzip_inputJ(inJson);
}

function NGenXXZUnZipProcessDo(unzip) {
    let inJson = JSON.stringify({
        "unzip": unzip
    });
    let outJson = ngenxx_z_unzip_process_doJ(inJson);
    return _json2Array(outJson);
}

function NGenXXZUnZipProcessFinished(unzip) {
    let inJson = JSON.stringify({
        "unzip": unzip
    });
    return ngenxx_z_unzip_process_finishedJ(inJson);
}

function NGenXXZUnZipRelease(unzip) {
    let inJson = JSON.stringify({
        "unzip": unzip
    });
    ngenxx_z_unzip_releaseJ(inJson);
}

// Z.ByteArray

function NGenXXZZipBytes(mode, bufferSize, bytes, format) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "mode": mode,
        "bufferSize": bufferSize,
        "format": format,
        "inBytes": bytes,
        "inLen": bytes.length
    });
    let outJson = ngenxx_z_bytes_zipJ(inJson);
    return _json2Array(outJson);
}

function NGenXXZUnZipBytes(bufferSize, bytes, format) {
    let inJson = JSON.stringify({
        "bufferSize": bufferSize,
        "format": format,
        "inBytes": bytes,
        "inLen": bytes.length
    });
    let outJson = ngenxx_z_bytes_unzipJ(inJson);
    return _json2Array(outJson);
}

// Z.Stream

let NGenXXZBufferSize = 16 * 1024;

function NGenXXZStream(bufferSize, readFunc, writeFunc, flushFunc,
    z, inputFunc, processDoFunc, processFinishedFunc) {
    var inputFinished = false;
    var processFinished = false;
    do {
        let inBytes = readFunc();
        inputFinished = inBytes.length < bufferSize;
        NGenXXLogPrint(NGenXXLogLevel.Debug, `z <- len:${inBytes.length} finished:${inputFinished}`);

        let inputRet = inputFunc(z, inBytes, inputFinished);
        if (inputRet <= 0) {
            NGenXXLogPrint(NGenXXLogLevel.Error, 'z input failed!');
            return false;
        }

        processFinished = false;
        do {
            let outBytes = processDoFunc(z);
            if (outBytes.length === 0) {
                NGenXXLogPrint(NGenXXLogLevel.Error, 'z process failed!');
                return false;
            }
            processFinished = processFinishedFunc(z);
            NGenXXLogPrint(NGenXXLogLevel.Debug, `z -> len:${outBytes.length} finished:${processFinished}`);

            writeFunc(outBytes);
        } while (!processFinished);
    } while (!inputFinished);

    flushFunc();
    return true;
}

function NGenXXZZipStream(mode, bufferSize, format, readFunc, writeFunc, flushFunc) {
    let zip = NGenXXZZipInit(mode, bufferSize, format);

    let res = NGenXXZStream(bufferSize, readFunc, writeFunc, flushFunc, zip,
        (z, buffer, inputFinished) => {
            return NGenXXZZipInput(z, buffer, inputFinished);
        }, (z) => {
            return NGenXXZZipProcessDo(z);
        }, (z) => {
            return NGenXXZZipProcessFinished(z);
        });

    NGenXXZZipRelease(zip);
    return res;
}

function NGenXXZUnZipStream(bufferSize, format, readFunc, writeFunc, flushFunc) {
    let unzip = NGenXXZUnZipInit(bufferSize, format);

    let res = NGenXXZStream(bufferSize, readFunc, writeFunc, flushFunc, unzip,
        (z, buffer, inputFinished) => {
            return NGenXXZUnZipInput(z, buffer, inputFinished);
        }, (z) => {
            return NGenXXZUnZipProcessDo(z);
        }, (z) => {
            return NGenXXZUnZipProcessFinished(z);
        });

    NGenXXZUnZipRelease(unzip);
    return res;
}

// Z.file

function NGenXXZZipFile(mode, format, inFilePath, outFilePath) {
    let bufferSize = NGenXXZBufferSize;

    let inF = std.open(inFilePath, 'r');
    let outF = std.open(outFilePath, 'w');
    
    let res = NGenXXZZipStream(mode, bufferSize, format,
        () => {
            let inBuffer = new ArrayBuffer(bufferSize);
            inF.read(inBuffer, 0, bufferSize);
            return _buffer2Bytes(inBuffer);
        },
        (bytes) => {
            let outBuffer = _bytes2Buffer(bytes);
            outF.write(outBuffer, 0, bytes.length);
        },
        () => {
            outF.flush();
        }
    );

    outF.close();
    inF.close();
    return res;
}

function NGenXXZUnZipFile(format, inFilePath, outFilePath) {
    let bufferSize = NGenXXZBufferSize;

    let inF = std.open(inFilePath, 'r');
    let outF = std.open(outFilePath, 'w');

    let res = NGenXXZUnZipStream(bufferSize, format,
        () => {
            let inBuffer = new ArrayBuffer(bufferSize);
            inF.read(inBuffer, 0, bufferSize);
            return _buffer2Bytes(inBuffer);
        },
        (bytes) => {
            let outBuffer = _bytes2Buffer(bytes);
            outF.write(outBuffer, 0, bytes.length);
        },
        () => {
            outF.flush();
        }
    );

    outF.close();
    inF.close();
    return res;
}
