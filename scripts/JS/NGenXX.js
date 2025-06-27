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
    return ngenxx_get_version();
}

function NGenXXRootPath() {
    return ngenxx_root_path();
}

function NGenXXCallPlatform(msg) {
    msg = msg || '';
    return ngenxx_call_platform(msg);
}

/*const NGenXXLogLevel = Object.freeze({
    Debug: 3,
    Info: 4,
    Warn: 5,
    Error: 6,
    Fatal: 7,
    None: 8
});*/

function NGenXXLogPrint(level, content) {
    content = content || '';
    let inJson = JSON.stringify({
        "level": level,
        "content": content
    });
    ngenxx_log_print(inJson);
}

function NGenXXDevicePlatform() {
    return ngenxx_device_type();
}

function NGenXXDeviceName() {
    return ngenxx_device_name();
}

function NGenXXDeviceManufacturer() {
    return ngenxx_device_manufacturer();
}

function NGenXXDeviceOSVersion() {
    return ngenxx_device_os_version();
}

function NGenXXDeviceCPUArch() {
    return ngenxx_device_cpu_arch();
}

function NGenXXNetHttpRequest(url, method, paramMap,  headerMap, rawBodyBytes, formFieldNameArray, formFieldMimeArray, formFieldDataArray, timeout) {
    paramStr = _Map2UrlStr(paramMap);
    headerArray = _Map2StrArray(headerMap);
    rawBodyBytes = rawBodyBytes || [];
    formFieldNameArray = formFieldNameArray || [];
    formFieldMimeArray = formFieldMimeArray || [];
    formFieldDataArray = formFieldDataArray || [];
    timeout = timeout || 15000;

    let inJson = JSON.stringify({
        "url": url,
        "method": method,
        "params": paramStr,
        "header_v": headerArray,
        "rawBodyBytes": rawBodyBytes,
        "form_field_name_v": formFieldNameArray,
        "form_field_mime_v": formFieldMimeArray,
        "form_field_data_v": formFieldDataArray,
        "timeout": timeout
    });

    return ngenxx_net_http_request(inJson);
}

function NGenXXNetHttpDownload(url, file, timeout) {
    timeout = timeout || 15000;
    let inJson = JSON.stringify({
        "url": url,
        "file": file,
        "timeout": timeout
    });

    return ngenxx_net_http_download(inJson);
}

function NGenXXStoreSQLiteOpen(_id) {
    let inJson = JSON.stringify({
        "_id": _id
    });
    return ngenxx_store_sqlite_open(inJson);
}

function NGenXXStoreSQLiteExecute(conn, sql) {
    let inJson = JSON.stringify({
        "conn": conn,
        "sql": sql
    });
    return ngenxx_store_sqlite_execute(inJson);
}

function NGenXXStoreSQLiteQueryDo(conn, sql) {
    let inJson = JSON.stringify({
        "conn": conn,
        "sql": sql
    });
    return ngenxx_store_sqlite_query_do(inJson);
}

function NGenXXStoreSQLiteQueryReadRow(query_result) {
    let inJson = JSON.stringify({
        "query_result": query_result
    });
    return ngenxx_store_sqlite_query_read_row(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnText(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return ngenxx_store_sqlite_query_read_column_text(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnInteger(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return ngenxx_store_sqlite_query_read_column_integer(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnFloat(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return ngenxx_store_sqlite_query_read_column_float(inJson);
}

function NGenXXStoreSQLiteQueryDrop(query_result) {
    let inJson = JSON.stringify({
        "query_result": query_result
    });
    ngenxx_store_sqlite_query_drop(inJson);
}

function NGenXXStoreSQLiteClose(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    ngenxx_store_sqlite_close(inJson);
}

function NGenXXStoreKVOpen(_id) {
    let inJson = JSON.stringify({
        "_id": _id
    });
    return ngenxx_store_kv_open(inJson);
}

function NGenXXStoreKVReadString(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_read_string(inJson);
}

function NGenXXStoreKVWriteString(conn, k, s) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": s
    });
    return ngenxx_store_kv_write_string(inJson);
}

function NGenXXStoreKVReadInteger(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_read_integer(inJson);
}

function NGenXXStoreKVWriteInteger(conn, k, i) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": i
    });
    return ngenxx_store_kv_write_integer(inJson);
}

function NGenXXStoreKVReadFloat(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_read_float(inJson);
}

