#ifndef NGENXX_JNI_UTIL_HXX_
#define NGENXX_JNI_UTIL_HXX_

#include <jni.h>
#include <sstream>
#include <string>

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
    return boxJNum<jint>(env, j, JNumCls(Integer), JNumSig(Integer, I));
}

inline jobject boxJLong(JNIEnv *env, const jlong j)
{
    return boxJNum<jlong>(env, j, JNumCls(Long), JNumSig(Long, J));
}

inline jobject boxJFloat(JNIEnv *env, const jfloat j)
{
    return boxJNum<jfloat>(env, j, JNumCls(Float), JNumSig(Float, F));
}

inline jobject boxJDouble(JNIEnv *env, const jdouble j)
{
    return boxJNum<jdouble>(env, j, JNumCls(Double), JNumSig(Double, D));
}

inline jstring boxJString(JNIEnv *env, const char *str)
{
    if (str == nullptr) 
    {
        str = "";
    }
    auto sLen = static_cast<jsize>(strlen(str));
    auto p = reinterpret_cast<const unsigned char*>(str);
    std::ostringstream oss;
    size_t i = 0;
    
    while (i < sLen)
    {
        if (p[i] < 0x80) 
        { // 1-byte character
            oss << p[i];
            i++;
        } 
        else if ((p[i] >> 5) == 0b110 && 
                 i + 1 < sLen &&
                 (p[i + 1] >> 6) == 0b10) 
        { // valid 2-byte character
            oss << p[i] << p[i + 1];
            i += 2;
        } 
        else if ((p[i] >> 4) == 0b1110 && 
                 i + 2 < sLen &&
                 (p[i + 1] >> 6) == 0b10 && 
                 (p[i + 2] >> 6) == 0b10) 
        { // valid 3-byte character
            oss << p[i] << p[i + 1] << p[i + 2];
            i += 3;
        } 
        else if ((p[i] >> 3) == 0b11110 && 
                 i + 3 < sLen &&
                 (p[i + 1] >> 6) == 0b10 && 
                 (p[i + 2] >> 6) == 0b10 && 
                 (p[i + 3] >> 6) == 0b10) 
        { // valid 4-byte character
            oss << p[i] << p[i + 1] << p[i + 2] << p[i + 3];
            i += 4;
        } 
        else [[unlikely]]
        { // invalid UTF-8 sequence
            oss << '?';
            i++;
        }
    }

    auto fixedStr = oss.str();
    auto fixedLen = static_cast<jsize>(fixedStr.length());
    auto strBytes = env->NewByteArray(fixedLen);
    env->SetByteArrayRegion(strBytes, 0, fixedLen,
                           reinterpret_cast<const jbyte*>(fixedStr.c_str()));
    auto strClass = env->FindClass(JLS);
    auto strConstructor = env->GetMethodID(strClass, "<init>", "([B" LJLS_ ")V");
    auto charsetName = env->NewStringUTF("UTF-8");
    auto jStr = reinterpret_cast<jstring>(env->NewObject(strClass, strConstructor,
                                                         strBytes, charsetName));
    env->DeleteLocalRef(strBytes);
    env->DeleteLocalRef(charsetName);
    env->DeleteLocalRef(strClass);
    return jStr;
}

inline jmethodID getLambdaMethod(JNIEnv *env, const char* cls, const char *sig)
{
    auto function1Class = env->FindClass(cls);
    return env->GetMethodID(function1Class, "invoke", sig);
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