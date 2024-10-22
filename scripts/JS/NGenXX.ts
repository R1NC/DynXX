declare function _buffer2Bytes(buffer: ArrayBuffer): number[]

declare function _bytes2Buffer(bytes: number[]): ArrayBuffer

declare function NGenXXGetVersion(): string

declare function NGenXXRootPath(): string

const enum NGenXXLogLevel {
    Info = 0,
    Debug,
    Warn,
    Error,
    None
}

declare function NGenXXLogPrint(level: NGenXXLogLevel, content: string): void

const enum NGenXXDeviceType {
    Unknown = 0,
    Android,
    ApplePhone,
    ApplePad,
    AppleMac,
    AppleWatch,
    AppleTV,
    HarmonyOS,
    Windows,
    Linux,
    Web
}

const enum NGenXXDeviceCpuArch {
    Unknown = 0,
    X86,
    X86_64,
    IA64,
    ARM,
    ARM_64
}

declare function NGenXXDevicePlatform(): NGenXXDeviceType

declare function NGenXXDeviceName(): string

declare function NGenXXDeviceManufacturer(): string

declare function NGenXXDeviceOSVersion(): string

declare function NGenXXDeviceCPUArch(): NGenXXDeviceCpuArch

const enum NGenXXHttpMethod {
    Get = 0,
    Post,
    Put
}

declare function NGenXXNetHttpRequest(
    url: string,
    paramMap: Map<string, string | number>,
    method: NGenXXHttpMethod,
    headerMap: Map<string, string | number>,
    formFieldNameArray: string[],
    formFieldMimeArray: string[],
    formFieldDataArray: string[],
    timeout: number
): string

declare function NGenXXStoreSQLiteOpen(_id: string): number

declare function NGenXXStoreSQLiteExecute(conn: number, sql: string): boolean

declare function NGenXXStoreSQLiteQueryDo(conn: number, sql: string): number

declare function NGenXXStoreSQLiteQueryReadRow(query_result: number): boolean

declare function NGenXXStoreSQLiteQueryReadColumnText(query_result: number, column: string): string

declare function NGenXXStoreSQLiteQueryReadColumnInteger(query_result: number, column: string): number

declare function NGenXXStoreSQLiteQueryReadColumnFloat(query_result: number, column: string): number

declare function NGenXXStoreSQLiteQueryDrop(query_result: number): void

declare function NGenXXStoreSQLiteClose(conn: number): void

declare function NGenXXStoreKVOpen(_id: string): number

declare function NGenXXStoreKVReadString(conn: number, k: string): string

declare function NGenXXStoreKVWriteString(conn: number, k: string, s: string): boolean

declare function NGenXXStoreKVReadInteger(conn: number, k: string): number

declare function NGenXXStoreKVWriteInteger(conn: number, k: string, i: number): boolean

declare function NGenXXStoreKVReadFloat(conn: number, k: string): number

declare function NGenXXStoreKVWriteFloat(conn: number, k: string, f: number): boolean

declare function NGenXXStoreKVContains(conn: number, k: string): boolean

declare function NGenXXStoreKVRemove(conn: number, k: string): void

declare function NGenXXStoreKVClear(conn: number): void

declare function NGenXXStoreKVClose(conn: number): void

declare function NGenXXStr2Bytes(str: string): number[]

declare function NGenXXBytes2Str(bytes: number[]): string

declare function NGenXXCodingHexBytes2Str(bytes: number[]): string

declare function NGenXXCodingHexStr2Bytes(hexStr: string): number[]

declare function NGenXXCryptoRand(len: number): number[]

declare function NGenXXCryptoAesEncrypt(inBytes: number[], keyBytes: number[]): number[]

declare function NGenXXCryptoAesDecrypt(inBytes: number[], keyBytes: number[]): number[]

declare function NGenXXCryptoAesGcmEncrypt(inBytes: number[], keyBytes: number[], ivBytes: number[], aadBytes: number[], tagBits: number): number[]

declare function NGenXXCryptoAesGcmDecrypt(inBytes: number[], keyBytes: number[], ivBytes: number[], aadBytes: number[], tagBits: number): number[]

declare function NGenXXCryptoHashMD5(inBytes: number[]): number[]

declare function NGenXXCryptoHashSHA256(inBytes: number[]): number[]

declare function NGenXXCryptoBase64Encode(inBytes: number[]): number[]

declare function NGenXXCryptoBase64Decode(inBytes: number[]): number[]

const enum NGenXXZZipMode {
    Default = -1,
    PreferSpeed = 1,
    PreferSize = 9
}

const enum NGenXXZFormat {
    ZLib = 0,
    GZip,
    Raw
}

declare const NGenXXZBufferSize: number

declare function NGenXXZZipInit(mode: NGenXXZZipMode, bufferSize: number, format: NGenXXZFormat): number

declare function NGenXXZZipInput(zip: number, bytes: number[], finish: boolean): number

declare function NGenXXZZipProcessDo(zip: number): number[]

declare function NGenXXZZipProcessFinished(zip: number): boolean

declare function NGenXXZZipRelease(zip: number): void

declare function NGenXXZUnZipInit(bufferSize: number, format: NGenXXZFormat): number

declare function NGenXXZUnZipInput(unzip: number, bytes: number[], finish: boolean): number

declare function NGenXXZUnZipProcessDo(unzip: number): number[]

declare function NGenXXZUnZipProcessFinished(unzip: number): boolean

declare function NGenXXZUnZipRelease(unzip: number): void

declare function NGenXXZZipBytes(mode: NGenXXZZipMode, bufferSize: number, bytes: number[], format: NGenXXZFormat): number[]

declare function NGenXXZUnZipBytes(bufferSize: number, bytes: number[], format: NGenXXZFormat): number[]

declare function NGenXXZZipStream(
    mode: NGenXXZZipMode,
    bufferSize: number,
    format: NGenXXZFormat,
    readFunc: () => number[],
    writeFunc: (bytes: number[]) => void,
    flushFunc: () => void
): boolean

declare function NGenXXZUnZipStream(
    bufferSize: number,
    format: NGenXXZFormat,
    readFunc: () => number[],
    writeFunc: (bytes: number[]) => void,
    flushFunc: () => void
): boolean

declare function NGenXXZZipFile(mode: number, bufferSize: number, format: NGenXXZFormat, inFilePath: string, outFilePath: string): boolean

declare function NGenXXZUnZipFile(bufferSize: number, format: NGenXXZFormat, inFilePath: string, outFilePath: string): boolean