function NGenXXStoreKVWriteFloat(conn, k, f) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": f
    });
    return ngenxx_store_kv_write_float(inJson);
}

function NGenXXStoreKVAllKeys(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    let outJson = ngenxx_store_kv_all_keys(inJson);
    return _json2Array(outJson);
}

function NGenXXStoreKVContains(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_contains(inJson);
}

function NGenXXStoreKVRemove(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    ngenxx_store_kv_remove(inJson);
}

function NGenXXStoreKVClear(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    ngenxx_store_kv_clear(inJson);
}

function NGenXXStoreKVClose(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    ngenxx_store_kv_close(inJson);
}

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
        "inBytes": bytes
    });
    return ngenxx_coding_hex_bytes2str(inJson);
}

function NGenXXCodingHexStr2Bytes(str) {
    str = str || '';
    let inJson = JSON.stringify({
        "str": str
    });
    let outJson = ngenxx_coding_hex_str2bytes(inJson);
    return _json2Array(outJson);
}

function NGenXXCodingBytes2Str(bytes) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "inBytes": bytes
    });
    return ngenxx_coding_bytes2str(inJson);
}

function NGenXXCodingStr2Bytes(str) {
    str = str || '';
    let inJson = JSON.stringify({
        "str": str
    });
    let outJson = ngenxx_coding_str2bytes(inJson);
    return _json2Array(outJson);
}

function NGenXXCodingCaseUpper(str) {
    str = str || '';
    let inJson = JSON.stringify({
        "str": str
    });
    return ngenxx_coding_case_upper(inJson);
}

function NGenXXCodingCaseLower(str) {
    str = str || '';
    let inJson = JSON.stringify({
        "str": str
    });
    return ngenxx_coding_case_lower(inJson);
}

function NGenXXCryptoRand(len) {
    let inJson = JSON.stringify({
        "len": len
    });
    let outJson = ngenxx_crypto_rand(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesEncrypt(inBytes, keyBytes) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "keyBytes": keyBytes
    });
    let outJson = ngenxx_crypto_aes_encrypt(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesDecrypt(inBytes, keyBytes) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "keyBytes": keyBytes
    });
    let outJson = ngenxx_crypto_aes_decrypt(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesGcmEncrypt(inBytes, keyBytes, ivBytes, tagBits, aadBytes) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    ivBytes = ivBytes || [];
    aadBytes = aadBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "keyBytes": keyBytes,
        "initVectorBytes": ivBytes,
        "aadBytes": aadBytes,
        "tagBits": tagBits
    });
    let outJson = ngenxx_crypto_aes_gcm_encrypt(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesGcmDecrypt(inBytes, keyBytes, ivBytes, tagBits, aadBytes) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    ivBytes = ivBytes || [];
    aadBytes = aadBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "keyBytes": keyBytes,
        "initVectorBytes": ivBytes,
        "aadBytes": aadBytes,
        "tagBits": tagBits
    });
    let outJson = ngenxx_crypto_aes_gcm_decrypt(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoRsaGenKey(base64, isPublic) {
    let inJson = JSON.stringify({
        "base64": base64,
        "isPublic": isPublic
    });
    let outJson = ngenxx_crypto_rsa_gen_key(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoRsaEncrypt(inBytes, keyBytes) {
    let inJson = JSON.stringify({
        "inBytes": inBytes || [],
        "keyBytes": keyBytes || []
    });
    let outJson = ngenxx_crypto_rsa_encrypt(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoRsaDecrypt(inBytes, keyBytes) {
    let inJson = JSON.stringify({
        "inBytes": inBytes || [],
        "keyBytes": keyBytes || []
    });
    let outJson = ngenxx_crypto_rsa_decrypt(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoHashMD5(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes
    });
    let outJson = ngenxx_crypto_hash_md5(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoHashSHA256(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes
    });
    let outJson = ngenxx_crypto_hash_sha256(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoBase64Encode(inBytes, noNewLines) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "noNewLines": noNewLines
    });
    let outJson = ngenxx_crypto_base64_encode(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoBase64Decode(inBytes, noNewLines) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "noNewLines": noNewLines
    });
    let outJson = ngenxx_crypto_base64_decode(inJson);
    return _json2Array(outJson);
}

//let NGenXXZBufferSize = 16 * 1024

function _NGenXXZZipInit(mode, bufferSize, format) {
    let inJson = JSON.stringify({
        "mode": mode,
        "bufferSize": bufferSize,
        "format": format
    });
    return ngenxx_z_zip_init(inJson);
}

function _NGenXXZZipInput(zip, bytes, finish) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "zip": zip,
        "inBytes": bytes,
        "inFinish": finish ? 1 : 0
    });
    return ngenxx_z_zip_input(inJson);
}

