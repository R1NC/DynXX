#ifndef NGENXX_JNI_UTIL_HXX_
#define NGENXX_JNI_UTIL_HXX_

#include <jni.h>

#include <cstdlib>
#include <string>
#include <type_traits>

#include "../../../../../../build.Android/output/include/NGenXXTypes.h"

static inline JNIEnv *currentEnv(JavaVM *vm) {
    JNIEnv *env = NULL;
    if (vm != nullptr) {
        vm->AttachCurrentThread(&env, nullptr);
    }
    return env;
}

jbyteArray moveToJByteArray(JNIEnv *env, const byte *bytes, const size_t outLen, const bool needFree)
{
    jbyteArray jba;
    if (bytes && outLen > 0)
    {
        jba = env->NewByteArray(static_cast<jsize>(outLen));
        env->SetByteArrayRegion(jba, 0, static_cast<jsize>(outLen), reinterpret_cast<jbyte *>(std::decay_t<byte *>(bytes)));
        if (needFree) {
            std::free(static_cast<void *>(std::decay_t<byte *>(bytes)));
        }
    }
    else
    {
        jba = env->NewByteArray(0);
    }
    return jba;
}

#endif // NGENXX_JNI_UTIL_HXX_