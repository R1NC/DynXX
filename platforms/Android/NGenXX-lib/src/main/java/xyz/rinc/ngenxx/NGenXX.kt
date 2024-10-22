package xyz.rinc.ngenxx

class NGenXX {
    companion object {
        init {
            System.loadLibrary("NGenXXJNI")
        }

        external fun getVersion(): String
        external fun init(root: String): Boolean
        external fun release()

        enum class LogLevel(val value: Int) {
            Info(0),
            Debug(1),
            Warn(2),
            Error(3),
            None(4)
        }
        external fun logSetLevel(level: Int)
        external fun logSetCallback(callback: ((level: Int, content: String) -> Unit)?)
        external fun logPrint(level: Int, content: String)

        enum class HttpMethod(val value: Int) {
            Get(0),
            Post(1),
            Put(2),
        }
        external fun netHttpRequest(url: String, params: String?, method: Int,
                                    headerV: Array<String>?,
                                    formFieldNameV: Array<String>?,
                                    formFieldMimeV: Array<String>?,
                                    formFieldDataV: Array<String>?,
                                    filePath: String?,
                                    fileLength: Long,
                                    timeout: Long): String?

        external fun lLoadF(file: String): Boolean
        external fun lLoadS(script: String): Boolean
        external fun lCall(func: String, params: String?): String?

        external fun jLoadF(file: String): Boolean
        external fun jLoadS(script: String, name: String): Boolean
        external fun jLoadB(bytes: ByteArray): Boolean
        external fun jCall(func: String, params: String?): String?

        external fun storeSQLiteOpen(id: String): Long
        external fun storeSQLiteExecute(conn: Long, sql: String): Boolean
        external fun storeSQLiteQueryDo(conn: Long, sql: String): Long
        external fun storeSQLiteQueryReadRow(queryResult: Long): Boolean
        external fun storeSQLiteQueryReadColumnText(queryResult: Long, column: String): String?
        external fun storeSQLiteQueryReadColumnInteger(queryResult: Long, column: String): Long
        external fun storeSQLiteQueryReadColumnFloat(queryResult: Long, column: String): Double
        external fun storeSQLiteQueryDrop(queryResult: Long)
        external fun storeSQLiteClose(conn: Long)
        
        external fun storeKVOpen(id: String): Long
        external fun storeKVReadString(conn: Long, k: String): String?
        external fun storeKVWriteString(conn: Long, k: String, v: String?): Boolean
        external fun storeKVReadInteger(conn: Long, k: String): Long
        external fun storeKVWriteInteger(conn: Long, k: String, v: Long): Boolean
        external fun storeKVReadFloat(conn: Long, k: String): Double
        external fun storeKVWriteFloat(conn: Long, k: String, v: Double): Boolean
        external fun storeKVContains(conn: Long, k: String): Boolean
        external fun storeKVClear(conn: Long)
        external fun storeKVClose(conn: Long)

        external fun deviceType(): Int
        external fun deviceName(): String?
        external fun deviceManufacturer(): String?
        external fun deviceOsVersion(): String?
        external fun deviceCpuArch(): Int

        external fun codingHexBytes2Str(bytes: ByteArray): String
        external fun codingHexStr2Bytes(str: String): ByteArray

        external fun cryptoRandom(len: Int): ByteArray
        external fun cryptoAesEncrypt(input: ByteArray, key: ByteArray): ByteArray
        external fun cryptoAesDecrypt(input: ByteArray, key: ByteArray): ByteArray
        external fun cryptoAesGcmEncrypt(input: ByteArray, key: ByteArray, initVector: ByteArray, aad: ByteArray?, tagBits: Int): ByteArray
        external fun cryptoAesGcmDecrypt(input: ByteArray, key: ByteArray, initVector: ByteArray, aad: ByteArray?, tagBits: Int): ByteArray
        external fun cryptoHashMd5(input: ByteArray): ByteArray
        external fun cryptoHashSha256(input: ByteArray): ByteArray
        external fun cryptoBase64Encode(input: ByteArray): ByteArray
        external fun cryptoBase64Decode(input: ByteArray): ByteArray

        external fun jsonDecoderInit(json: String): Long
        external fun jsonDecoderIsArray(decoder: Long, node: Long): Boolean
        external fun jsonDecoderIsObject(decoder: Long, node: Long): Boolean
        external fun jsonDecoderReadNode(decoder: Long, node: Long, k: String): Long
        external fun jsonDecoderReadString(decoder: Long, node: Long): String?
        external fun jsonDecoderReadNumber(decoder: Long, node: Long): Double
        external fun jsonDecoderReadChild(decoder: Long, node: Long): Long
        external fun jsonDecoderReadNext(decoder: Long, node: Long): Long
        external fun jsonDecoderRelease(decoder: Long)

        enum class ZipMode(val value: Int) {
            Default(-1),
            PreferSpeed(1),
            PreferSize(9)
        }
        enum class ZFormat(val value: Int) {
            ZLib(0),
            GZip(1),
            Raw(2)
        }
        external fun zZipInit(mode: Int, bufferSize: Long, format: Int): Long
        external fun zZipInput(zip: Long, inBytes: ByteArray, inLen: Int, inFinish: Boolean): Long
        external fun zZipProcessDo(zip: Long): ByteArray
        external fun zZipProcessFinished(zip: Long): Boolean
        external fun zZipRelease(zip: Long)
        external fun zUnZipInit(bufferSize: Long, format: Int): Long
        external fun zUnZipInput(unzip: Long, inBytes: ByteArray, inLen: Int, inFinish: Boolean): Long
        external fun zUnZipProcessDo(unzip: Long): ByteArray
        external fun zUnZipProcessFinished(unzip: Long): Boolean
        external fun zUnZipRelease(unzip: Long)
        external fun zZipBytes(mode: Int, bufferSize: Long, format: Int, bytes: ByteArray): ByteArray
        external fun zUnZipBytes(bufferSize: Long, format: Int, bytes: ByteArray): ByteArray
    }
}