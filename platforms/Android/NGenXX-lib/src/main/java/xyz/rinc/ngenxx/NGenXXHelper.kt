package xyz.rinc.ngenxx

import java.io.File
import java.io.InputStream
import java.io.OutputStream

class NGenXXHelper {
    companion object {

        enum class LogLevel(val value: Int) {
            Debug(3),
            Info(4),
            Warn(5),
            Error(6),
            Fatal(7),
            None(8)
        }

        fun logSetLevel(level: LogLevel) {
            NGenXX.logSetLevel(level.value)
        }

        fun logPrint(level: LogLevel, content: String) {
            NGenXX.logPrint(level.value, content)
        }

        enum class HttpMethod(val value: Int) {
            Get(0),
            Post(1),
            Put(2),
        }

        data class HttpFormField(val name: String, val mime: String, val data: String)
        fun netHttpRequest(url: String,
                           paramMap: Map<String, String>?,
                           method: HttpMethod,
                           headerMap: Map<String, String>?,
                           formFieldV: Array<HttpFormField>?,
                           file: File?,
                           timeout: Long): String? {
            val paramSB = StringBuilder()
            paramMap?.forEach { h ->
                paramSB.append(if (paramSB.isEmpty()) "?" else "&")
                paramSB.append("${h.key}=${h.value}")
            }

            val headerL = mutableListOf<String>()
            headerMap?.forEach { h ->
                headerL.add("${h.key}:${h.value}")
            }

            val formFieldNameL = mutableListOf<String>()
            val formFieldMimeL = mutableListOf<String>()
            val formFieldDataL = mutableListOf<String>()
            formFieldV?.forEach { ff ->
                formFieldNameL.add(ff.name)
                formFieldMimeL.add(ff.mime)
                formFieldDataL.add(ff.data)
            }

            return NGenXX.netHttpRequest(url, paramSB.toString(), method.value,
                headerL.toTypedArray(),
                formFieldNameL.toTypedArray(),
                formFieldMimeL.toTypedArray(),
                formFieldDataL.toTypedArray(),
                file?.absolutePath,
                file?.length() ?: 0,
                timeout
            )
        }

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

        fun zZip(mode: ZipMode, format: ZFormat, inStream: InputStream, outStream: OutputStream): Boolean {
            val zip = NGenXX.zZipInit(mode.value, Z_BUFFER_SIZE.toLong(), format.ordinal)
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

        fun zUnZip(format: ZFormat, inStream: InputStream, outStream: OutputStream): Boolean {
            val unzip = NGenXX.zUnZipInit(Z_BUFFER_SIZE.toLong(), format.ordinal)
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