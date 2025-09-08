function jTestDeviceInfo(): void {
    let platform: DynXXDeviceType = DynXXDevicePlatform()
    DynXXLogPrint(DynXXLogLevel.Debug, `Platform: ${platform}`)
    let name: string = DynXXDeviceName()
    DynXXLogPrint(DynXXLogLevel.Debug, `Name: ${name}`)
    let manufacturer: string = DynXXDeviceManufacturer()
    DynXXLogPrint(DynXXLogLevel.Debug, `Manufacturer: ${manufacturer}`)
    let osVersion: string = DynXXDeviceOSVersion()
    DynXXLogPrint(DynXXLogLevel.Debug, `OS Version: ${osVersion}`)
    let cpuArch: DynXXDeviceCpuArch = DynXXDeviceCPUArch()
    DynXXLogPrint(DynXXLogLevel.Debug, `CPU Arch: ${cpuArch}`)
}

function testMicrotask(): void {
    queueMicrotask(() => {
        jTestNetHttpReqPro('https://rinc.xyz').then((rsp)=>{
            DynXXLogPrint(DynXXLogLevel.Debug, `rsp: ${rsp}`)
        })
    })
}

function jTestNetHttpReqPro(url: string): Promise<string> {
    return new Promise((resolve, reject) => {
        if (url === undefined || url.trim() === "") {
            reject(new Error('invalid url'))
        }

        let method: number = DynXXHttpMethod.Get

        let paramMap: Map<string, number | string> = new Map()
        paramMap.set('p0', 123)
        paramMap.set('p1', 'abc')

        let headerMap: Map<string, string> = new Map()
        headerMap.set('User-Agent', 'DynXX')
        headerMap.set('Cache-Control', 'no-cache')

        DynXXNetHttpRequest(url, method, paramMap, headerMap).then((rsp) => {
            resolve(JSON.stringify(rsp))
        })
    })
}

