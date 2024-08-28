export const getVersion: () => string;

export const logSetLevel: (level: number) => void;

export type LogCallbackType = (level: number, log: string) => void;
export const logSetCallback: (callback?: LogCallbackType) => void;

export const logPrint: (level: number, content: string) => void;

export const netHttpReq: (url: string, params: string) => string;

export const lCreate: () => number;

export const lLoadF: (lstate: number, file : string) => number;

export const lLoadS: (lstate: number, script : string) => number;

export const lCall: (lstate: number, func: string, params: string) => number;

export const lDestroy: (lstate: number) => void;