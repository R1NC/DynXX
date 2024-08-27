package xyz.rinc.enginexx

class EngineXX {
    companion object {
        init {
            System.loadLibrary("EngineXXJNI")
        }

        external fun getVersion(): String

        external fun httpRequest(url: String, params: String?): String?

        external fun lCreate(): Long

        external fun lDestroy(lState: Long)

        external fun lLoadF(lState: Long, file: String): Int

        external fun lLoadS(lState: Long, script: String): Int

        external fun lCall(lState: Long, func : String, params :String?): String?
    }
}