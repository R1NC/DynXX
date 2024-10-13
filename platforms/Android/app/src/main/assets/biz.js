function jTestBase64(s) {
    var en = NGenXXCryptoBase64Encode(s);
    NGenXXLogPrint(1, `Base64 encoded: ${en}`);

    var de = NGenXXCryptoBase64Decode(en);
    NGenXXLogPrint(1, `Base64 decoded: ${de}`);
}

function jTestHash(s) {
    var md5 = NGenXXCryptoHashMD5(s);
    NGenXXLogPrint(1, `MD5: ${md5}`);

    var sha256 = NGenXXCryptoHashSHA256(s);
    NGenXXLogPrint(1, `SHA256: ${sha256}`);
}

function jTestAes(str) {
    var key = "QWERTYUIOPASDFGH";
    var enc = NGenXXCryptoAesEncrypt(str, key);
    var dec = NGenXXCryptoAesDecrypt(enc, key);
    return dec;
}

function jTestAesGcm(str) {
    var key = "QWERTYUIOPASDFGH";
    var iv = "ASDFGHJKLZXC";
    var tagBits = 96;
    var enc = NGenXXCryptoAesGcmEncrypt(str, key, iv, tagBits);
    var dec = NGenXXCryptoAesGcmDecrypt(enc, key, iv, tagBits);
    return dec;
}
