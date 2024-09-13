package xyz.rinc.ngenxx

class NGenXX {
    companion object {
        init {
            System.loadLibrary("NGenXXJNI")
        }

        external fun getVersion(): String
        external fun init(root: String): Boolean
        external fun release()

        external fun logSetLevel(level: Int)
        external fun logSetCallback(callback: ((level: Int, content: String) -> Unit)?)
        external fun logPrint(level: Int, content: String)

        external fun netHttpRequest(url: String, params: String?, method: Int, headers: Array<String>?, timeout: Long): String?

        external fun lLoadF(file: String): Boolean
        external fun lLoadS(script: String): Boolean
        external fun lCall(func : String, params :String?): String?

        external fun storeSQLiteOpen(file: String): Long
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
    }
}