package xyz.rinc.enginexx.demo

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
import xyz.rinc.enginexx.EngineXX
import xyz.rinc.enginexx.demo.ui.theme.EngineXXDemoTheme

class MainActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        val lState = EngineXX.lCreate()
        val luaScript = application.assets.open("biz.lua").bufferedReader().use {
            it.readText()
        }
        EngineXX.lLoadS(lState, luaScript)

        val v = EngineXX.lCall(lState, "lTestGetVersion", null)

        val params = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}"
        val rsp = EngineXX.lCall(lState, "lTestHttpReq", params)

        EngineXX.lDestroy(lState)

        setContent {
            EngineXXDemoTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    Greeting(
                        txt = "$v\n\n$rsp",
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
    EngineXXDemoTheme {
        Greeting("Android")
    }
}