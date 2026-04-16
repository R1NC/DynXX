function TestDeviceInfo() {
    let platform = DynXXDevicePlatform()
    DynXXLogPrint(DynXXLogLevel.Debug, `Platform: ${platform}`)
    let name = DynXXDeviceName()
    DynXXLogPrint(DynXXLogLevel.Debug, `Name: ${name}`)
    let manufacturer = DynXXDeviceManufacturer()
    DynXXLogPrint(DynXXLogLevel.Debug, `Manufacturer: ${manufacturer}`)
    let osVersion = DynXXDeviceOSVersion()
    DynXXLogPrint(DynXXLogLevel.Debug, `OS Version: ${osVersion}`)
    let cpuArch = DynXXDeviceCPUArch()
    DynXXLogPrint(DynXXLogLevel.Debug, `CPU Arch: ${cpuArch}`)
}

function TestMicrotask() {
    queueMicrotask(() => {
        TestNetHttpReqPro('https://rinc.xyz').then((rsp)=>{
            DynXXLogPrint(DynXXLogLevel.Debug, `rsp: ${rsp}`)
        })
    })
}

function TestNetHttpReqPro(url) {
    return new Promise((resolve, reject) => {
        if (url === undefined || url.trim() === "") {
            reject(new Error('invalid url'))
        }

        let method = DynXXHttpMethod.Get

        let paramMap = new Map()
        paramMap.set('p0', 123)
        paramMap.set('p1', 'abc')

        let headerMap = new Map()
        headerMap.set('User-Agent', 'DynXX')
        headerMap.set('Cache-Control', 'no-cache')

        DynXXNetHttpRequest(url, method, paramMap, headerMap).then((rsp) => {
            resolve(JSON.stringify(rsp))
        })
    })
}

function TestPromise() {
    TestNetHttpReqPro('https://rinc.xyz')
        .then(res => {
            DynXXLogPrint(DynXXLogLevel.Debug, `Response: ${res}`)
            return TestNetHttpReqPro('https://abc.xyz')
        }, err => {
            DynXXLogPrint(DynXXLogLevel.Error, `${err}`)
        }).then(res => {
            DynXXLogPrint(DynXXLogLevel.Debug, `Response: ${res}`)
            return TestNetHttpReqPro('https://cn.bing.com')
        }, err => {
            DynXXLogPrint(DynXXLogLevel.Error, `${err}`)
        }).then(res => {
            DynXXLogPrint(DynXXLogLevel.Debug, `Response: ${res}`)
        }, err => {
            DynXXLogPrint(DynXXLogLevel.Error, `${err}`)
        })
}

function TestPromiseAll() {
    const pro0 = TestNetHttpReqPro('https://rinc.xyz')
    const pro1 = TestNetHttpReqPro('https://abc.xyz')
    const pro2 = TestNetHttpReqPro('https://cn.bing.com')
    Promise.all([pro0, pro1, pro2]).then((values) => {
        console.log(values);
    })
}

async function TestAwait() {
    try {
        const res = await TestNetHttpReqPro('https://rinc.xyz')
        DynXXLogPrint(DynXXLogLevel.Debug, `Response: ${res}`)
    } catch (err) {
        DynXXLogPrint(DynXXLogLevel.Error, `${err}`)
    }
}

function TestKV() {
    let kvId = 'test_kv'
    let conn = DynXXKVOpen(kvId)
    if (conn) {
        let kS = "kS"
        if (DynXXKVContains(conn, kS)) {
            DynXXKVRemove(conn, kS)
        }
        DynXXKVWriteString(conn, kS, "DynXX")
        let vS = DynXXKVReadString(conn, kS)
        DynXXLogPrint(DynXXLogLevel.Debug, `KV read ${kS}: ${vS}`)

        let kI = "kI"
        if (DynXXKVContains(conn, kI)) {
            DynXXKVRemove(conn, kI)
        }
        DynXXKVWriteInteger(conn, kI, 12345678909666666)
        let vI = DynXXKVReadInteger(conn, kI)
        DynXXLogPrint(DynXXLogLevel.Debug, `KV read ${kI}: ${vI}`)

        let kF = "kF"
        if (DynXXKVContains(conn, kF)) {
            DynXXKVRemove(conn, kF)
        }
        DynXXKVWriteFloat(conn, kF, -0.12345678987654321)
        let vF = DynXXKVReadFloat(conn, kF)
        DynXXLogPrint(DynXXLogLevel.Debug, `KV read ${kF}: ${vF}`)

        let keys = DynXXKVAllKeys(conn)
        keys.forEach((item, index, arr) => {
            DynXXLogPrint(DynXXLogLevel.Debug, `KV key${index}: ${item}`)
        })

        DynXXKVClose(conn)
    } else {
        DynXXLogPrint(DynXXLogLevel.Debug, `KV open failed!!!`)
    }
}

