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

inline jobject boxJInt(JNIEnv *env, const jint ji)
{
    jclass integerClass = env->FindClass("java/lang/Integer");
    jmethodID intValueOf = env->GetStaticMethodID(integerClass, "valueOf", "(I)Ljava/lang/Integer;");
    return env->CallStaticObjectMethod(integerClass, intValueOf, ji);
}

inline jobject boxJLong(JNIEnv *env, const jlong jl)
{
    jclass longClass = env->FindClass("java/lang/Long");
    jmethodID longValueOf = env->GetStaticMethodID(longClass, "valueOf", "(J)Ljava/lang/Long;");
    return env->CallStaticObjectMethod(longClass, longValueOf, jl);
}

inline jmethodID getLambdaMethod1(JNIEnv *env)
{
    jclass function1Class = env->FindClass("kotlin/jvm/functions/Function1");
    return env->GetMethodID(function1Class, "invoke", "(Ljava/lang/Object;)Ljava/lang/Object;");
}

inline jmethodID getLambdaMethod2(JNIEnv *env)
{
    jclass function1Class = env->FindClass("kotlin/jvm/functions/Function2");
    return env->GetMethodID(function1Class, "invoke", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
}

inline jbyteArray moveToJByteArray(JNIEnv *env, const byte *bytes, size_t outLen, bool needFree)
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