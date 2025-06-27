function jTestDeviceInfo(): void {
    let platform: NGenXXDeviceType = NGenXXDevicePlatform()
    NGenXXLogPrint(NGenXXLogLevel.Debug, `Platform: ${platform}`)
    let name: string = NGenXXDeviceName()
    NGenXXLogPrint(NGenXXLogLevel.Debug, `Name: ${name}`)
    let manufacturer: string = NGenXXDeviceManufacturer()
    NGenXXLogPrint(NGenXXLogLevel.Debug, `Manufacturer: ${manufacturer}`)
    let osVersion: string = NGenXXDeviceOSVersion()
    NGenXXLogPrint(NGenXXLogLevel.Debug, `OS Version: ${osVersion}`)
    let cpuArch: NGenXXDeviceCpuArch = NGenXXDeviceCPUArch()
    NGenXXLogPrint(NGenXXLogLevel.Debug, `CPU Arch: ${cpuArch}`)
}

function testMicrotask(): void {
    queueMicrotask(() => {
        jTestNetHttpReqPro('https://rinc.xyz').then((rsp)=>{
            NGenXXLogPrint(NGenXXLogLevel.Debug, `rsp: ${rsp}`)
        })
    })
}

function jTestNetHttpReqPro(url: string): Promise<string> {
    return new Promise((resolve, reject) => {
        if (url === undefined || url.trim() === "") {
            reject(new Error('invalid url'))
        }

        let method: number = NGenXXHttpMethod.Get

        let paramMap: Map<string, number | string> = new Map()
        paramMap.set('p0', 123)
        paramMap.set('p1', 'abc')

        let headerMap: Map<string, string> = new Map()
        headerMap.set('User-Agent', 'NGenXX')
        headerMap.set('Cache-Control', 'no-cache')

        NGenXXNetHttpRequest(url, method, paramMap, headerMap).then((rsp) => {
            resolve(JSON.stringify(rsp))
        })
    })
}

function testPromise(): void {
    jTestNetHttpReqPro('https://rinc.xyz')
        .then(res => {
            NGenXXLogPrint(NGenXXLogLevel.Debug, `Response: ${res}`)
            return jTestNetHttpReqPro('https://abc.xyz')
        }, err => {
            NGenXXLogPrint(NGenXXLogLevel.Error, `${err}`)
        }).then(res => {
            NGenXXLogPrint(NGenXXLogLevel.Debug, `Response: ${res}`)
            return jTestNetHttpReqPro('https://cn.bing.com')
        }, err => {
            NGenXXLogPrint(NGenXXLogLevel.Error, `${err}`)
        }).then(res => {
            NGenXXLogPrint(NGenXXLogLevel.Debug, `Response: ${res}`)
        }, err => {
            NGenXXLogPrint(NGenXXLogLevel.Error, `${err}`)
        })
}

function testPromiseAll(): void {
    const pro0 = jTestNetHttpReqPro('https://rinc.xyz')
    const pro1 = jTestNetHttpReqPro('https://abc.xyz')
    const pro2 = jTestNetHttpReqPro('https://cn.bing.com')
    Promise.all([pro0, pro1, pro2]).then((values) => {
        console.log(values);
    })
}

async function testAwait(): Promise<void> {
    try {
        const res = await jTestNetHttpReqPro('https://rinc.xyz')
        NGenXXLogPrint(NGenXXLogLevel.Debug, `Response: ${res}`)
    } catch (err) {
        NGenXXLogPrint(NGenXXLogLevel.Error, `${err}`)
    }
}

function jTestStoreKV(): void {
    let kvId: string = 'test_kv'
    let conn: string = NGenXXStoreKVOpen(kvId)
    if (conn) {
        let kS: string = "kS"
        if (NGenXXStoreKVContains(conn, kS)) {
            NGenXXStoreKVRemove(conn, kS)
        }
        NGenXXStoreKVWriteString(conn, kS, "NGenXX")
        let vS: string = NGenXXStoreKVReadString(conn, kS)
        NGenXXLogPrint(NGenXXLogLevel.Debug, `KV read ${kS}: ${vS}`)

        let kI: string = "kI"
        if (NGenXXStoreKVContains(conn, kI)) {
            NGenXXStoreKVRemove(conn, kI)
        }
        NGenXXStoreKVWriteInteger(conn, kI, 12345678909666666)
        let vI: number = NGenXXStoreKVReadInteger(conn, kI)
        NGenXXLogPrint(NGenXXLogLevel.Debug, `KV read ${kI}: ${vI}`)

        let kF: string = "kF"
        if (NGenXXStoreKVContains(conn, kF)) {
            NGenXXStoreKVRemove(conn, kF)
        }
        NGenXXStoreKVWriteFloat(conn, kF, -0.12345678987654321)
        let vF: number = NGenXXStoreKVReadFloat(conn, kF)
        NGenXXLogPrint(NGenXXLogLevel.Debug, `KV read ${kF}: ${vF}`)

        let keys: string[] = NGenXXStoreKVAllKeys(conn)
        keys.forEach((item, index, arr) => {
            NGenXXLogPrint(NGenXXLogLevel.Debug, `KV key${index}: ${item}`)
        })

        NGenXXStoreKVClose(conn)
    } else {
        NGenXXLogPrint(NGenXXLogLevel.Debug, `KV open failed!!!`)
    }
}

let sqlPrepareData: string = `
DROP TABLE IF EXISTS TestTable;
CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, s TEXT, i INTEGER, f FLOAT);
INSERT OR IGNORE INTO TestTable (s, i, f) VALUES
('iOS', 1, 0.111111111),
('Android', 2, 0.2222222222),
('HarmonyOS', 3, 0.3333333333);`

