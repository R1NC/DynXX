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
        DynXXLogPrint(DynXXLogLevel.Debug, `${x}`);
    });
}

function DynXXGetVersion() {
    return dynxx_get_version();
}

function DynXXRootPath() {
    return dynxx_root_path();
}

function DynXXCallPlatform(msg) {
    msg = msg || '';
    return dynxx_call_platform(msg);
}

/*const DynXXLogLevel = Object.freeze({
    Debug: 3,
    Info: 4,
    Warn: 5,
    Error: 6,
    Fatal: 7,
    None: 8
});*/

function DynXXLogPrint(level, content) {
    content = content || '';
    let inJson = JSON.stringify({
        "level": level,
        "content": content
    });
    dynxx_log_print(inJson);
}

function DynXXDevicePlatform() {
    return dynxx_device_type();
}

function DynXXDeviceName() {
    return dynxx_device_name();
}

function DynXXDeviceManufacturer() {
    return dynxx_device_manufacturer();
}

function DynXXDeviceOSVersion() {
    return dynxx_device_os_version();
}

function DynXXDeviceCPUArch() {
    return dynxx_device_cpu_arch();
}

function DynXXNetHttpRequest(url, method, paramMap,  headerMap, rawBodyBytes, formFieldNameArray, formFieldMimeArray, formFieldDataArray, timeout) {
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

    return dynxx_net_http_request(inJson);
}

function DynXXNetHttpDownload(url, file, timeout) {
    timeout = timeout || 15000;
    let inJson = JSON.stringify({
        "url": url,
        "file": file,
        "timeout": timeout
    });

    return dynxx_net_http_download(inJson);
}

function DynXXSQLiteOpen(_id) {
    let inJson = JSON.stringify({
        "_id": _id
    });
    return dynxx_sqlite_open(inJson);
}

function DynXXSQLiteExecute(conn, sql) {
    let inJson = JSON.stringify({
        "conn": conn,
        "sql": sql
    });
    return dynxx_sqlite_execute(inJson);
}

function DynXXSQLiteQueryDo(conn, sql) {
    let inJson = JSON.stringify({
        "conn": conn,
        "sql": sql
    });
    return dynxx_sqlite_query_do(inJson);
}

function DynXXSQLiteQueryReadRow(query_result) {
    let inJson = JSON.stringify({
        "query_result": query_result
    });
    return dynxx_sqlite_query_read_row(inJson);
}

function DynXXSQLiteQueryReadColumnText(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return dynxx_sqlite_query_read_column_text(inJson);
}

function DynXXSQLiteQueryReadColumnInteger(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return dynxx_sqlite_query_read_column_integer(inJson);
}

function DynXXSQLiteQueryReadColumnFloat(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return dynxx_sqlite_query_read_column_float(inJson);
}

function DynXXSQLiteQueryDrop(query_result) {
    let inJson = JSON.stringify({
        "query_result": query_result
    });
    dynxx_sqlite_query_drop(inJson);
}

function DynXXSQLiteClose(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    dynxx_sqlite_close(inJson);
}

function DynXXKVOpen(_id) {
    let inJson = JSON.stringify({
        "_id": _id
    });
    return dynxx_kv_open(inJson);
}

function DynXXKVReadString(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return dynxx_kv_read_string(inJson);
}

function DynXXKVWriteString(conn, k, s) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": s
    });
    return dynxx_kv_write_string(inJson);
}

function DynXXKVReadInteger(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return dynxx_kv_read_integer(inJson);
}

function DynXXKVWriteInteger(conn, k, i) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": i
    });
    return dynxx_kv_write_integer(inJson);
}

function DynXXKVReadFloat(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return dynxx_kv_read_float(inJson);
}

function DynXXKVWriteFloat(conn, k, f) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": f
    });
    return dynxx_kv_write_float(inJson);
}

function DynXXKVAllKeys(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    let outJson = dynxx_kv_all_keys(inJson);
    return _json2Array(outJson);
}

function DynXXKVContains(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return dynxx_kv_contains(inJson);
}

function DynXXKVRemove(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    dynxx_kv_remove(inJson);
}

function DynXXKVClear(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    dynxx_kv_clear(inJson);
}

function DynXXKVClose(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    dynxx_kv_close(inJson);
}

function DynXXStr2Bytes(str) {
    return Array.from(str, char => char.charCodeAt(0))
}

function DynXXBytes2Str(bytes) {
    return bytes.map((b) => {
        return b > 0 ? String.fromCharCode(b) : '';
    }).join('');
}

