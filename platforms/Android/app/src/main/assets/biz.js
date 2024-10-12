function jTestBase64(s) {
    var en = NGenXXCryptoBase64Encode(s);
    NGenXXLogPrint(1, `Base64 encoded: ${en}`);

    var de = NGenXXCryptoBase64Decode(en);
    NGenXXLogPrint(1, `Base64 decoded: ${de}`);
}

function jTestHash(s) {
    var md5Bytes = NGenXXCryptoHashMD5(s);
    var md5HexStr = NGenXXCodingHexBytes2Str(md5Bytes);
    NGenXXLogPrint(1, `MD5: ${md5HexStr}`);

    var sha256Bytes = NGenXXCryptoHashSHA256(s);
    var sha256HexStr = NGenXXCodingHexBytes2Str(sha256Bytes);
    NGenXXLogPrint(1, `SHA256: ${sha256HexStr}`);
}
