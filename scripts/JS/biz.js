function jTestStdOs() {
    std.puts(`Test JS std & os at ${os.now()}...\n`);

    let url = 'https://rinc.xyz';
    os.sleep(3333);
    std.puts(`Send HTTP req from JS async at ${os.now()}...\n`);
    
    std.evalScript(`jTestNetHttpReq("${url}");`, {async: true})
    .then(res => {
        std.puts(res);
    }, err => {
        std.puts(err);
    });
    
    std.gc();
    //std.exit(0);
}

function jTestNetHttpReq(url) {
    let params = "p0=1&p1=2";
    let method = 0;
    let headerV = ["Cache-Control: no-cache"];
    let timeout = 55555;
    let rsp = NGenXXNetHttpRequest(url, params, method, headerV, null, null, null, timeout);
    console.log(rsp);
    return rsp;
}

function jTestStoreKV() {
    let kvId = 'test_kv';
    let conn = NGenXXStoreKVOpen(kvId);
    if (conn > 0) {
        let kS = "kS";
        if (NGenXXStoreKVContains(conn, kS)) {
            NGenXXStoreKVRemove(conn, kS);
        }
        NGenXXStoreKVWriteString(conn, kS, "NGenXX");
        let vS = NGenXXStoreKVReadString(conn, kS);
        NGenXXLogPrint(NGenXXLogLevel.Debug, `KV read ${kS}: ${vS}`);

        let kI = "kI";
        if (NGenXXStoreKVContains(conn, kI)) {
            NGenXXStoreKVRemove(conn, kI);
        }
        NGenXXStoreKVWriteInteger(conn, kI, 12345678909666666);
        let vI = NGenXXStoreKVReadInteger(conn, kI);
        NGenXXLogPrint(NGenXXLogLevel.Debug, `KV read ${kI}: ${vI}`);

        let kF = "kF";
        if (NGenXXStoreKVContains(conn, kF)) {
            NGenXXStoreKVRemove(conn, kF);
        }
        NGenXXStoreKVWriteFloat(conn, kF, -0.12345678987654321);
        let vF = NGenXXStoreKVReadFloat(conn, kF);
        NGenXXLogPrint(NGenXXLogLevel.Debug, `KV read ${kF}: ${vF}`);

        NGenXXStoreKVClose(conn);
    } else {
        NGenXXLogPrint(NGenXXLogLevel.Debug, `KV open failed!!!`);
    }
}

let sqlPrepareData = `
DROP TABLE IF EXISTS TestTable;
CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, s TEXT, i INTEGER, f FLOAT);
INSERT OR IGNORE INTO TestTable (s, i, f) VALUES
('iOS', 1, 0.111111111),
('Android', 2, 0.2222222222),
('HarmonyOS', 3, 0.3333333333);`;

let sqlQuery = `SELECT * FROM TestTable;`;

function jTestStoreSQLite() {
    let dbId = 'test_db';
    let conn = NGenXXStoreSQLiteOpen(dbId);
    if (conn > 0) {
        if (NGenXXStoreSQLiteExecute(conn, sqlPrepareData)) {
            let queryResult = NGenXXStoreSQLiteQueryDo(conn, sqlQuery);
            if (queryResult > 0) {
                while (NGenXXStoreSQLiteQueryReadRow(queryResult)) {
                    let s = NGenXXStoreSQLiteQueryReadColumnText(queryResult, 's');
                    let i = NGenXXStoreSQLiteQueryReadColumnInteger(queryResult, 'i');
                    let f = NGenXXStoreSQLiteQueryReadColumnFloat(queryResult, 'f');
                    NGenXXLogPrint(NGenXXLogLevel.Debug, `s:${s} i:${i} f:${f}`);
                }

                NGenXXStoreSQLiteQueryDrop(queryResult);
            } else {
                NGenXXLogPrint(NGenXXLogLevel.Debug, `SQLite query failed!!!`);
            }
        } else {
            NGenXXLogPrint(NGenXXLogLevel.Debug, `SQLite execute failed!!!`);
        }
        NGenXXStoreSQLiteClose(conn);
    } else {
        NGenXXLogPrint(NGenXXLogLevel.Debug, `SQLite open failed!!!`);
    }
}

function jTestCryptoBase64(s) {
    let inBytes = NGenXXStr2Bytes(s);
    let enBytes = NGenXXCryptoBase64Encode(inBytes);
    let enS = NGenXXBytes2Str(enBytes);
    NGenXXLogPrint(NGenXXLogLevel.Debug, `Base64 encoded: ${enS}`);

    let deBytes = NGenXXCryptoBase64Decode(enBytes);
    let deS = NGenXXBytes2Str(deBytes);
    NGenXXLogPrint(NGenXXLogLevel.Debug, `Base64 decoded: ${deS}`);
}

function jTestCryptoHash(s) {
    let inBytes = NGenXXStr2Bytes(s);

    let md5Bytes = NGenXXCryptoHashMD5(inBytes);
    let md5S = NGenXXCodingHexBytes2Str(md5Bytes);
    NGenXXLogPrint(NGenXXLogLevel.Debug, `MD5: ${md5S}`);

    let sha256Bytes = NGenXXCryptoHashSHA256(s);
    let sha256S = NGenXXCodingHexBytes2Str(sha256Bytes);
    NGenXXLogPrint(NGenXXLogLevel.Debug, `SHA256: ${sha256S}`);
}

let AES_KEY = "QWERTYUIOPASDFGH";

function jTestCryptoAes(s) {
    let inBytes = NGenXXStr2Bytes(s);
    let keyBytes = NGenXXStr2Bytes(AES_KEY);

    let enBytes = NGenXXCryptoAesEncrypt(inBytes, keyBytes);
    let enS = NGenXXCodingHexBytes2Str(enBytes);
    NGenXXLogPrint(NGenXXLogLevel.Debug, `AES encoded: ${enS}`);

    let deBytes = NGenXXCryptoAesDecrypt(enBytes, keyBytes);
    let deS = NGenXXBytes2Str(deBytes);
    NGenXXLogPrint(NGenXXLogLevel.Debug, `AES decoded: ${deS}`);
}

function jTestCryptoAesGcm(s) {
    let inBytes = NGenXXStr2Bytes(s);
    let keyBytes = NGenXXStr2Bytes(AES_KEY);
    let ivBytes = NGenXXCryptoRand(12);
    let aadBytes = null;
    let tagBits = 96;

    let enBytes = NGenXXCryptoAesGcmEncrypt(inBytes, keyBytes, ivBytes, aadBytes, tagBits);
    let enS = NGenXXCodingHexBytes2Str(enBytes);
    NGenXXLogPrint(NGenXXLogLevel.Debug, `AES-GCM encoded: ${enS}`);

    let deBytes = NGenXXCryptoAesGcmDecrypt(enBytes, keyBytes, ivBytes, aadBytes, tagBits);
    let deS = NGenXXBytes2Str(deBytes);
    NGenXXLogPrint(NGenXXLogLevel.Debug, `AES-GCM decoded: ${deS}`);
}