function DynXXCodingHexBytes2Str(bytes) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "inBytes": bytes
    });
    return dynxx_coding_hex_bytes2str(inJson);
}

function DynXXCodingHexStr2Bytes(str) {
    str = str || '';
    let inJson = JSON.stringify({
        "str": str
    });
    let outJson = dynxx_coding_hex_str2bytes(inJson);
    return _json2Array(outJson);
}

function DynXXCodingBytes2Str(bytes) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "inBytes": bytes
    });
    return dynxx_coding_bytes2str(inJson);
}

function DynXXCodingStr2Bytes(str) {
    str = str || '';
    let inJson = JSON.stringify({
        "str": str
    });
    let outJson = dynxx_coding_str2bytes(inJson);
    return _json2Array(outJson);
}

function DynXXCodingCaseUpper(str) {
    str = str || '';
    let inJson = JSON.stringify({
        "str": str
    });
    return dynxx_coding_case_upper(inJson);
}

function DynXXCodingCaseLower(str) {
    str = str || '';
    let inJson = JSON.stringify({
        "str": str
    });
    return dynxx_coding_case_lower(inJson);
}

function DynXXCryptoRand(len) {
    let inJson = JSON.stringify({
        "len": len
    });
    let outJson = dynxx_crypto_rand(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoAesEncrypt(inBytes, keyBytes) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "keyBytes": keyBytes
    });
    let outJson = dynxx_crypto_aes_encrypt(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoAesDecrypt(inBytes, keyBytes) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "keyBytes": keyBytes
    });
    let outJson = dynxx_crypto_aes_decrypt(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoAesGcmEncrypt(inBytes, keyBytes, ivBytes, tagBits, aadBytes) {
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
    let outJson = dynxx_crypto_aes_gcm_encrypt(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoAesGcmDecrypt(inBytes, keyBytes, ivBytes, tagBits, aadBytes) {
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
    let outJson = dynxx_crypto_aes_gcm_decrypt(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoRsaGenKey(base64, isPublic) {
    let inJson = JSON.stringify({
        "base64": base64,
        "isPublic": isPublic
    });
    let outJson = dynxx_crypto_rsa_gen_key(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoRsaEncrypt(inBytes, keyBytes) {
    let inJson = JSON.stringify({
        "inBytes": inBytes || [],
        "keyBytes": keyBytes || []
    });
    let outJson = dynxx_crypto_rsa_encrypt(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoRsaDecrypt(inBytes, keyBytes) {
    let inJson = JSON.stringify({
        "inBytes": inBytes || [],
        "keyBytes": keyBytes || []
    });
    let outJson = dynxx_crypto_rsa_decrypt(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoHashMD5(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes
    });
    let outJson = dynxx_crypto_hash_md5(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoHashSHA1(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes
    });
    let outJson = dynxx_crypto_hash_sha1(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoHashSHA256(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes
    });
    let outJson = dynxx_crypto_hash_sha256(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoBase64Encode(inBytes, noNewLines) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "noNewLines": (noNewLines === undefined || noNewLines === 1 || noNewLines === true) ? 1 : 0
    });
    let outJson = dynxx_crypto_base64_encode(inJson);
    return _json2Array(outJson);
}

function DynXXCryptoBase64Decode(inBytes, noNewLines) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "noNewLines": (noNewLines === undefined || noNewLines === 1 || noNewLines === true) ? 1 : 0
    });
    let outJson = dynxx_crypto_base64_decode(inJson);
    return _json2Array(outJson);
}

//let DynXXZBufferSize = 16 * 1024

function _DynXXZZipInit(mode, bufferSize, format) {
    let inJson = JSON.stringify({
        "mode": mode,
        "bufferSize": bufferSize,
        "format": format
    });
    return dynxx_z_zip_init(inJson);
}

function _DynXXZZipInput(zip, bytes, finish) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "zip": zip,
        "inBytes": bytes,
        "inFinish": finish ? 1 : 0
    });
    return dynxx_z_zip_input(inJson);
}

function _DynXXZZipProcessDo(zip) {
    let inJson = JSON.stringify({
        "zip": zip
    });
    let outJson = dynxx_z_zip_process_do(inJson);
    return _json2Array(outJson);
}

function _DynXXZZipProcessFinished(zip) {
    let inJson = JSON.stringify({
        "zip": zip
    });
    return dynxx_z_zip_process_finished(inJson);
}

function _DynXXZZipRelease(zip) {
    let inJson = JSON.stringify({
        "zip": zip
    });
    dynxx_z_zip_release(inJson);
}

function _DynXXZUnZipInit(bufferSize, format) {
    let inJson = JSON.stringify({
        "bufferSize": bufferSize,
        "format": format
    });
    return dynxx_z_unzip_init(inJson);
}

function _DynXXZUnZipInput(unzip, bytes, finish) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "unzip": unzip,
        "inBytes": bytes,
        "inFinish": finish ? 1 : 0
    });
    return dynxx_z_unzip_input(inJson);
}

