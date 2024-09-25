package xyz.rinc.ngenxx

import java.io.InputStream
import java.io.OutputStream

class NGenXXHelper {
    companion object {

        const val Z_BUFFER_SIZE = 16 * 1024

        fun zZipF(mode: NGenXX.Companion.ZipMode, inStream: InputStream, outStream: OutputStream): Boolean {
            val inBuffer = ByteArray(Z_BUFFER_SIZE)
            var inLen: Int

            val zip = NGenXX.zZipInit(mode.value, inBuffer.size.toLong())
            if (zip > 0) {
                var inputFinished: Boolean
                do {
                    inLen = inStream.read(inBuffer, 0, inBuffer.size)
                    inputFinished = inLen < inBuffer.size
                    val ret = NGenXX.zZipInput(zip, inBuffer, inLen, inputFinished)
                    if (ret == 0L) {
                        return false
                    }

                    var processFinished: Boolean
                    do {
                        val outBytes = NGenXX.zZipProcessDo(zip)
                        if (outBytes.isEmpty()) {
                            return false
                        }
                        processFinished = NGenXX.zZipProcessFinished(zip)

                        outStream.write(outBytes)
                    } while(!processFinished)
                } while (!inputFinished)

                outStream.flush()
                NGenXX.zZipRelease(zip)
                return true
            }
            return false
        }

        fun zUnZipF(inStream: InputStream, outStream: OutputStream): Boolean {
            val inBuffer = ByteArray(Z_BUFFER_SIZE)
            var inLen: Int

            val unzip = NGenXX.zUnZipInit(Z_BUFFER_SIZE.toLong())
            if (unzip > 0) {
                var inputFinished: Boolean
                do {
                    inLen = inStream.read(inBuffer, 0, inBuffer.size)
                    inputFinished = inLen == -1
                    val ret = NGenXX.zUnZipInput(unzip, inBuffer, inLen, inputFinished)
                    if (ret == 0L) {
                        return false
                    }

                    var processFinished: Boolean
                    do {
                        val outBytes = NGenXX.zUnZipProcessDo(unzip)
                        if (outBytes.isEmpty()) {
                            return false
                        }
                        processFinished = NGenXX.zUnZipProcessFinished(unzip)

                        outStream.write(outBytes)
                    } while(!processFinished)
                } while (!inputFinished)

                outStream.flush()
                NGenXX.zUnZipRelease(unzip)
                return true
            }
            return false
        }
    }
}