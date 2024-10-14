function jTestNetHttpReq() {
    let url = "https://rinc.xyz";
    let params = "p0=1&p1=2";
    let method = 0;
    let headerV = ["Cache-Control: no-cache"];
    let timeout = 55555;
    return NGenXXNetHttpRequest(url, params, method, headerV, null, null, null, timeout);
}

function jTestCryptoBase64(s) {
    let inBytes = NGenXXStr2Bytes(s);
    let enBytes = NGenXXCryptoBase64Encode(inBytes);
    let enS = NGenXXBytes2Str(enBytes);
    NGenXXLogPrint(1, `Base64 encoded: ${enS}`);

    let deBytes = NGenXXCryptoBase64Decode(enBytes);
    let deS = NGenXXBytes2Str(deBytes);
    NGenXXLogPrint(1, `Base64 decoded: ${deS}`);
}

function jTestCryptoHash(s) {
    let inBytes = NGenXXStr2Bytes(s);

    let md5Bytes = NGenXXCryptoHashMD5(inBytes);
    let md5S = NGenXXCodingHexBytes2Str(md5Bytes);
    NGenXXLogPrint(1, `MD5: ${md5S}`);

    let sha256Bytes = NGenXXCryptoHashSHA256(s);
    let sha256S = NGenXXCodingHexBytes2Str(sha256Bytes);
    NGenXXLogPrint(1, `SHA256: ${sha256S}`);
}

let AES_KEY = "QWERTYUIOPASDFGH";

function jTestCryptoAes(s) {
    let inBytes = NGenXXStr2Bytes(s);
    let keyBytes = NGenXXStr2Bytes(AES_KEY);

    let enBytes = NGenXXCryptoAesEncrypt(inBytes, keyBytes);
    let enS = NGenXXCodingHexBytes2Str(enBytes);
    NGenXXLogPrint(1, `AES encoded: ${enS}`);

    let deBytes = NGenXXCryptoAesDecrypt(enBytes, keyBytes);
    let deS = NGenXXBytes2Str(deBytes);
    NGenXXLogPrint(1, `AES decoded: ${deS}`);
}

function jTestCryptoAesGcm(s) {
    let inBytes = NGenXXStr2Bytes(s);
    let keyBytes = NGenXXStr2Bytes(AES_KEY);
    let ivBytes = NGenXXCryptoRand(12);
    let aadBytes = null;
    let tagBits = 96;

    let enBytes = NGenXXCryptoAesGcmEncrypt(inBytes, keyBytes, ivBytes, aadBytes, tagBits);
    let enS = NGenXXCodingHexBytes2Str(enBytes);
    NGenXXLogPrint(1, `AES-GCM encoded: ${enS}`);

    let deBytes = NGenXXCryptoAesGcmDecrypt(enBytes, keyBytes, ivBytes, aadBytes, tagBits);
    let deS = NGenXXBytes2Str(deBytes);
    NGenXXLogPrint(1, `AES-GCM decoded: ${deS}`);
}
