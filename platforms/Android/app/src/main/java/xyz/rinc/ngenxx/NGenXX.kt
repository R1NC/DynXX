package xyz.rinc.ngenxx

class NGenXX {
    companion object {
        init {
            System.loadLibrary("NGenXXJNI")
        }

        external fun getVersion(): String
        external fun init(): Long
        external fun release(handle: Long)

        external fun logSetLevel(level: Int)
        external fun logSetCallback(callback: ((level: Int, content: String) -> Unit)?)
        external fun logPrint(level: Int, content: String)

        external fun netHttpRequest(url: String, params: String?, method: Int, headers: Array<String>?, timeout: Long): String?

        external fun lLoadF(handle: Long, file: String): Boolean
        external fun lLoadS(handle: Long, script: String): Boolean
        external fun lCall(handle: Long, func : String, params :String?): String?
    }
}