#ifndef NGENXX_JNI_UTIL_HXX_
#define NGENXX_JNI_UTIL_HXX_

#include <jni.h>

#include "../../../../../../build.Android/output/include/NGenXXTypes.hxx"

inline JNIEnv *currentEnv(JavaVM *vm)
{
    JNIEnv *env = nullptr;
    if (vm != nullptr)
    {
        vm->AttachCurrentThread(&env, nullptr);
    }
    return env;
}

template <NumberT T>
inline jobject boxJNum(JNIEnv *env, const T j, const char *cls, const char *sig) {
    auto booleanClass = env->FindClass(cls);
    auto boolValueOf = env->GetStaticMethodID(booleanClass, "valueOf", sig);
    return env->CallStaticObjectMethod(booleanClass, boolValueOf, j);
}

inline jobject boxJBoolean(JNIEnv *env, const jboolean j) {
    return boxJNum<jboolean>(env, j, "java/lang/Boolean", "(Z)Ljava/lang/Boolean;");
}

inline jobject boxJInt(JNIEnv *env, const jint j)
{
    return boxJNum<jboolean>(env, j, "java/lang/Integer", "(I)Ljava/lang/Integer;");
}

inline jobject boxJLong(JNIEnv *env, const jlong j)
{
    return boxJNum<jboolean>(env, j, "java/lang/Long", "(J)Ljava/lang/Long;");
}

inline jobject boxJFloat(JNIEnv *env, const jfloat j)
{
    return boxJNum<jboolean>(env, j, "java/lang/Float", "(F)Ljava/lang/Float;");
}

inline jobject boxJDouble(JNIEnv *env, const jdouble j)
{
    return boxJNum<jboolean>(env, j, "java/lang/Double", "(D)Ljava/lang/Double;");
}

inline jmethodID getLambdaMethod(JNIEnv *env, const char* cls, const char *sig)
{
    auto function1Class = env->FindClass(cls);
    return env->GetMethodID(function1Class, "invoke", sig);
}

inline jmethodID getLambdaMethod1(JNIEnv *env)
{
    return getLambdaMethod(env, "kotlin/jvm/functions/Function1", "(Ljava/lang/Object;)Ljava/lang/Object;");
}

inline jmethodID getLambdaMethod2(JNIEnv *env)
{
    return getLambdaMethod(env, "kotlin/jvm/functions/Function2", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
}

inline jbyteArray moveToJByteArray(JNIEnv *env, const byte *bytes, size_t outLen, bool needFree)
{
    jbyteArray jba;
    if (bytes && outLen > 0)
    {
        jba = env->NewByteArray(static_cast<jsize>(outLen));
        env->SetByteArrayRegion(jba, 0, static_cast<jsize>(outLen), const_cast<jbyte *>(reinterpret_cast<const jbyte *>(bytes)));
        if (needFree)
        {
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