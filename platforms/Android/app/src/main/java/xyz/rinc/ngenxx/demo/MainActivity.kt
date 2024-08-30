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

        NGenXX.logSetCallback {level, content ->
            android.util.Log.d("NGenXX", "$level | $content")
        }

        val lState = NGenXX.lCreate()
        val luaScript = application.assets.open("biz.lua").bufferedReader().use {
            it.readText()
        }
        NGenXX.lLoadS(lState, luaScript)

        val params = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}"
        val rsp = NGenXX.lCall(lState, "lNetHttpReq", params)

        NGenXX.lDestroy(lState)

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