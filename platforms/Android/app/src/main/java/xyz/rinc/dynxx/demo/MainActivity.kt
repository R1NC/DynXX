package xyz.rinc.dynxx.demo

import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.result.contract.ActivityResultContracts
import androidx.core.content.ContextCompat
import xyz.rinc.dynxx.DynXX

class MainActivity : ComponentActivity() {

    private val reqPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { _ ->
        DynXXTest.init(application)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (Build.VERSION.SDK_INT < 32 &&
            ContextCompat.checkSelfPermission(this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            reqPermissionLauncher.launch(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
        } else {
            DynXXTest.init(application)
        }

        DynXXTest.goAllAsync(100)
    }

    override fun onDestroy() {
        super.onDestroy()
        DynXX.release()
    }
}