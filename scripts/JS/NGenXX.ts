/// Base

declare function NGenXXGetVersion(): string

declare function NGenXXRootPath(): string

declare function NGenXXAskPlatform(msg: string): string

/// Log

const enum NGenXXLogLevel {
    Debug = 3,
    Info,
    Warn,
    Error,
    Fatal,
    None
}

declare function NGenXXLogPrint(level: NGenXXLogLevel, content: string): void

/// DeviceInfo

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

/// Net.Http

const enum NGenXXHttpMethod {
    Get = 0,
    Post,
    Put
}

type NGenXXHttpResponse = {
    code: number;
    contentType: string;
    data: string;
};

declare function NGenXXNetHttpRequest(
    url: string,
    method: NGenXXHttpMethod,
    paramMap?: Map<string, string | number>,
    headerMap?: Map<string, string>,
    rawBodyBytes?: number[],
    formFieldNameArray?: string[],
    formFieldMimeArray?: string[],
    formFieldDataArray?: string[],
    timeout?: number
): Promise<NGenXXHttpResponse>

/// Store.SQLite

declare function NGenXXStoreSQLiteOpen(_id: string): string

declare function NGenXXStoreSQLiteExecute(conn: string, sql: string): Promise<boolean>

declare function NGenXXStoreSQLiteQueryDo(conn: string, sql: string): Promise<string>

declare function NGenXXStoreSQLiteQueryReadRow(query_result: string): boolean

declare function NGenXXStoreSQLiteQueryReadColumnText(query_result: string, column: string): string

declare function NGenXXStoreSQLiteQueryReadColumnInteger(query_result: string, column: string): number

declare function NGenXXStoreSQLiteQueryReadColumnFloat(query_result: string, column: string): number

declare function NGenXXStoreSQLiteQueryDrop(query_result: string): void

declare function NGenXXStoreSQLiteClose(conn: string): void

/// Store.KV

declare function NGenXXStoreKVOpen(_id: string): string

declare function NGenXXStoreKVReadString(conn: string, k: string): string

declare function NGenXXStoreKVWriteString(conn: string, k: string, s: string): boolean

declare function NGenXXStoreKVReadInteger(conn: string, k: string): number

declare function NGenXXStoreKVWriteInteger(conn: string, k: string, i: number): boolean

declare function NGenXXStoreKVReadFloat(conn: string, k: string): number

declare function NGenXXStoreKVWriteFloat(conn: string, k: string, f: number): boolean

declare function NGenXXStoreKVAllKeys(conn: string): string[]

declare function NGenXXStoreKVContains(conn: string, k: string): boolean

declare function NGenXXStoreKVRemove(conn: string, k: string): void

declare function NGenXXStoreKVClear(conn: string): void

declare function NGenXXStoreKVClose(conn: string): void

/// Coding

declare function NGenXXStr2Bytes(str: string): number[]

declare function NGenXXBytes2Str(bytes: number[]): string

declare function NGenXXCodingHexBytes2Str(bytes: number[]): string

declare function NGenXXCodingHexStr2Bytes(str: string): number[]

declare function NGenXXCodingBytes2Str(bytes: number[]): string

declare function NGenXXCodingStr2Bytes(str: string): number[]

declare function NGenXXCodingCaseUpper(str: string): string

declare function NGenXXCodingCaseLower(str: string): string

/// Crypto

declare function NGenXXCryptoRand(len: number): number[]

declare function NGenXXCryptoAesEncrypt(inBytes: number[], keyBytes: number[]): number[]

declare function NGenXXCryptoAesDecrypt(inBytes: number[], keyBytes: number[]): number[]

declare function NGenXXCryptoAesGcmEncrypt(
    inBytes: number[], 
    keyBytes: number[], 
    ivBytes: number[], 
    tagBits: number,
    aadBytes?: number[]
): number[]

declare function NGenXXCryptoAesGcmDecrypt(
    inBytes: number[], 
    keyBytes: number[], 
    ivBytes: number[], 
    tagBits: number,
    aadBytes?: number[]
): number[]

declare function NGenXXCryptoHashMD5(inBytes: number[]): number[]

declare function NGenXXCryptoHashSHA256(inBytes: number[]): number[]

declare function NGenXXCryptoBase64Encode(inBytes: number[]): number[]

declare function NGenXXCryptoBase64Decode(inBytes: number[]): number[]

/// Zip

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

declare function NGenXXZZipBytes(
    bytes: number[],
    mode?: NGenXXZZipMode, 
    bufferSize?: number, 
    format?: NGenXXZFormat
): number[]

declare function NGenXXZUnZipBytes(
    bytes: number[],
    bufferSize?: number, 
    format?: NGenXXZFormat
): number[]

declare function NGenXXZZipStream(
    readFunc: () => number[],
    writeFunc: (bytes: number[]) => void,
    flushFunc: () => void,
    mode?: NGenXXZZipMode,
    bufferSize?: number,
    format?: NGenXXZFormat
): boolean

declare function NGenXXZUnZipStream(
    readFunc: () => number[],
    writeFunc: (bytes: number[]) => void,
    flushFunc: () => void,
    bufferSize?: number,
    format?: NGenXXZFormat
): boolean

declare function NGenXXZZipFile(
    inFilePath: string, 
    outFilePath: string,
    mode?: number, 
    bufferSize?: number, 
    format?: NGenXXZFormat
): boolean

declare function NGenXXZUnZipFile(
    inFilePath: string, 
    outFilePath: string,
    bufferSize?: number, 
    format?: NGenXXZFormat
): boolean
