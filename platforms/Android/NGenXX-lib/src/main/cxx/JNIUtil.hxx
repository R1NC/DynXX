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

#define JNumCls(T) "java/lang/"#T
#define JNumSig(T, S) "(" #S ")L" JNumCls(T) ";"

template <NumberT T>
inline jobject boxJNum(JNIEnv *env, const T j, const char *cls, const char *sig) {
    auto jClass = env->FindClass(cls);
    auto jValueOf = env->GetStaticMethodID(jClass, "valueOf", sig);
    return env->CallStaticObjectMethod(jClass, jValueOf, j);
}

inline jobject boxJBoolean(JNIEnv *env, const jboolean j) {
    return boxJNum<jboolean>(env, j, JNumCls(Boolean), JNumSig(Boolean, Z));
}

inline jobject boxJInt(JNIEnv *env, const jint j)
{
    return boxJNum<jboolean>(env, j, JNumCls(Integer), JNumSig(Integer, I));
}

inline jobject boxJLong(JNIEnv *env, const jlong j)
{
    return boxJNum<jboolean>(env, j, JNumCls(Long), JNumSig(Long, J));
}

inline jobject boxJFloat(JNIEnv *env, const jfloat j)
{
    return boxJNum<jboolean>(env, j, JNumCls(Float), JNumSig(Float, F));
}

inline jobject boxJDouble(JNIEnv *env, const jdouble j)
{
    return boxJNum<jboolean>(env, j, JNumCls(Double), JNumSig(Double, D));
}

#define K_FUN "kotlin/jvm/functions/Function"
#define J_OBJ "Ljava/lang/Object;"

inline jmethodID getLambdaMethod(JNIEnv *env, const char* cls, const char *sig)
{
    auto function1Class = env->FindClass(cls);
    return env->GetMethodID(function1Class, "invoke", sig);
}

inline jmethodID getLambdaMethod1(JNIEnv *env)
{
    return getLambdaMethod(env, K_FUN "1", "(" J_OBJ ")" J_OBJ);
}

inline jmethodID getLambdaMethod2(JNIEnv *env)
{
    return getLambdaMethod(env, K_FUN "2", "(" J_OBJ J_OBJ ")" J_OBJ);
}

inline jbyteArray moveToJByteArray(JNIEnv *env, const byte *bytes, size_t outLen, const bool needFree)
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