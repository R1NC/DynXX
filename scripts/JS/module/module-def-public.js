import {ngenxxLog} from "./module-def-private.js";

export function logD(content) {
    _log(3, content);
}

function _log(level, content) {
    content = content || '';
    ngenxxLog(level, content);
}
