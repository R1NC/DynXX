package xyz.rinc.ngenxx.demo

import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.core.content.ContextCompat
import xyz.rinc.ngenxx.NGenXX
import xyz.rinc.ngenxx.NGenXXHelper
import xyz.rinc.ngenxx.demo.ui.theme.NGenXXTheme
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream

class MainActivity : ComponentActivity() {

    private val reqPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { isGranted ->
        if (isGranted) {
            testNGenXX()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        if (ContextCompat.checkSelfPermission(this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            reqPermissionLauncher.launch(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
        } else {
            testNGenXX()
        }

        setContent {
            NGenXXTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    Greeting(
                        txt = "",
                        modifier = Modifier.padding(innerPadding)
                    )
                }
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        NGenXX.release()
    }

    private fun testNGenXX() {
        val dir = filesDir?.absolutePath ?: return
        if (!NGenXX.init(dir)) return

        NGenXX.logSetCallback {level, content ->
            Log.d("NGenXX", "$level | $content")
        }

        NGenXX.logPrint(1, "deviceType:${NGenXX.deviceType()}")
        NGenXX.logPrint(1, "deviceName:${NGenXX.deviceName()}")
        NGenXX.logPrint(1, "deviceManufacturer:${NGenXX.deviceManufacturer()}")
        NGenXX.logPrint(1, "deviceOsVersion:${NGenXX.deviceOsVersion()}")
        NGenXX.logPrint(1, "deviceCpuArch:${NGenXX.deviceCpuArch()}")

        val inputStr = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM<>()[]{}@#$%^&*-=+~!/|_,;:'`"
        val inputBytes = inputStr.toByteArray(Charsets.UTF_8)
        val keyStr = "MNBVCXZLKJHGFDSA"
        val keyBytes = keyStr.toByteArray(Charsets.UTF_8)
        val aesEncodedBytes = NGenXX.cryptoAesEncrypt(inputBytes, keyBytes)
        val aesDecodedBytes = NGenXX.cryptoAesDecrypt(aesEncodedBytes, keyBytes)
        val aesDecodedStr = aesDecodedBytes.toString(Charsets.UTF_8)
        NGenXX.logPrint(1,"AES->$aesDecodedStr")
        val aesgcmIV = NGenXX.cryptoRandom(12)
        val aesgcmAad = null
        val aesgcmTagBits = 15 * 8
        val aesgcmEncodedBytes = NGenXX.cryptoAesGcmEncrypt(inputBytes, keyBytes, aesgcmIV,
            aesgcmAad, aesgcmTagBits)
        val aesgcmDecodedBytes = NGenXX.cryptoAesGcmDecrypt(aesgcmEncodedBytes, keyBytes, aesgcmIV,
            aesgcmAad, aesgcmTagBits)
        val aesgcmDecodedStr = aesgcmDecodedBytes.toString(Charsets.UTF_8)
        NGenXX.logPrint(1,"AES-GCM->$aesgcmDecodedStr")
        val md5Bytes = NGenXX.cryptoHashMd5(inputBytes)
        val md5hex = NGenXX.codingHexBytes2Str(md5Bytes)
        NGenXX.logPrint(1,"md5->$md5hex")
        val sha256bytes = NGenXX.cryptoHashSha256(inputBytes)
        val sha256hex = NGenXX.codingHexBytes2Str(sha256bytes)
        NGenXX.logPrint(1,"sha256->$sha256hex")
        val base64Encoded = NGenXX.cryptoBase64Encode(inputBytes)
        val base64Decoded = NGenXX.cryptoBase64Decode(base64Encoded)
        val base64DecodedStr = base64Decoded.toString(Charsets.UTF_8)
        NGenXX.logPrint(1,"base64->$base64DecodedStr")

        val jsonParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"header_v\":[\"Cache-Control: no-cache\"], \"header_c\":1, \"timeout\":6666}"

        application.assets.open("biz.lua").bufferedReader().use {
            if (!NGenXX.lLoadS(it.readText())) return
            val rsp = NGenXX.lCall("lNetHttpRequest", jsonParams)
            /*val rsp = NGenXX.netHttpRequest("https://rinc.xyz",
                "p0=1&p1=2&p2=3",
                0,
                arrayOf("Cache-Control: no-cache"),
                5555
            )*/
            NGenXX.logPrint(1, rsp!!)
        }

        application.assets.open("NGenXX.js").bufferedReader().use { it0 ->
            if (!NGenXX.jLoadS(it0.readText(), "NGenXX.js")) return
            application.assets.open("biz.js").bufferedReader().use { it1 ->
                if (!NGenXX.jLoadS(it1.readText(), "biz.js")) return
                NGenXX.jCall("jTestBase64", "NGenXX")
            }
        }

        val jsonDecoder = NGenXX.jsonDecoderInit(jsonParams)
        if (jsonDecoder > 0) {
            val urlNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "url")
            if (urlNode > 0) {
                val url = NGenXX.jsonDecoderReadString(jsonDecoder, urlNode)
                NGenXX.logPrint(1, "url:$url")
            }
            val headerCNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "header_c")
            if (headerCNode > 0) {
                val headerC = NGenXX.jsonDecoderReadNumber(jsonDecoder, headerCNode)
                NGenXX.logPrint(1, "header_c:${headerC.toInt()}")
            }
            val headerVNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "header_v")
            if (headerVNode > 0) {
                var headerNode = NGenXX.jsonDecoderReadChild(jsonDecoder, headerVNode)
                while (headerNode > 0) {
                    val header = NGenXX.jsonDecoderReadString(jsonDecoder, headerNode)
                    NGenXX.logPrint(1, "header:$header")
                    headerNode = NGenXX.jsonDecoderReadNext(jsonDecoder, headerNode)
                }
            }
            NGenXX.jsonDecoderRelease(jsonDecoder)
        }

        val kvConn = NGenXX.storeKVOpen("test")
        if (kvConn > 0) {
            NGenXX.storeKVWriteString(kvConn,"s", "NGenXX")
            val s = NGenXX.storeKVReadString(kvConn,"s")
            NGenXX.logPrint(1, "s->$s")
            NGenXX.storeKVWriteInteger(kvConn,"i", 1234567890)
            val i = NGenXX.storeKVReadInteger(kvConn,"i")
            NGenXX.logPrint(1, "i->$i")
            NGenXX.storeKVWriteFloat(kvConn,"f", 0.123456789)
            val f = NGenXX.storeKVReadFloat(kvConn,"f")
            NGenXX.logPrint(1, "f->$f")
            NGenXX.storeKVClose(kvConn)
        }

        val dbConn = NGenXX.storeSQLiteOpen("test")
        if (dbConn > 0) {
            val prepareSQL = application.assets.open("prepare_data.sql").bufferedReader().use {
                it.readText()
            }
            NGenXX.storeSQLiteExecute(dbConn, prepareSQL)
            val querySQL = application.assets.open("query.sql").bufferedReader().use {
                it.readText()
            }
            val queryResult = NGenXX.storeSQLiteQueryDo(dbConn, querySQL)
            if (queryResult > 0) {
                while (NGenXX.storeSQLiteQueryReadRow(queryResult)) {
                    val s = NGenXX.storeSQLiteQueryReadColumnText(queryResult, "platform");
                    val i = NGenXX.storeSQLiteQueryReadColumnInteger(queryResult, "i")
                    val f = NGenXX.storeSQLiteQueryReadColumnFloat(queryResult, "f")
                    NGenXX.logPrint(1,"$s->$i->$f")
                }
                NGenXX.storeSQLiteQueryDrop(queryResult)
            }
            NGenXX.storeSQLiteClose(dbConn)
        }

        assets.open("prepare_data.sql").use { zipInStream ->
            val zipFile = File(externalCacheDir, "x.zip")
            if (!zipFile.exists()) {
                zipFile.delete()
            }
            zipFile.createNewFile()
            FileOutputStream(zipFile).use { zipOutStream ->
                val zipSuccess = NGenXXHelper.zZip(NGenXX.Companion.ZipMode.Default, NGenXX.Companion.ZFormat.GZip, zipInStream, zipOutStream)
                if (zipSuccess) {
                    FileInputStream(zipFile).use { unzipInStream ->
                        val unzipFile = File(externalCacheDir, "x.txt")
                        if (!unzipFile.exists()) {
                            unzipFile.delete()
                        }
                        unzipFile.createNewFile()
                        FileOutputStream(unzipFile).use { unzipOutStream ->
                            val unzipSuccess = NGenXXHelper.zUnZip(NGenXX.Companion.ZFormat.GZip, unzipInStream, unzipOutStream)
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun Greeting(txt: String, modifier: Modifier = Modifier) {
    Text(
        text = txt,
        modifier = modifier
    )
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    NGenXXTheme {
        Greeting("Android")
    }
}