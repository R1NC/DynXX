package xyz.rinc.ngenxx.demo

import android.app.Application
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import xyz.rinc.ngenxx.NGenXX
import xyz.rinc.ngenxx.NGenXXHelper
import xyz.rinc.ngenxx.NGenXXHelper.Companion.LogLevel
import xyz.rinc.ngenxx.NGenXXHelper.Companion.HttpMethod
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream

class NGenXXTest {
    companion object {

        private var sApplication: Application? = null
        private var sJsLoaded: Boolean = false

        fun init(application: Application) {
            sApplication = application
            val dir = sApplication?.filesDir?.absolutePath ?: return
            if (!NGenXX.init(dir)) return

            NGenXXHelper.logSetLevel(LogLevel.Debug)

            NGenXX.jSetMsgCallback { msg ->
                "Android->$msg@${System.currentTimeMillis()}"
            }
        }

        fun goAllAsync(count: Int) = runBlocking {
            goAll(count)
        }

        private suspend fun goAll(count: Int) = coroutineScope {
            if (count < 1) return@coroutineScope
            for (i in 1 .. count) {
                launch {
                    go()
                }
            }
        }

        private fun go() {
            NGenXXHelper.logSetLevel(LogLevel.Debug)
            NGenXXHelper.logPrint(LogLevel.Debug, "deviceType:${NGenXX.deviceType()}")
            NGenXXHelper.logPrint(LogLevel.Debug, "deviceName:${NGenXX.deviceName()}")
            NGenXXHelper.logPrint(LogLevel.Debug, "deviceManufacturer:${NGenXX.deviceManufacturer()}")
            NGenXXHelper.logPrint(LogLevel.Debug, "deviceOsVersion:${NGenXX.deviceOsVersion()}")
            NGenXXHelper.logPrint(LogLevel.Debug, "deviceCpuArch:${NGenXX.deviceCpuArch()}")

            val inputStr = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM<>()[]{}@#$%^&*-=+~!/|_,;:'`"
            val inputBytes = inputStr.toByteArray(Charsets.UTF_8)
            val keyStr = "MNBVCXZLKJHGFDSA"
            val keyBytes = keyStr.toByteArray(Charsets.UTF_8)
            val aesEncodedBytes = NGenXX.cryptoAesEncrypt(inputBytes, keyBytes)
            val aesDecodedBytes = NGenXX.cryptoAesDecrypt(aesEncodedBytes, keyBytes)
            val aesDecodedStr = aesDecodedBytes.toString(Charsets.UTF_8)
            NGenXXHelper.logPrint(LogLevel.Debug,"AES->$aesDecodedStr")
            val aesgcmIV = NGenXX.cryptoRandom(12)
            val aesgcmAad = null
            val aesgcmTagBits = 15 * 8
            val aesgcmEncodedBytes = NGenXX.cryptoAesGcmEncrypt(inputBytes, keyBytes, aesgcmIV,
                aesgcmAad, aesgcmTagBits)
            val aesgcmDecodedBytes = NGenXX.cryptoAesGcmDecrypt(aesgcmEncodedBytes, keyBytes, aesgcmIV,
                aesgcmAad, aesgcmTagBits)
            val aesgcmDecodedStr = aesgcmDecodedBytes.toString(Charsets.UTF_8)
            NGenXXHelper.logPrint(LogLevel.Debug,"AES-GCM->$aesgcmDecodedStr")
            val md5Bytes = NGenXX.cryptoHashMd5(inputBytes)
            val md5hex = NGenXX.codingHexBytes2Str(md5Bytes)
            NGenXXHelper.logPrint(LogLevel.Debug,"md5->$md5hex")
            val sha256bytes = NGenXX.cryptoHashSha256(inputBytes)
            val sha256hex = NGenXX.codingHexBytes2Str(sha256bytes)
            NGenXXHelper.logPrint(LogLevel.Debug,"sha256->$sha256hex")
            val base64Encoded = NGenXX.cryptoBase64Encode(inputBytes)
            val base64Decoded = NGenXX.cryptoBase64Decode(base64Encoded)
            val base64DecodedStr = base64Decoded.toString(Charsets.UTF_8)
            NGenXXHelper.logPrint(LogLevel.Debug,"base64->$base64DecodedStr")

            val jsonParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"header_v\":[\"Cache-Control: no-cache\"], \"header_c\":1, \"timeout\":6666}"

            sApplication?.assets?.open("biz.lua")?.bufferedReader().use {
                if (!NGenXX.lLoadS(it?.readText()?:"")) return
                val rsp = NGenXX.lCall("lTestNetHttpRequest", "https://rinc.xyz")
                NGenXXHelper.logPrint(LogLevel.Debug, rsp!!)
            }

            if (sJsLoaded) {
                NGenXX.jCall("jTestBase64", "NGenXX", false)
            } else {
                sApplication?.assets?.open("NGenXX.js")?.bufferedReader().use { it0 ->
                    if (!NGenXX.jLoadS(it0?.readText() ?: "", "NGenXX.js", false)) return
                    sApplication?.assets?.open("biz.js")?.bufferedReader().use { it1 ->
                        if (!NGenXX.jLoadS(it1?.readText() ?: "", "biz.js", false)) return
                        NGenXX.jCall("jTestBase64", "NGenXX", false)
                    }
                }
            }

            val jsonDecoder = NGenXX.jsonDecoderInit(jsonParams)
            if (jsonDecoder != 0L) {
                val urlNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "url")
                if (urlNode != 0L) {
                    val url = NGenXX.jsonDecoderReadString(jsonDecoder, urlNode)
                    NGenXXHelper.logPrint(LogLevel.Debug, "url:$url")
                }
                val headerCNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "header_c")
                if (headerCNode != 0L) {
                    val headerC = NGenXX.jsonDecoderReadNumber(jsonDecoder, headerCNode)
                    NGenXXHelper.logPrint(LogLevel.Debug, "header_c:${headerC.toInt()}")
                }
                val headerVNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "header_v")
                if (headerVNode != 0L) {
                    var headerNode = NGenXX.jsonDecoderReadChild(jsonDecoder, headerVNode)
                    while (headerNode != 0L) {
                        val header = NGenXX.jsonDecoderReadString(jsonDecoder, headerNode)
                        NGenXXHelper.logPrint(LogLevel.Debug, "header:$header")
                        headerNode = NGenXX.jsonDecoderReadNext(jsonDecoder, headerNode)
                    }
                }
                NGenXX.jsonDecoderRelease(jsonDecoder)
            }

            val kvConn = NGenXX.storeKVOpen("test")
            if (kvConn != 0L) {
                NGenXX.storeKVWriteString(kvConn,"s", "NGenXX")
                val s = NGenXX.storeKVReadString(kvConn,"s")
                NGenXXHelper.logPrint(LogLevel.Debug, "s->$s")
                NGenXX.storeKVWriteInteger(kvConn,"i", 1234567890)
                val i = NGenXX.storeKVReadInteger(kvConn,"i")
                NGenXXHelper.logPrint(LogLevel.Debug, "i->$i")
                NGenXX.storeKVWriteFloat(kvConn,"f", 0.123456789)
                val f = NGenXX.storeKVReadFloat(kvConn,"f")
                NGenXXHelper.logPrint(LogLevel.Debug, "f->$f")
                NGenXX.storeKVClose(kvConn)
            }

            val dbConn = NGenXX.storeSQLiteOpen("test")
            if (dbConn != 0L) {
                val prepareSQL = sApplication?.assets?.open("prepare_data.sql")?.bufferedReader().use {
                    it?.readText()
                }
                NGenXX.storeSQLiteExecute(dbConn, prepareSQL!!)
                val querySQL = sApplication?.assets?.open("query.sql")?.bufferedReader().use {
                    it?.readText()
                }
                val queryResult = NGenXX.storeSQLiteQueryDo(dbConn, querySQL!!)
                if (queryResult != 0L) {
                    while (NGenXX.storeSQLiteQueryReadRow(queryResult)) {
                        val s = NGenXX.storeSQLiteQueryReadColumnText(queryResult, "platform");
                        val i = NGenXX.storeSQLiteQueryReadColumnInteger(queryResult, "i")
                        val f = NGenXX.storeSQLiteQueryReadColumnFloat(queryResult, "f")
                        NGenXXHelper.logPrint(LogLevel.Debug,"$s->$i->$f")
                    }
                    NGenXX.storeSQLiteQueryDrop(queryResult)
                }
                NGenXX.storeSQLiteClose(dbConn)
            }

            sApplication?.assets?.open("prepare_data.sql").use { zipInStream ->
                val zipFile = File(sApplication?.externalCacheDir, "x.zip")
                if (!zipFile.exists()) {
                    zipFile.delete()
                }
                zipFile.createNewFile()
                FileOutputStream(zipFile).use { zipOutStream ->
                    val zipSuccess = NGenXXHelper.zZip(zipInStream!!, zipOutStream)
                    if (zipSuccess) {
                        FileInputStream(zipFile).use { unzipInStream ->
                            val unzipFile = File(sApplication?.externalCacheDir, "x.txt")
                            if (!unzipFile.exists()) {
                                unzipFile.delete()
                            }
                            unzipFile.createNewFile()
                            FileOutputStream(unzipFile).use { unzipOutStream ->
                                val unzipSuccess = NGenXXHelper.zUnZip(unzipInStream, unzipOutStream)
                            }
                        }
                    }
                }
            }
        }
    }
}