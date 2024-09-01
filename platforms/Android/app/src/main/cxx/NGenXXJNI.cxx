#include <jni.h>
#include "../../../../../../build.Android/output/include/NGenXX.h"

extern "C"
JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_init(JNIEnv *env,
                                                         jobject thiz)
{
    ngenxx_init();
}

extern "C"
JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_release(JNIEnv *env,
                                                         jobject thiz)
{
    ngenxx_release();
}

extern "C"
JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_getVersion(JNIEnv *env,
                                                         jobject thiz)
{
    const char *cV = ngenxx_get_version();
    return env->NewStringUTF(cV);
}

#pragma mark Log

static JNIEnv *sEnv;
static jobject sLogCallback;
static jmethodID sLogCallbackMethodId;

static void ngenxx_jni_log_callback(int level, const char *content) {
    if (sEnv && sLogCallback && sLogCallbackMethodId) {
        jstring jContent = sEnv->NewStringUTF(content);
        sEnv->CallVoidMethod(sLogCallback, sLogCallbackMethodId, level, jContent);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_logSetLevel(JNIEnv *env, jobject thiz,
                                                                     jint level) {
    ngenxx_log_set_level(level);
}

extern "C"
JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_logSetCallback(JNIEnv *env, jobject thiz,
                                                                        jobject callback) {
    if (callback) {
        ngenxx_log_set_callback(ngenxx_jni_log_callback);
        sEnv = env;
        sLogCallback = env->NewWeakGlobalRef(callback);
        jclass jcallback_class = env->GetObjectClass(callback);
        if (jcallback_class) {
            sLogCallbackMethodId = env->GetMethodID(jcallback_class, "invoke", "(ILjava/lang/String;)V");
        }
    } else {
        ngenxx_log_set_callback(nullptr);
        sEnv = nullptr;
        env->DeleteWeakGlobalRef(sLogCallback);
        sLogCallback = nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_logPrint(JNIEnv *env, jobject thiz,
                                                                  jint level, jstring content) {
    const char* cContent = env->GetStringUTFChars(content, JNI_FALSE);
    ngenxx_log_print(level, cContent);
}

#pragma mark Net

extern "C"
JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_netHttpReq(JNIEnv *env,
                                                          jobject thiz,
                                                          jstring url,
                                                          jstring params)
{
    const char *cUrl = env->GetStringUTFChars(url, JNI_FALSE);
    const char *cParams = env->GetStringUTFChars(params, JNI_FALSE);
    const char *cRsp = ngenxx_net_http_request(cUrl, cParams);
    return env->NewStringUTF(cRsp);
}

#pragma mark Lua

extern "C"
JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_lCreate(JNIEnv *env, jobject thiz)
{
    return (jlong)ngenxx_L_create();
}

extern "C"
JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_lDestroy(JNIEnv *env, jobject thiz,
                                                       jlong lState)
{
    ngenxx_L_destroy((void *)lState);
}

extern "C"
JNIEXPORT jint JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_lLoadF(JNIEnv *env, jobject thiz,
                                                     jlong lState, jstring file)
{
    const char *cFile = env->GetStringUTFChars(file, JNI_FALSE);
    return ngenxx_L_loadF((void *)lState, cFile);
}

extern "C"
JNIEXPORT jint JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_lLoadS(JNIEnv *env, jobject thiz,
                                                     jlong lState, jstring script)
{
    const char *cScript = env->GetStringUTFChars(script, JNI_FALSE);
    return ngenxx_L_loadS((void *)lState, cScript);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_lCall(JNIEnv *env, jobject thiz,
                                                    jlong lState, jstring func,
                                                    jstring params)
{
    const char *cFunc = env->GetStringUTFChars(func, JNI_FALSE);
    const char *cParams = params ? env->GetStringUTFChars(params, JNI_FALSE) : nullptr;
    const char *res = ngenxx_L_call((void *)lState, cFunc, cParams);
    return env->NewStringUTF(res);
}