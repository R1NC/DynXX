package xyz.rinc.ngenxx

import java.io.InputStream
import java.io.OutputStream

class NGenXXHelper {
    companion object {

        private const val Z_BUFFER_SIZE = 16 * 1024

        private fun zProcess(bufferSize: Int, inStream: InputStream, outStream: OutputStream,
                             inputF: ((inBuffer: ByteArray, inLen: Int, inputFinished: Boolean) -> Long),
                             processDoF: (() -> ByteArray),
                             processFinishedF: (() -> Boolean)): Boolean {
            val inBuffer = ByteArray(bufferSize)
            var inLen: Int

            var inputFinished: Boolean
            do {
                inLen = inStream.read(inBuffer, 0, inBuffer.size)
                inputFinished = inLen < bufferSize
                val ret = inputF(inBuffer, inLen, inputFinished);
                if (ret == 0L) {
                    return false
                }

                var processFinished: Boolean
                do {
                    val outBytes = processDoF()
                    if (outBytes.isEmpty()) {
                        return false
                    }
                    processFinished = processFinishedF()

                    outStream.write(outBytes)
                } while(!processFinished)
            } while (!inputFinished)
            outStream.flush()

            return true
        }

        fun zZipF(mode: NGenXX.Companion.ZipMode, inStream: InputStream, outStream: OutputStream): Boolean {
            val zip = NGenXX.zZipInit(mode.value, Z_BUFFER_SIZE.toLong())
            if (zip <= 0) return false

            val success = zProcess(Z_BUFFER_SIZE, inStream, outStream, {inBuffer: ByteArray, inLen: Int, inputFinished: Boolean ->
                NGenXX.zZipInput(zip, inBuffer, inLen, inputFinished)
            }, {
                NGenXX.zZipProcessDo(zip)
            }, {
                NGenXX.zZipProcessFinished(zip)
            })

            NGenXX.zZipRelease(zip)
            return success
        }

        fun zUnZipF(inStream: InputStream, outStream: OutputStream): Boolean {
            val unzip = NGenXX.zUnZipInit(Z_BUFFER_SIZE.toLong())
            if (unzip <= 0) return false

            val success = zProcess(Z_BUFFER_SIZE, inStream, outStream, {inBuffer: ByteArray, inLen: Int, inputFinished: Boolean ->
                NGenXX.zUnZipInput(unzip, inBuffer, inLen, inputFinished)
            }, {
                NGenXX.zUnZipProcessDo(unzip)
            }, {
                NGenXX.zUnZipProcessFinished(unzip)
            })

            NGenXX.zUnZipRelease(unzip)
            return success
        }
    }
}