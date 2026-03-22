plugins {
    alias(libs.plugins.android.library)
}

android {
    namespace = "xyz.rinc.dynxx"
    compileSdk = 36

    defaultConfig {
        minSdk = 24

        externalNativeBuild {
            cmake {
                cppFlags("")
            }
        }
        ndk {
            //noinspection ChromeOsAbiSupport
            abiFilters.add("arm64-v8a")
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlin {
        jvmToolchain(17)
    }

    externalNativeBuild {
        cmake {
            version = "4.1.2"
            path = file("src/main/cxx/CMakeLists.txt")
        }
    }
    ndkVersion = "30.0.14904198"
}

dependencies {
    implementation(libs.androidx.core.ktx)
}
