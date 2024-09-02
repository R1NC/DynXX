package xyz.rinc.ngenxx

class NGenXX {
    companion object {
        init {
            System.loadLibrary("NGenXXJNI")
        }

        external fun getVersion(): String
        external fun init(useLua: Boolean): Long
        external fun release(handle: Long)

        external fun logSetLevel(level: Int)
        external fun logSetCallback(callback: ((level: Int, content: String) -> Unit)?)
        external fun logPrint(level: Int, content: String)

        external fun netHttpReq(url: String, params: String?): String?

        external fun lLoadF(handle: Long, file: String): Boolean
        external fun lLoadS(handle: Long, script: String): Boolean
        external fun lCall(handle: Long, func : String, params :String?): String?
    }
}