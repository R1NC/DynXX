export const getVersion: () => string;
export const init: (root: string) => boolean;
export const release: () => void;

export const enum LogLevel {
  Debug = 3,
  Info,
  Warn,
  Error,
  Fatal,
  None
}
export const logSetLevel: (level: number) => void;
export type LogCallbackType = (level: number, log: string) => void;
export const logSetCallback: (callback?: LogCallbackType) => void;
export const logPrint: (level: number, content: string) => void;

export const enum HttpMethod {
  Get = 0,
  Post,
  Put
}
export const netHttpRequest: (
  url: string,
  method: number,
  params?: string,
  headerV?: string[],
  formFieldNameV?: string[],
  formFieldMimeV?: string[],
  formFieldDataV?: string[],
  filePath?: string, fileLength?: number,
  timeout?: number) => string;

export const lLoadF: (file: string) => boolean;
export const lLoadS: (script: string) => boolean;
export const lCall: (func: string, params: string) => string;

export const jLoadF: (file: string, isModule: boolean) => boolean;
export const jLoadS: (script: string, name: string, isModule: boolean) => boolean;
export const jLoadB: (bytes: number[], isModule: boolean) => boolean;
export const jCall: (func: string, params: string, await: boolean) => string;

export const sqliteOpen: (id: string) => number;
export const sqliteExecute: (conn: number, sql: string) => boolean;
export const sqliteQueryDo: (conn: number, sql: string) => number;
export const sqliteQueryReadRow: (queryResult: number) => boolean;
export const sqliteQueryReadColumnText: (queryResult: number, column: string) => string;
export const sqliteQueryReadColumnInteger: (queryResult: number, column: string) => number;
export const sqliteQueryReadColumnFloat: (queryResult: number, column: string) => number;
export const sqliteQueryDrop: (queryResult: number) => void;
export const sqliteClose: (conn: number) => void;

export const kvOpen: (id: string) => number;
export const kvReadString: (conn: number, k: string) => string;
export const kvWriteString: (conn: number, k: string, v: string) => boolean;
export const kvReadInteger: (conn: number, k: string) => number;
export const kvWriteInteger: (conn: number, k: string, v: number) => boolean;
export const kvReadFloat: (conn: number, k: string) => number;
export const kvWriteFloat: (conn: number, k: string, v: number) => boolean;
export const kvContains: (conn: number, k: string) => boolean;
export const kvRemove: (conn: number, k: string) => boolean;
export const kvClear: (conn: number) => void;
export const kvClose: (conn: number) => void;

export const deviceType: () => number;
export const deviceName: () => string;
export const deviceManufacturer: () => string;
export const deviceOsVersion: () => string;
export const deviceCpuArch: () => number;

export const codingHexBytes2str: (bytes: number[]) => string;
export const codingHexStr2Bytes: (str: string) => number[];

export const cryptoRand: (len: number) => number[];
export const cryptoAesEncrypt: (inBytes: number[], keyBytes: number[]) => number[];
export const cryptoAesDecrypt: (inBytes: number[], keyBytes: number[]) => number[];
export const cryptoAesGcmEncrypt: (inBytes: number[], keyBytes: number[], initVectorBytes: number[], tagBits: number, aadBytes?: number[]) => number[];
export const cryptoAesGcmDecrypt: (inBytes: number[], keyBytes: number[], initVectorBytes: number[], tagBits: number, aadBytes?: number[]) => number[];

export const cryptoRsaGenKey: (base64: string, is_public: boolean) => string;
export const cryptoRsaEncrypt: (inBytes: number[], keyBytes: number[], padding: number) => number[];
export const cryptoRsaDecrypt: (inBytes: number[], keyBytes: number[], padding: number) => number[];

export const cryptoHashMd5: (inBytes: number[]) => number[];
export const cryptoHashSha1: (inBytes: number[]) => number[];
export const cryptoHashSha256: (inBytes: number[]) => number[];
export const cryptoBase64Encode: (inBytes: number[], noNewLines: boolean) => number[];
export const cryptoBase64Decode: (inBytes: number[], noNewLines: boolean) => number[];

export const jsonReadType: (node?: number) => number;
export const jsonDecoderInit: (json: string) => number;
export const jsonDecoderReadNode: (decoder: number, k: string, node?: number) => number;
export const jsonDecoderReadChild: (decoder: number, node?: number) => number;
export const jsonDecoderReadNext: (decoder: number, node?: number) => number;
export const jsonDecoderReadString: (decoder: number, node?: number) => string;
export const jsonDecoderReadInteger: (decoder: number, node?: number) => number;
export const jsonDecoderReadFloat: (decoder: number, node?: number) => number;
export const jsonDecoderRelease: (decoder: number) => void;

export const zZipInit: (mode: number, bufferSize: number, format: number) => number;
export const zZipInput: (zip: number, inBytes: number[], inFinish: boolean) => number;
export const zZipProcessDo: (zip: number) => number[];
export const zZipProcessFinished: (zip: number) => boolean;
export const zZipRelease: (zip: number) => void;
export const zUnZipInit: (bufferSize: number, format: number) => number;
export const zUnZipInput: (unzip: number, inBytes: number[], inFinish: boolean) => number;
export const zUnZipProcessDo: (unzip: number) => number[];
export const zUnZipProcessFinished: (unzip: number) => boolean;
export const zUnZipRelease: (unzip: number) => void;
export const zZipBytes: (mode: number, bufferSize: number, format: number, inBytes: number[]) => number[];
export const zUnzipBytes: (bufferSize: number, format: number, inBytes: number[]) => number[];