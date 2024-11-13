export function ngenxxLog(level, content) {
    let inJson = JSON.stringify({
        "level": level,
        "content": content
    });
    ngenxx_log_printJ(inJson);
}
