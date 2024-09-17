package xyz.rinc.ngenxx.demo

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import xyz.rinc.ngenxx.NGenXX
import xyz.rinc.ngenxx.demo.ui.theme.NGenXXTheme

class MainActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        
        val dir = filesDir?.absolutePath ?: return
        if (!NGenXX.init(dir)) return

        NGenXX.logSetCallback {level, content ->
            android.util.Log.d("NGenXX", "$level | $content")
        }

        NGenXX.logPrint(1, "deviceType:${NGenXX.deviceType()}")
        NGenXX.logPrint(1, "deviceName:${NGenXX.deviceName()}")
        NGenXX.logPrint(1, "deviceManufacturer:${NGenXX.deviceManufacturer()}")
        NGenXX.logPrint(1, "deviceOsVersion:${NGenXX.deviceOsVersion()}")
        NGenXX.logPrint(1, "deviceCpuArch:${NGenXX.deviceCpuArch()}")

        val luaScript = application.assets.open("biz.lua").bufferedReader().use {
            it.readText()
        }
        NGenXX.lLoadS(luaScript)

        val params = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"headers_v\":[\"Cache-Control: no-cache\"], \"headers_c\":1, \"timeout\":6666}"
        val rsp = NGenXX.lCall("lNetHttpRequest", params)
        /*val rsp = NGenXX.netHttpRequest("https://rinc.xyz",
            "p0=1&p1=2&p2=3",
            0,
            arrayOf("Cache-Control: no-cache"),
            5555
        )*/

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
                    val platform = NGenXX.storeSQLiteQueryReadColumnText(queryResult, "platform");
                    val vendor = NGenXX.storeSQLiteQueryReadColumnText(queryResult, "vendor");
                    NGenXX.logPrint(1,"$platform->$vendor")
                }
                NGenXX.storeSQLiteQueryDrop(queryResult)
            }
            NGenXX.storeSQLiteClose(dbConn)
        }

        setContent {
            NGenXXTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    Greeting(
                        txt = "$rsp",
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