#ifndef NGENXX_JNI_UTIL_HXX_
#define NGENXX_JNI_UTIL_HXX_

#include <jni.h>

#include "../../../../../../build.Android/output/include/NGenXXTypes.hxx"

#define L "L"
#define _ ";"
#define JL "java/lang/"
#define JLO JL "Object"
#define LJLO L JLO
#define LJLO_ LJLO _
#define JLS JL "String"
#define LJLS L JLS
#define LJLS_ LJLS _
#define JNumCls(T) JL #T
#define JNumSig(T, S) "(" #S ")" L JNumCls(T) _
#define KF "kotlin/jvm/functions/Function"
#define KF1 KF "1"
#define KF2 KF "2"
#define LKF1 L KF1
#define LKF2 L KF2
#define LKF1_ LKF1 _
#define LKF2_ LKF2 _

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
jobject boxJNum(JNIEnv *env, const T j, const char *cls, const char *sig)
{
    auto jClass = env->FindClass(cls);
    auto jValueOf = env->GetStaticMethodID(jClass, "valueOf", sig);
    return env->CallStaticObjectMethod(jClass, jValueOf, j);
}

inline jobject boxJBoolean(JNIEnv *env, const jboolean j)
{
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

inline jmethodID getLambdaMethod(JNIEnv *env, const char* cls, const char *sig)
{
    auto function1Class = env->FindClass(cls);
    return env->GetMethodID(function1Class, "invoke", sig);
}

inline jmethodID getLambdaMethod1(JNIEnv *env)
{
    return getLambdaMethod(env, KF1, "(" LJLO_ ")" LJLO_);
}

inline jmethodID getLambdaMethod2(JNIEnv *env)
{
    return getLambdaMethod(env, KF2, "(" LJLO_ LJLO_ ")" LJLO_);
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