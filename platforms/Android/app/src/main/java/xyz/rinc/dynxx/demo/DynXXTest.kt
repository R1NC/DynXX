package xyz.rinc.dynxx.demo

import android.app.Application
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import xyz.rinc.dynxx.DynXX
import xyz.rinc.dynxx.DynXXHelper
import xyz.rinc.dynxx.DynXXHelper.Companion.LogLevel
import xyz.rinc.dynxx.DynXXHelper.Companion.HttpMethod
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream

class DynXXTest {
    companion object {

        private var sApplication: Application? = null
        private var sJsLoaded: Boolean = false
        private var sLuaLoaded: Boolean = false

        fun init(application: Application) {
            DynXXHelper.logSetLevel(LogLevel.Debug)
            
            sApplication = application
            val dir = sApplication?.filesDir?.absolutePath ?: return
            if (!DynXX.init(dir)) return

            DynXX.jSetMsgCallback { msg ->
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
            DynXXHelper.logSetLevel(LogLevel.Debug)
            /*DynXX.logSetCallback{ level, msg ->
                android.util.Log.d("@_@", "$level | $msg")
            }*/

            DynXXHelper.logPrint(LogLevel.Debug, "deviceType:${DynXX.deviceType()}")
            DynXXHelper.logPrint(LogLevel.Debug, "deviceName:${DynXX.deviceName()}")
            DynXXHelper.logPrint(LogLevel.Debug, "deviceManufacturer:${DynXX.deviceManufacturer()}")
            DynXXHelper.logPrint(LogLevel.Debug, "deviceOsVersion:${DynXX.deviceOsVersion()}")
            DynXXHelper.logPrint(LogLevel.Debug, "deviceCpuArch:${DynXX.deviceCpuArch()}")

            val inputStr = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM<>()[]{}@#$%^&*-=+~!/|_,;:'`"
            val inputBytes = inputStr.toByteArray(Charsets.UTF_8)
            val keyStr = "MNBVCXZLKJHGFDSA"
            val keyBytes = keyStr.toByteArray(Charsets.UTF_8)
            val aesEncodedBytes = DynXX.cryptoAesEncrypt(inputBytes, keyBytes)
            val aesDecodedBytes = DynXX.cryptoAesDecrypt(aesEncodedBytes, keyBytes)
            val aesDecodedStr = aesDecodedBytes.toString(Charsets.UTF_8)
            DynXXHelper.logPrint(LogLevel.Debug,"AES->$aesDecodedStr")
            val aesgcmIV = DynXX.cryptoRandom(12)
            val aesgcmAad = null
            val aesgcmTagBits = 15 * 8
            val aesgcmEncodedBytes = DynXX.cryptoAesGcmEncrypt(inputBytes, keyBytes, aesgcmIV,
                aesgcmAad, aesgcmTagBits)
            val aesgcmDecodedBytes = DynXX.cryptoAesGcmDecrypt(aesgcmEncodedBytes, keyBytes, aesgcmIV,
                aesgcmAad, aesgcmTagBits)
            val aesgcmDecodedStr = aesgcmDecodedBytes.toString(Charsets.UTF_8)
            DynXXHelper.logPrint(LogLevel.Debug,"AES-GCM->$aesgcmDecodedStr")
            val md5Bytes = DynXX.cryptoHashMd5(inputBytes)
            val md5hex = DynXX.codingHexBytes2Str(md5Bytes)
            DynXXHelper.logPrint(LogLevel.Debug,"md5->$md5hex")
            val sha256bytes = DynXX.cryptoHashSha256(inputBytes)
            val sha256hex = DynXX.codingHexBytes2Str(sha256bytes)
            DynXXHelper.logPrint(LogLevel.Debug,"sha256->$sha256hex")
            val base64Encoded = DynXX.cryptoBase64Encode(inputBytes, true)
            val base64Decoded = DynXX.cryptoBase64Decode(base64Encoded, true)
            val base64DecodedStr = base64Decoded.toString(Charsets.UTF_8)
            DynXXHelper.logPrint(LogLevel.Debug,"base64->$base64DecodedStr")

            val jsonParams = """{
                "url": "https://rinc.xyz", 
                "params": "p0=1&p1=2&p2=3", 
                "method":0,
                "header_v": [
                    "Cache-Control: no-cache"
                ],
                "timeout": 6666
            }""".trimIndent()

            if (sLuaLoaded) {
                DynXXHelper.logPrint(LogLevel.Debug, DynXX.lCall("TestNetHttpRequest", "https://rinc.xyz")!!)
            } else {
                sApplication?.assets?.open("json.lua")?.bufferedReader().use { it0->
                    if (!DynXX.lLoadS(it0?.readText()?:"")) return
                    sApplication?.assets?.open("DynXX.lua")?.bufferedReader().use { it1->
                        if (!DynXX.lLoadS(it1?.readText()?:"")) return
                        sApplication?.assets?.open("biz.lua")?.bufferedReader().use { it2->
                            if (!DynXX.lLoadS(it2?.readText()?:"")) return
                            sLuaLoaded = true
                            DynXXHelper.logPrint(LogLevel.Debug, DynXX.lCall("TestNetHttpRequest", "https://rinc.xyz")!!)
                        }
                    }
                }
            }

            if (sJsLoaded) {
                DynXX.jCall("jTestCryptoBase64", "DynXX", false)
            } else {
                sApplication?.assets?.open("DynXX.js")?.bufferedReader().use { it0 ->
                    if (!DynXX.jLoadS(it0?.readText() ?: "", "DynXX.js", false)) return
                    sApplication?.assets?.open("biz.js")?.bufferedReader().use { it1 ->
                        if (!DynXX.jLoadS(it1?.readText() ?: "", "biz.js", false)) return
                        sJsLoaded = true
                        DynXX.jCall("jTestCryptoBase64", "DynXX", false)
                    }
                }
            }

            val jsonDecoder = DynXX.jsonDecoderInit(jsonParams)
            if (jsonDecoder != 0L) {
                val urlNode = DynXX.jsonDecoderReadNode(jsonDecoder, 0, "url")
                if (urlNode != 0L) {
                    val url = DynXX.jsonDecoderReadString(jsonDecoder, urlNode)
                    DynXXHelper.logPrint(LogLevel.Debug, "url:$url")
                }
                val methodNode = DynXX.jsonDecoderReadNode(jsonDecoder, 0, "method")
                if (methodNode != 0L) {
                    val method = DynXX.jsonDecoderReadInteger(jsonDecoder, methodNode)
                    DynXXHelper.logPrint(LogLevel.Debug, "method:${method.toInt()}")
                }
                val headerVNode = DynXX.jsonDecoderReadNode(jsonDecoder, 0, "header_v")
                if (headerVNode != 0L) {
                    var headerNode = DynXX.jsonDecoderReadChild(jsonDecoder, headerVNode)
                    while (headerNode != 0L) {
                        val header = DynXX.jsonDecoderReadString(jsonDecoder, headerNode)
                        DynXXHelper.logPrint(LogLevel.Debug, "header:$header")
                        headerNode = DynXX.jsonDecoderReadNext(jsonDecoder, headerNode)
                    }
                }
                DynXX.jsonDecoderRelease(jsonDecoder)
            }

            val kvConn = DynXX.kvOpen("test")
            if (kvConn != 0L) {
                DynXX.kvWriteString(kvConn,"s", "DynXX")
                val s = DynXX.kvReadString(kvConn,"s")
                DynXXHelper.logPrint(LogLevel.Debug, "s->$s")
                DynXX.kvWriteInteger(kvConn,"i", 1234567890)
                val i = DynXX.kvReadInteger(kvConn,"i")
                DynXXHelper.logPrint(LogLevel.Debug, "i->$i")
                DynXX.kvWriteFloat(kvConn,"f", 0.123456789)
                val f = DynXX.kvReadFloat(kvConn,"f")
                DynXXHelper.logPrint(LogLevel.Debug, "f->$f")
                DynXX.kvClose(kvConn)
            }

            val dbConn = DynXX.sqliteOpen("test")
            if (dbConn != 0L) {
                val prepareSQL = sApplication?.assets?.open("prepare_data.sql")?.bufferedReader().use {
                    it?.readText()
                }
                DynXX.sqliteExecute(dbConn, prepareSQL!!)
                val querySQL = sApplication?.assets?.open("query.sql")?.bufferedReader().use {
                    it?.readText()
                }
                val queryResult = DynXX.sqliteQueryDo(dbConn, querySQL!!)
                if (queryResult != 0L) {
                    while (DynXX.sqliteQueryReadRow(queryResult)) {
                        val s = DynXX.sqliteQueryReadColumnText(queryResult, "platform");
                        val i = DynXX.sqliteQueryReadColumnInteger(queryResult, "i")
                        val f = DynXX.sqliteQueryReadColumnFloat(queryResult, "f")
                        DynXXHelper.logPrint(LogLevel.Debug,"$s->$i->$f")
                    }
                    DynXX.sqliteQueryDrop(queryResult)
                }
                DynXX.sqliteClose(dbConn)
            }

            sApplication?.assets?.open("prepare_data.sql").use { zipInStream ->
                val zipFile = File(sApplication?.externalCacheDir, "x.zip")
                if (!zipFile.exists()) {
                    zipFile.delete()
                }
                zipFile.createNewFile()
                FileOutputStream(zipFile).use { zipOutStream ->
                    val zipSuccess = DynXXHelper.zZip(zipInStream!!, zipOutStream)
                    if (zipSuccess) {
                        FileInputStream(zipFile).use { unzipInStream ->
                            val unzipFile = File(sApplication?.externalCacheDir, "x.txt")
                            if (!unzipFile.exists()) {
                                unzipFile.delete()
                            }
                            unzipFile.createNewFile()
                            FileOutputStream(unzipFile).use { unzipOutStream ->
                                val unzipSuccess = DynXXHelper.zUnZip(unzipInStream, unzipOutStream)
                            }
                        }
                    }
                }
            }
        }
    }
}