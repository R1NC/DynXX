/// Base

declare function DynXXGetVersion(): string

declare function DynXXRootPath(): string

declare function DynXXCallPlatform(msg: string): string

/// Log

const enum DynXXLogLevel {
    Debug = 3,
    Info,
    Warn,
    Error,
    Fatal,
    None
}

declare function DynXXLogPrint(level: DynXXLogLevel, content: string): void

/// DeviceInfo

const enum DynXXDeviceType {
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

const enum DynXXDeviceCpuArch {
    Unknown = 0,
    X86,
    X86_64,
    IA64,
    ARM,
    ARM_64
}

declare function DynXXDevicePlatform(): DynXXDeviceType

declare function DynXXDeviceName(): string

declare function DynXXDeviceManufacturer(): string

declare function DynXXDeviceOSVersion(): string

declare function DynXXDeviceCPUArch(): DynXXDeviceCpuArch

/// Net.Http

const enum DynXXHttpMethod {
    Get = 0,
    Post,
    Put
}

type DynXXHttpResponse = {
    code: number;
    contentType: string;
    headers: any;
    data: string;
};

declare function DynXXNetHttpRequest(
    url: string,
    method: DynXXHttpMethod,
    paramMap?: Map<string, string | number>,
    headerMap?: Map<string, string>,
    rawBodyBytes?: number[],
    formFieldNameArray?: string[],
    formFieldMimeArray?: string[],
    formFieldDataArray?: string[],
    timeout?: number
): Promise<DynXXHttpResponse>

declare function DynXXNetHttpDownload(
    url: string,
    file: string,
    timeout?: number
): Promise<boolean>

/// Store.SQLite

declare function DynXXStoreSQLiteOpen(_id: string): string

declare function DynXXStoreSQLiteExecute(conn: string, sql: string): Promise<boolean>

declare function DynXXStoreSQLiteQueryDo(conn: string, sql: string): Promise<string>

declare function DynXXStoreSQLiteQueryReadRow(query_result: string): boolean

declare function DynXXStoreSQLiteQueryReadColumnText(query_result: string, column: string): string

declare function DynXXStoreSQLiteQueryReadColumnInteger(query_result: string, column: string): number

declare function DynXXStoreSQLiteQueryReadColumnFloat(query_result: string, column: string): number

declare function DynXXStoreSQLiteQueryDrop(query_result: string): void

declare function DynXXStoreSQLiteClose(conn: string): void

/// Store.KV

declare function DynXXStoreKVOpen(_id: string): string

declare function DynXXStoreKVReadString(conn: string, k: string): string

declare function DynXXStoreKVWriteString(conn: string, k: string, s: string): boolean

declare function DynXXStoreKVReadInteger(conn: string, k: string): number

declare function DynXXStoreKVWriteInteger(conn: string, k: string, i: number): boolean

declare function DynXXStoreKVReadFloat(conn: string, k: string): number

declare function DynXXStoreKVWriteFloat(conn: string, k: string, f: number): boolean

declare function DynXXStoreKVAllKeys(conn: string): string[]

declare function DynXXStoreKVContains(conn: string, k: string): boolean

declare function DynXXStoreKVRemove(conn: string, k: string): void

declare function DynXXStoreKVClear(conn: string): void

declare function DynXXStoreKVClose(conn: string): void

/// Coding

declare function DynXXStr2Bytes(str: string): number[]

declare function DynXXBytes2Str(bytes: number[]): string

declare function DynXXCodingHexBytes2Str(bytes: number[]): string

declare function DynXXCodingHexStr2Bytes(str: string): number[]

declare function DynXXCodingBytes2Str(bytes: number[]): string

declare function DynXXCodingStr2Bytes(str: string): number[]

declare function DynXXCodingCaseUpper(str: string): string

declare function DynXXCodingCaseLower(str: string): string

/// Crypto

declare function DynXXCryptoRand(len: number): number[]

declare function DynXXCryptoAesEncrypt(inBytes: number[], keyBytes: number[]): number[]

declare function DynXXCryptoAesDecrypt(inBytes: number[], keyBytes: number[]): number[]

declare function DynXXCryptoAesGcmEncrypt(
    inBytes: number[], 
    keyBytes: number[], 
    ivBytes: number[], 
    tagBits: number,
    aadBytes?: number[]
): number[]

declare function DynXXCryptoAesGcmDecrypt(
    inBytes: number[], 
    keyBytes: number[], 
    ivBytes: number[], 
    tagBits: number,
    aadBytes?: number[]
): number[]

declare function DynXXCryptoRsaGenKey(base64: string, isPublic: boolean): number[]

declare function DynXXCryptoRsaEncrypt(inBytes: number[], keyBytes: number[]): number[]

declare function DynXXCryptoRsaDecrypt(inBytes: number[], keyBytes: number[]): number[]

declare function DynXXCryptoHashMD5(inBytes: number[]): number[]

declare function DynXXCryptoHashSHA1(inBytes: number[]): number[]

declare function DynXXCryptoHashSHA256(inBytes: number[]): number[]

declare function DynXXCryptoBase64Encode(inBytes: number[], noNewLines?: boolean): number[]

declare function DynXXCryptoBase64Decode(inBytes: number[], noNewLines?: boolean): number[]

/// Zip

const enum DynXXZFormat {
    ZLib = 0,
    GZip,
    Raw
}

const enum DynXXZZipMode {
    Default = -1,
    PreferSpeed = 1,
    PreferSize = 9
}

declare const DynXXZBufferSize: number

declare function DynXXZZipBytes(
    bytes: number[],
    mode?: DynXXZZipMode, 
    bufferSize?: number, 
    format?: DynXXZFormat
): Promise<number[]>

declare function DynXXZUnZipBytes(
    bytes: number[],
    bufferSize?: number, 
    format?: DynXXZFormat
): Promise<number[]>

declare function DynXXZZipStream(
    readFunc: () => number[],
    writeFunc: (bytes: number[]) => void,
    flushFunc: () => void,
    mode?: DynXXZZipMode,
    bufferSize?: number,
    format?: DynXXZFormat
): boolean

declare function DynXXZUnZipStream(
    readFunc: () => number[],
    writeFunc: (bytes: number[]) => void,
    flushFunc: () => void,
    bufferSize?: number,
    format?: DynXXZFormat
): boolean

declare function DynXXZZipFile(
    inFilePath: string, 
    outFilePath: string,
    mode?: number, 
    bufferSize?: number, 
    format?: DynXXZFormat
): boolean

declare function DynXXZUnZipFile(
    inFilePath: string, 
    outFilePath: string,
    bufferSize?: number, 
    format?: DynXXZFormat
): boolean
