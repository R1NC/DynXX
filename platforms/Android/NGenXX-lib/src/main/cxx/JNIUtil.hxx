#ifndef NGENXX_JNI_UTIL_HXX_
#define NGENXX_JNI_UTIL_HXX_

#include <jni.h>
#include <cstdlib>
#include <string>
#include <functional>
#include <type_traits>

#include "../../../../../../build.Android/output/include/NGenXXTypes.h"

static inline void *runInCurrentEnv(JavaVM *vm, const std::function<void *(JNIEnv *env)> &task)
{
    if (vm == nullptr) {
        return nullptr;
    }
    JNIEnv *env;
    vm->AttachCurrentThread(&env, nullptr);
    if (env == nullptr) {
        return nullptr;
    }
    void *t = task(env);
    // vm->DetachCurrentThread();
    return t;
}

jbyteArray moveToJByteArray(JNIEnv *env, const byte *bytes, const size_t outLen, const bool needFree)
{
    jbyteArray jba;
    if (bytes && outLen > 0)
    {
        jba = env->NewByteArray(static_cast<jsize>(outLen));
        env->SetByteArrayRegion(jba, 0, static_cast<jsize>(outLen), reinterpret_cast<jbyte *>(const_cast<byte *>(bytes)));
        if (needFree) {
            free(static_cast<void *>(const_cast<byte *>(bytes)));
        }
    }
    else
    {
        jba = env->NewByteArray(0);
    }
    return jba;
}

#endif // NGENXX_JNI_UTIL_HXX_