function _NGenXXZZipProcessDo(zip) {
    let inJson = JSON.stringify({
        "zip": zip
    });
    let outJson = ngenxx_z_zip_process_do(inJson);
    return _json2Array(outJson);
}

function _NGenXXZZipProcessFinished(zip) {
    let inJson = JSON.stringify({
        "zip": zip
    });
    return ngenxx_z_zip_process_finished(inJson);
}

function _NGenXXZZipRelease(zip) {
    let inJson = JSON.stringify({
        "zip": zip
    });
    ngenxx_z_zip_release(inJson);
}

function _NGenXXZUnZipInit(bufferSize, format) {
    let inJson = JSON.stringify({
        "bufferSize": bufferSize,
        "format": format
    });
    return ngenxx_z_unzip_init(inJson);
}

function _NGenXXZUnZipInput(unzip, bytes, finish) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "unzip": unzip,
        "inBytes": bytes,
        "inFinish": finish ? 1 : 0
    });
    return ngenxx_z_unzip_input(inJson);
}

function _NGenXXZUnZipProcessDo(unzip) {
    let inJson = JSON.stringify({
        "unzip": unzip
    });
    let outJson = ngenxx_z_unzip_process_do(inJson);
    return _json2Array(outJson);
}

function _NGenXXZUnZipProcessFinished(unzip) {
    let inJson = JSON.stringify({
        "unzip": unzip
    });
    return ngenxx_z_unzip_process_finished(inJson);
}

function _NGenXXZUnZipRelease(unzip) {
    let inJson = JSON.stringify({
        "unzip": unzip
    });
    ngenxx_z_unzip_release(inJson);
}

function NGenXXZZipBytes(bytes, mode, bufferSize, format) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "mode": mode,
        "bufferSize": bufferSize,
        "format": format,
        "inBytes": bytes
    });
    return ngenxx_z_bytes_zip(inJson);
}

function NGenXXZUnZipBytes(bytes, bufferSize, format) {
    let inJson = JSON.stringify({
        "bufferSize": bufferSize,
        "format": format,
        "inBytes": bytes
    });
    return ngenxx_z_bytes_unzip(inJson);
}

function _NGenXXZStream(bufferSize, readFunc, writeFunc, flushFunc,
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

function NGenXXZZipStream(readFunc, writeFunc, flushFunc, mode, bufferSize, format) {
    let zip = _NGenXXZZipInit(mode, bufferSize, format);

    let res = _NGenXXZStream(bufferSize, readFunc, writeFunc, flushFunc, zip,
        (z, buffer, inputFinished) => {
            return _NGenXXZZipInput(z, buffer, inputFinished);
        }, (z) => {
            return _NGenXXZZipProcessDo(z);
        }, (z) => {
            return _NGenXXZZipProcessFinished(z);
        });

    _NGenXXZZipRelease(zip);
    return res;
}

function NGenXXZUnZipStream(readFunc, writeFunc, flushFunc, bufferSize, format) {
    let unzip = _NGenXXZUnZipInit(bufferSize, format);

    let res = _NGenXXZStream(bufferSize, readFunc, writeFunc, flushFunc, unzip,
        (z, buffer, inputFinished) => {
            return _NGenXXZUnZipInput(z, buffer, inputFinished);
        }, (z) => {
            return _NGenXXZUnZipProcessDo(z);
        }, (z) => {
            return _NGenXXZUnZipProcessFinished(z);
        });

    _NGenXXZUnZipRelease(unzip);
    return res;
}

function NGenXXZZipFile(inFilePath, outFilePath, mode, bufferSize, format) {
    let inF = std.open(inFilePath, 'r');
    let outF = std.open(outFilePath, 'w');

    let res = NGenXXZZipStream(
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
        },
        mode, 
        bufferSize, 
        format
    );

    outF.close();
    inF.close();
    return res;
}

function NGenXXZUnZipFile(inFilePath, outFilePath, bufferSize, format) {
    let inF = std.open(inFilePath, 'r');
    let outF = std.open(outFilePath, 'w');

    let res = NGenXXZUnZipStream(
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
        },
        bufferSize, 
        format
    );

    outF.close();
    inF.close();
    return res;
}
