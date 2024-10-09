export const getVersion: () => string;
export const init: (root: string) => boolean;
export const release: () => void;

export const logSetLevel: (level: number) => void;
export type LogCallbackType = (level: number, log: string) => void;
export const logSetCallback: (callback?: LogCallbackType) => void;
export const logPrint: (level: number, content: string) => void;

export const netHttpRequest: (url: string, params: string, method: number, headers: string[], timeout: number) => string;

export const lLoadF: (file : string) => boolean;
export const lLoadS: (script : string) => boolean;
export const lCall: (func: string, params: string) => string;

export const storeSQLiteOpen: (id: string) => number;
export const storeSQLiteExecute: (conn: number, sql: string) => boolean;
export const storeSQLiteQueryDo: (conn: number, sql: string) => number;
export const storeSQLiteQueryReadRow: (queryResult: number) => boolean;
export const storeSQLiteQueryReadColumnText: (queryResult: number, column: string) => string;
export const storeSQLiteQueryReadColumnInteger: (queryResult: number, column: string) => number;
export const storeSQLiteQueryReadColumnFloat: (queryResult: number, column: string) => number;
export const storeSQLiteQueryDrop: (queryResult: number) => void;
export const storeSQLiteClose: (conn: number) => void;
        
export const storeKVOpen: (id: string) => number;
export const storeKVReadString: (conn: number, k: string) => string;
export const storeKVWriteString: (conn: number, k: string, v: string) => boolean;
export const storeKVReadInteger: (conn: number, k: string) => number;
export const storeKVWriteInteger: (conn: number, k: string, v: number) => boolean;
export const storeKVReadFloat: (conn: number, k: string) => number;
export const storeKVWriteFloat: (conn: number, k: string, v: number) => boolean;
export const storeKVContains: (conn: number, k: string) => boolean;
export const storeKVClear: (conn: number) => void;
export const storeKVClose: (conn: number) => void;

export const deviceType:() => number;
export const deviceName:() => string;
export const deviceManufacturer:() => string;
export const deviceOsVersion:() => string;
export const deviceCpuArch:() => number;

export const codingHexBytes2str:(bytes: number[]) => string;
export const codingHexStr2Bytes:(str: string) => number[];

export const cryptoRand:(len: number) => number[];
export const cryptoAesEncrypt:(inBytes: number[], keyBytes: number[]) => number[];
export const cryptoAesDecrypt:(inBytes: number[], keyBytes: number[]) => number[];
export const cryptoAesGcmEncrypt:(inBytes: number[], keyBytes: number[], initVectorBytes: number[], aadBytes: number[], tagBits: number) => number[];
export const cryptoAesGcmDecrypt:(inBytes: number[], keyBytes: number[], initVectorBytes: number[], aadBytes: number[], tagBits: number) => number[];
export const cryptoHashMd5:(inBytes: number[]) => number[];
export const cryptoHashSha256:(inBytes: number[]) => number[];
export const cryptoBase64Encode:(inBytes: number[]) => number[];
export const cryptoBase64Decode:(inBytes: number[]) => number[];

export const jsonDecoderInit:(json: string) => number;
export const jsonDecoderIsArray:(decoder: number, node: number) => boolean;
export const jsonDecoderIsObject:(decoder: number, node: number) => boolean;
export const jsonDecoderReadNode:(decoder: number, node: number, k: string) => number;
export const jsonDecoderReadChild:(decoder: number, node: number) => number;
export const jsonDecoderReadNext:(decoder: number, node: number) => number;
export const jsonDecoderReadString:(decoder: number, node: number) => string;
export const jsonDecoderReadNumber:(decoder: number, node: number) => number;
export const jsonDecoderRelease:(decoder: number) => void;