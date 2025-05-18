#ifndef NGENXX_JNI_UTIL_HXX_
#define NGENXX_JNI_UTIL_HXX_

#include <jni.h>

#include "../../../../../../build.Android/output/include/NGenXXTypes.hxx"

inline JNIEnv *currentEnv(JavaVM *vm) {
    JNIEnv *env = nullptr;
    if (vm != nullptr) {
        vm->AttachCurrentThread(&env, nullptr);
    }
    return env;
}

jbyteArray moveToJByteArray(JNIEnv *env, const byte *bytes, size_t outLen, bool needFree)
{
    jbyteArray jba;
    if (bytes && outLen > 0)
    {
        jba = env->NewByteArray(static_cast<jsize>(outLen));
        env->SetByteArrayRegion(jba, 0, static_cast<jsize>(outLen), const_cast<jbyte *>(reinterpret_cast<const jbyte *>(bytes)));
        if (needFree) {
            freeX(bytes);
        }
    }
    else
    {
        jba = env->NewByteArray(0);
    }
    return jba;
}

#endif // NGENXX_JNI_UTIL_HXX_