function _DynXXZUnZipProcessDo(unzip) {
    let inJson = JSON.stringify({
        "unzip": unzip
    });
    let outJson = dynxx_z_unzip_process_do(inJson);
    return _json2Array(outJson);
}

function _DynXXZUnZipProcessFinished(unzip) {
    let inJson = JSON.stringify({
        "unzip": unzip
    });
    return dynxx_z_unzip_process_finished(inJson);
}

function _DynXXZUnZipRelease(unzip) {
    let inJson = JSON.stringify({
        "unzip": unzip
    });
    dynxx_z_unzip_release(inJson);
}

function DynXXZZipBytes(bytes, mode, bufferSize, format) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "mode": mode,
        "bufferSize": bufferSize,
        "format": format,
        "inBytes": bytes
    });
    return dynxx_z_bytes_zip(inJson);
}

function DynXXZUnZipBytes(bytes, bufferSize, format) {
    let inJson = JSON.stringify({
        "bufferSize": bufferSize,
        "format": format,
        "inBytes": bytes
    });
    return dynxx_z_bytes_unzip(inJson);
}

function _DynXXZStream(bufferSize, readFunc, writeFunc, flushFunc,
    z, inputFunc, processDoFunc, processFinishedFunc) {
    var inputFinished = false;
    var processFinished = false;
    do {
        let inBytes = readFunc();
        inputFinished = inBytes.length < bufferSize;
        DynXXLogPrint(DynXXLogLevel.Debug, `z <- len:${inBytes.length} finished:${inputFinished}`);

        let inputRet = inputFunc(z, inBytes, inputFinished);
        if (inputRet <= 0) {
            DynXXLogPrint(DynXXLogLevel.Error, 'z input failed!');
            return false;
        }

        processFinished = false;
        do {
            let outBytes = processDoFunc(z);
            if (outBytes.length === 0) {
                DynXXLogPrint(DynXXLogLevel.Error, 'z process failed!');
                return false;
            }
            processFinished = processFinishedFunc(z);
            DynXXLogPrint(DynXXLogLevel.Debug, `z -> len:${outBytes.length} finished:${processFinished}`);

            writeFunc(outBytes);
        } while (!processFinished);
    } while (!inputFinished);

    flushFunc();
    return true;
}

function DynXXZZipStream(readFunc, writeFunc, flushFunc, mode, bufferSize, format) {
    let zip = _DynXXZZipInit(mode, bufferSize, format);

    let res = _DynXXZStream(bufferSize, readFunc, writeFunc, flushFunc, zip,
        (z, buffer, inputFinished) => {
            return _DynXXZZipInput(z, buffer, inputFinished);
        }, (z) => {
            return _DynXXZZipProcessDo(z);
        }, (z) => {
            return _DynXXZZipProcessFinished(z);
        });

    _DynXXZZipRelease(zip);
    return res;
}

function DynXXZUnZipStream(readFunc, writeFunc, flushFunc, bufferSize, format) {
    let unzip = _DynXXZUnZipInit(bufferSize, format);

    let res = _DynXXZStream(bufferSize, readFunc, writeFunc, flushFunc, unzip,
        (z, buffer, inputFinished) => {
            return _DynXXZUnZipInput(z, buffer, inputFinished);
        }, (z) => {
            return _DynXXZUnZipProcessDo(z);
        }, (z) => {
            return _DynXXZUnZipProcessFinished(z);
        });

    _DynXXZUnZipRelease(unzip);
    return res;
}

function DynXXZZipFile(inFilePath, outFilePath, mode, bufferSize, format) {
    let inF = std.open(inFilePath, 'r');
    let outF = std.open(outFilePath, 'w');

    let res = DynXXZZipStream(
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

function DynXXZUnZipFile(inFilePath, outFilePath, bufferSize, format) {
    let inF = std.open(inFilePath, 'r');
    let outF = std.open(outFilePath, 'w');

    let res = DynXXZUnZipStream(
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
