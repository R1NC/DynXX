export const getVersion: () => string;
export const init: (useLua: boolean) => number;
export const release: (handle: number) => void;

export const logSetLevel: (level: number) => void;
export type LogCallbackType = (level: number, log: string) => void;
export const logSetCallback: (callback?: LogCallbackType) => void;
export const logPrint: (level: number, content: string) => void;

export const netHttpReq: (url: string, params: string) => string;

export const lLoadF: (lstate: number, file : string) => boolean;
export const lLoadS: (lstate: number, script : string) => boolean;
export const lCall: (lstate: number, func: string, params: string) => number;