let sqlQuery: string = `SELECT * FROM TestTable;`

function jTestStoreSQLite(): void {
    let dbId: string = 'test_db'
    let conn: string = NGenXXStoreSQLiteOpen(dbId)
    if (conn) {
        NGenXXStoreSQLiteExecute(conn, sqlPrepareData).then((exeSuccess)=>{
            if (exeSuccess) {
                NGenXXStoreSQLiteQueryDo(conn, sqlQuery).then((queryResult)=>{
                    if (queryResult) {
                        while (NGenXXStoreSQLiteQueryReadRow(queryResult)) {
                            let s: string = NGenXXStoreSQLiteQueryReadColumnText(queryResult, 's')
                            let i: number = NGenXXStoreSQLiteQueryReadColumnInteger(queryResult, 'i')
                            let f: number = NGenXXStoreSQLiteQueryReadColumnFloat(queryResult, 'f')
                            NGenXXLogPrint(NGenXXLogLevel.Debug, `s:${s} i:${i} f:${f}`)
                        }
                        NGenXXStoreSQLiteQueryDrop(queryResult)
                    } else {
                        NGenXXLogPrint(NGenXXLogLevel.Debug, `SQLite query failed!!!`)
                    }
                })
            } else {
                NGenXXLogPrint(NGenXXLogLevel.Debug, `SQLite execute failed!!!`)
            }
            NGenXXStoreSQLiteClose(conn)
        })
    } else {
        NGenXXLogPrint(NGenXXLogLevel.Debug, `SQLite open failed!!!`)
    }
}

function jTestCryptoBase64(s: string): void {
    let noNewLines: boolean = true
    let inBytes: number[] = NGenXXStr2Bytes(s)
    let enBytes: number[] = NGenXXCryptoBase64Encode(inBytes, noNewLines)
    let enS: string = NGenXXBytes2Str(enBytes)
    NGenXXLogPrint(NGenXXLogLevel.Debug, `Base64 encoded: ${enS}`)

    let deBytes: number[] = NGenXXCryptoBase64Decode(enBytes, noNewLines)
    let deS: string = NGenXXBytes2Str(deBytes)
    NGenXXLogPrint(NGenXXLogLevel.Debug, `Base64 decoded: ${deS}`)
}

function jTestCryptoHash(s: string): void {
    let inBytes: number[] = NGenXXStr2Bytes(s)

    let md5Bytes: number[] = NGenXXCryptoHashMD5(inBytes)
    let md5S: string = NGenXXCodingHexBytes2Str(md5Bytes)
    NGenXXLogPrint(NGenXXLogLevel.Debug, `MD5: ${md5S}`)

    let sha256Bytes: number[] = NGenXXCryptoHashSHA256(inBytes)
    let sha256S: string = NGenXXCodingHexBytes2Str(sha256Bytes)
    NGenXXLogPrint(NGenXXLogLevel.Debug, `SHA256: ${sha256S}`)
}

let AES_KEY: string = "QWERTYUIOPASDFGH"

function jTestCryptoAes(s: string): void {
    let inBytes: number[] = NGenXXStr2Bytes(s)
    let keyBytes: number[] = NGenXXStr2Bytes(AES_KEY)

    let enBytes: number[] = NGenXXCryptoAesEncrypt(inBytes, keyBytes)
    let enS: string = NGenXXCodingHexBytes2Str(enBytes)
    NGenXXLogPrint(NGenXXLogLevel.Debug, `AES encoded: ${enS}`)

    let deBytes: number[] = NGenXXCryptoAesDecrypt(enBytes, keyBytes)
    let deS: string = NGenXXBytes2Str(deBytes)
    NGenXXLogPrint(NGenXXLogLevel.Debug, `AES decoded: ${deS}`)
}

function jTestCryptoAesGcm(s: string): void {
    let inBytes: number[] = NGenXXStr2Bytes(s)
    let keyBytes: number[] = NGenXXStr2Bytes(AES_KEY)
    let ivBytes: number[] = NGenXXCryptoRand(12)
    let tagBits: number = 96

    let enBytes: number[] = NGenXXCryptoAesGcmEncrypt(inBytes, keyBytes, ivBytes, tagBits)
    let enS: string = NGenXXCodingHexBytes2Str(enBytes)
    NGenXXLogPrint(NGenXXLogLevel.Debug, `AES-GCM encoded: ${enS}`)

    let deBytes: number[] = NGenXXCryptoAesGcmDecrypt(enBytes, keyBytes, ivBytes, tagBits)
    let deS: string = NGenXXBytes2Str(deBytes)
    NGenXXLogPrint(NGenXXLogLevel.Debug, `AES-GCM decoded: ${deS}`)
}

function jTestZip(): void {
    let root: string = NGenXXRootPath()
    let inFile: string = `${root}/test.js`
    let zipFile: string = `${root}/test.gzip`
    let zipRes: boolean = NGenXXZZipFile(inFile, zipFile)
    if (zipRes) {
        NGenXXLogPrint(NGenXXLogLevel.Debug, 'ZIP succeed!')
        let outFile: string = `${root}/test.txt`
        let unzipRes: boolean = NGenXXZUnZipFile(zipFile, outFile)
        NGenXXLogPrint(NGenXXLogLevel.Debug, `UNZIP result: ${unzipRes}`)
    } else {
        NGenXXLogPrint(NGenXXLogLevel.Debug, 'ZIP failed')
    }
}

function jTestCallPlatform(): void {
    let res = NGenXXCallPlatform("tsCallPlatformParam")
    NGenXXLogPrint(NGenXXLogLevel.Debug, `Return value from Platform: ${res}`)
}