var sqlPrepareData = `
DROP TABLE IF EXISTS TestTable;
CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, s TEXT, i INTEGER, f FLOAT);
INSERT OR IGNORE INTO TestTable (s, i, f) VALUES
('iOS', 1, 0.111111111),
('Android', 2, 0.2222222222),
('HarmonyOS', 3, 0.3333333333);`

var sqlQuery = `SELECT * FROM TestTable;`

function TestSQLite() {
    let dbId = 'test_db'
    let conn = DynXXSQLiteOpen(dbId)
    if (conn) {
        DynXXSQLiteExecute(conn, sqlPrepareData).then((exeSuccess)=>{
            if (exeSuccess) {
                DynXXSQLiteQueryDo(conn, sqlQuery).then((queryResult)=>{
                    if (queryResult) {
                        while (DynXXSQLiteQueryReadRow(queryResult)) {
                            let s = DynXXSQLiteQueryReadColumnText(queryResult, 's')
                            let i = DynXXSQLiteQueryReadColumnInteger(queryResult, 'i')
                            let f = DynXXSQLiteQueryReadColumnFloat(queryResult, 'f')
                            DynXXLogPrint(DynXXLogLevel.Debug, `s:${s} i:${i} f:${f}`)
                        }
                        DynXXSQLiteQueryDrop(queryResult)
                    } else {
                        DynXXLogPrint(DynXXLogLevel.Debug, `SQLite query failed!!!`)
                    }
                    DynXXSQLiteClose(conn)
                })
            } else {
                DynXXLogPrint(DynXXLogLevel.Debug, `SQLite execute failed!!!`)
                DynXXSQLiteClose(conn)
            }
        })
    } else {
        DynXXLogPrint(DynXXLogLevel.Debug, `SQLite open failed!!!`)
    }
}

function TestCryptoBase64(s) {
    let noNewLines = true
    let inBytes = DynXXStr2Bytes(s)
    let enBytes = DynXXCryptoBase64Encode(inBytes, noNewLines)
    let enS = DynXXBytes2Str(enBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `Base64 encoded: ${enS}`)

    let deBytes = DynXXCryptoBase64Decode(enBytes, noNewLines)
    let deS = DynXXBytes2Str(deBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `Base64 decoded: ${deS}`)
}

function TestCryptoHash(s) {
    let inBytes = DynXXStr2Bytes(s)

    let md5Bytes = DynXXCryptoHashMD5(inBytes)
    let md5S = DynXXCodingHexBytes2Str(md5Bytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `MD5: ${md5S}`)

    let sha256Bytes = DynXXCryptoHashSHA256(inBytes)
    let sha256S = DynXXCodingHexBytes2Str(sha256Bytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `SHA256: ${sha256S}`)
}

var AES_KEY = "QWERTYUIOPASDFGH"

function TestCryptoAes(s) {
    let inBytes = DynXXStr2Bytes(s)
    let keyBytes = DynXXStr2Bytes(AES_KEY)

    let enBytes = DynXXCryptoAesEncrypt(inBytes, keyBytes)
    let enS = DynXXCodingHexBytes2Str(enBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `AES encoded: ${enS}`)

    let deBytes = DynXXCryptoAesDecrypt(enBytes, keyBytes)
    let deS = DynXXBytes2Str(deBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `AES decoded: ${deS}`)
}

function TestCryptoAesGcm(s) {
    let inBytes = DynXXStr2Bytes(s)
    let keyBytes = DynXXStr2Bytes(AES_KEY)
    let ivBytes = DynXXCryptoRand(12)
    let tagBits = 96

    let enBytes = DynXXCryptoAesGcmEncrypt(inBytes, keyBytes, ivBytes, tagBits)
    let enS = DynXXCodingHexBytes2Str(enBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `AES-GCM encoded: ${enS}`)

    let deBytes = DynXXCryptoAesGcmDecrypt(enBytes, keyBytes, ivBytes, tagBits)
    let deS = DynXXBytes2Str(deBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `AES-GCM decoded: ${deS}`)
}

async function TestZip() {
    let inS = 'DynXX ZIP bytes test'
    let inBytes = DynXXStr2Bytes(inS)
    let zipOut = await DynXXZZipBytes(inBytes, -1, 16 * 1024, 1)
    let zipBytes = Array.isArray(zipOut) ? zipOut : _json2Array(zipOut)
    DynXXLogPrint(DynXXLogLevel.Debug, `ZIP bytes len: ${zipBytes.length}`)

    let unzipOut = await DynXXZUnZipBytes(zipBytes, 16 * 1024, 1)
    let unzipBytes = Array.isArray(unzipOut) ? unzipOut : _json2Array(unzipOut)
    let outS = DynXXBytes2Str(unzipBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `UNZIP bytes decoded: ${outS}`)
    DynXXLogPrint(DynXXLogLevel.Debug, `ZIP bytes roundtrip ok: ${outS === inS}`)
}

function TestCallPlatform() {
    let res = DynXXCallPlatform("tsCallPlatformParam")
    DynXXLogPrint(DynXXLogLevel.Debug, `Return value from Platform: ${res}`)
}