function testPromise(): void {
    jTestNetHttpReqPro('https://rinc.xyz')
        .then(res => {
            DynXXLogPrint(DynXXLogLevel.Debug, `Response: ${res}`)
            return jTestNetHttpReqPro('https://abc.xyz')
        }, err => {
            DynXXLogPrint(DynXXLogLevel.Error, `${err}`)
        }).then(res => {
            DynXXLogPrint(DynXXLogLevel.Debug, `Response: ${res}`)
            return jTestNetHttpReqPro('https://cn.bing.com')
        }, err => {
            DynXXLogPrint(DynXXLogLevel.Error, `${err}`)
        }).then(res => {
            DynXXLogPrint(DynXXLogLevel.Debug, `Response: ${res}`)
        }, err => {
            DynXXLogPrint(DynXXLogLevel.Error, `${err}`)
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
        DynXXLogPrint(DynXXLogLevel.Debug, `Response: ${res}`)
    } catch (err) {
        DynXXLogPrint(DynXXLogLevel.Error, `${err}`)
    }
}

function jTestKV(): void {
    let kvId: string = 'test_kv'
    let conn: string = DynXXKVOpen(kvId)
    if (conn) {
        let kS: string = "kS"
        if (DynXXKVContains(conn, kS)) {
            DynXXKVRemove(conn, kS)
        }
        DynXXKVWriteString(conn, kS, "DynXX")
        let vS: string = DynXXKVReadString(conn, kS)
        DynXXLogPrint(DynXXLogLevel.Debug, `KV read ${kS}: ${vS}`)

        let kI: string = "kI"
        if (DynXXKVContains(conn, kI)) {
            DynXXKVRemove(conn, kI)
        }
        DynXXKVWriteInteger(conn, kI, 12345678909666666)
        let vI: number = DynXXKVReadInteger(conn, kI)
        DynXXLogPrint(DynXXLogLevel.Debug, `KV read ${kI}: ${vI}`)

        let kF: string = "kF"
        if (DynXXKVContains(conn, kF)) {
            DynXXKVRemove(conn, kF)
        }
        DynXXKVWriteFloat(conn, kF, -0.12345678987654321)
        let vF: number = DynXXKVReadFloat(conn, kF)
        DynXXLogPrint(DynXXLogLevel.Debug, `KV read ${kF}: ${vF}`)

        let keys: string[] = DynXXKVAllKeys(conn)
        keys.forEach((item, index, arr) => {
            DynXXLogPrint(DynXXLogLevel.Debug, `KV key${index}: ${item}`)
        })

        DynXXKVClose(conn)
    } else {
        DynXXLogPrint(DynXXLogLevel.Debug, `KV open failed!!!`)
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

function jTestSQLite(): void {
    let dbId: string = 'test_db'
    let conn: string = DynXXSQLiteOpen(dbId)
    if (conn) {
        DynXXSQLiteExecute(conn, sqlPrepareData).then((exeSuccess)=>{
            if (exeSuccess) {
                DynXXSQLiteQueryDo(conn, sqlQuery).then((queryResult)=>{
                    if (queryResult) {
                        while (DynXXSQLiteQueryReadRow(queryResult)) {
                            let s: string = DynXXSQLiteQueryReadColumnText(queryResult, 's')
                            let i: number = DynXXSQLiteQueryReadColumnInteger(queryResult, 'i')
                            let f: number = DynXXSQLiteQueryReadColumnFloat(queryResult, 'f')
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

function jTestCryptoBase64(s: string): void {
    let noNewLines: boolean = true
    let inBytes: number[] = DynXXStr2Bytes(s)
    let enBytes: number[] = DynXXCryptoBase64Encode(inBytes, noNewLines)
    let enS: string = DynXXBytes2Str(enBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `Base64 encoded: ${enS}`)

    let deBytes: number[] = DynXXCryptoBase64Decode(enBytes, noNewLines)
    let deS: string = DynXXBytes2Str(deBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `Base64 decoded: ${deS}`)
}

function jTestCryptoHash(s: string): void {
    let inBytes: number[] = DynXXStr2Bytes(s)

    let md5Bytes: number[] = DynXXCryptoHashMD5(inBytes)
    let md5S: string = DynXXCodingHexBytes2Str(md5Bytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `MD5: ${md5S}`)

    let sha256Bytes: number[] = DynXXCryptoHashSHA256(inBytes)
    let sha256S: string = DynXXCodingHexBytes2Str(sha256Bytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `SHA256: ${sha256S}`)
}

let AES_KEY: string = "QWERTYUIOPASDFGH"

function jTestCryptoAes(s: string): void {
    let inBytes: number[] = DynXXStr2Bytes(s)
    let keyBytes: number[] = DynXXStr2Bytes(AES_KEY)

    let enBytes: number[] = DynXXCryptoAesEncrypt(inBytes, keyBytes)
    let enS: string = DynXXCodingHexBytes2Str(enBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `AES encoded: ${enS}`)

    let deBytes: number[] = DynXXCryptoAesDecrypt(enBytes, keyBytes)
    let deS: string = DynXXBytes2Str(deBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `AES decoded: ${deS}`)
}

function jTestCryptoAesGcm(s: string): void {
    let inBytes: number[] = DynXXStr2Bytes(s)
    let keyBytes: number[] = DynXXStr2Bytes(AES_KEY)
    let ivBytes: number[] = DynXXCryptoRand(12)
    let tagBits: number = 96

    let enBytes: number[] = DynXXCryptoAesGcmEncrypt(inBytes, keyBytes, ivBytes, tagBits)
    let enS: string = DynXXCodingHexBytes2Str(enBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `AES-GCM encoded: ${enS}`)

    let deBytes: number[] = DynXXCryptoAesGcmDecrypt(enBytes, keyBytes, ivBytes, tagBits)
    let deS: string = DynXXBytes2Str(deBytes)
    DynXXLogPrint(DynXXLogLevel.Debug, `AES-GCM decoded: ${deS}`)
}

function jTestZip(): void {
    let root: string = DynXXRootPath()
    let inFile: string = `${root}/test.js`
    let zipFile: string = `${root}/test.gzip`
    let zipRes: boolean = DynXXZZipFile(inFile, zipFile)
    if (zipRes) {
        DynXXLogPrint(DynXXLogLevel.Debug, 'ZIP succeed!')
        let outFile: string = `${root}/test.txt`
        let unzipRes: boolean = DynXXZUnZipFile(zipFile, outFile)
        DynXXLogPrint(DynXXLogLevel.Debug, `UNZIP result: ${unzipRes}`)
    } else {
        DynXXLogPrint(DynXXLogLevel.Debug, 'ZIP failed')
    }
}

function jTestCallPlatform(): void {
    let res = DynXXCallPlatform("tsCallPlatformParam")
    DynXXLogPrint(DynXXLogLevel.Debug, `Return value from Platform: ${res}`)
}