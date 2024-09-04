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

    private var handle = -1L

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        if (handle == -1L) {
            handle = NGenXX.init()

            NGenXX.logSetCallback {level, content ->
                android.util.Log.d("NGenXX", "$level | $content")
            }

            val luaScript = application.assets.open("biz.lua").bufferedReader().use {
                it.readText()
            }
            NGenXX.lLoadS(handle, luaScript)
        }

        //val params = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}"
        //val rsp = NGenXX.lCall(handle, "lNetHttpReq", params)
        val rsp = NGenXX.netHttpRequest("https://rinc.xyz",
            "p0=1&p1=2&p2=3",
            1,
            arrayOf("Accept-Encoding: gzip, deflate", "Cache-Control: no-cache"),
            5555
        )

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
        NGenXX.release(handle)
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