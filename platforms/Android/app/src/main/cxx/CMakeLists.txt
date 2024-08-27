cmake_minimum_required(VERSION 3.22.1)

project("EngineXXJNI")

add_library(${CMAKE_PROJECT_NAME} SHARED EngineXXJNI.cxx)

set(libDir ${PROJECT_SOURCE_DIR}/../../../../../../build.Android/output/Release/${ANDROID_ABI})

#Find libcrypto
#set(libcryptoPath ${libDir}/libcrypto.a)
#add_library(libcrypto STATIC IMPORTED)
#set_property(TARGET libcrypto PROPERTY IMPORTED_LOCATION ${libcryptoPath})

#Find libssl
#set(libsslPath ${libDir}/libssl.a)
#add_library(libssl STATIC IMPORTED)
#set_property(TARGET libssl PROPERTY IMPORTED_LOCATION ${libsslPath})

#Find libwolfssl
set(libwolfsslPath ${libDir}/libwolfssl.a)
add_library(libwolfssl STATIC IMPORTED)
set_property(TARGET libwolfssl PROPERTY IMPORTED_LOCATION ${libwolfsslPath})

#Find libcurl
set(libcurlPath ${libDir}/libcurl.a)
add_library(libcurl STATIC IMPORTED)
set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION ${libcurlPath})

#Find liblua
set(libluaPath ${libDir}/liblua.a)
add_library(liblua STATIC IMPORTED)
set_property(TARGET liblua PROPERTY IMPORTED_LOCATION ${libluaPath})

#Find libcjson
set(libcjsonPath ${libDir}/libcjson.a)
add_library(libcjson STATIC IMPORTED)
set_property(TARGET libcjson PROPERTY IMPORTED_LOCATION ${libcjsonPath})

#Find libEngineXX
set(libEngineXXPath ${libDir}/libEngineXX.a)
add_library(libEngineXX STATIC IMPORTED)
set_property(TARGET libEngineXX PROPERTY IMPORTED_LOCATION ${libEngineXXPath})

target_link_libraries(${CMAKE_PROJECT_NAME}
        #libcrypto
        #libssl
        libwolfssl
        libcurl
        liblua
        libcjson
        libEngineXX
        z
        android
        log
)