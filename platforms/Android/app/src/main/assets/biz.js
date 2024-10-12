function jTestBase64(s)
{
    var en = NGenXXBase64Encode(s);
    NGenXXLogPrint(1, `Base64 encoded: ${en}`);
    
    var de = NGenXXBase64Decode(en);
    NGenXXLogPrint(1, `Base64 decoded: ${de}`);
}
