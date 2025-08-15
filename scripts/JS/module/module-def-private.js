export function dynxxLog(level, content) {
    let inJson = JSON.stringify({
        "level": level,
        "content": content
    });
    dynxx_log_print(inJson);
}
