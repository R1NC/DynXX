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
    }
}