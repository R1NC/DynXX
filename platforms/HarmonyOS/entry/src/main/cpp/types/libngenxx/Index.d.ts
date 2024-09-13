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
export const lCall: (func: string, params: string) => number;