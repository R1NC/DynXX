package xyz.rinc.ngenxx

class NGenXX {
    companion object {
        init {
            System.loadLibrary("NGenXXJNI")
        }

        external fun getVersion(): String

        external fun logSetLevel(level: Int)

        external fun logSetCallback(callback: ((level: Int, content: String) -> Unit)?)

        external fun logPrint(level: Int, content: String)

        external fun netHttpReq(url: String, params: String?): String?

        external fun lCreate(): Long

        external fun lDestroy(lState: Long)

        external fun lLoadF(lState: Long, file: String): Int

        external fun lLoadS(lState: Long, script: String): Int

        external fun lCall(lState: Long, func : String, params :String?): String?
    }
}