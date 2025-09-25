#pragma once

#include <jni.h>
#include <string>

#include "../../../../../../build.Android/output/include/DynXX/CXX/Types.hxx"

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

JNIEnv *currentEnv(JavaVM *vm);

jclass findClassInCache(JNIEnv *env, const char *cls);

void releaseCachedClass(JNIEnv *env);

std::tuple<byte*, size_t> readJBytes(JNIEnv *env, jbyteArray jbArr);

void releaseJBytes(JNIEnv *env, jbyteArray jbArr, byte *cBytes);

const char* readJString(JNIEnv *env, jobject jStr);

void releaseJString(JNIEnv *env, jobject jStr, const char* cStr);

template<typename JT>
JT readJObjectArrayItem(JNIEnv *env, jobjectArray joArr, jsize idx) {
    return reinterpret_cast<JT>(env->GetObjectArrayElement(joArr, idx));
}

jobject boxJNum(JNIEnv *env, const char *cls, const char *sig, ...);

jobject boxJBoolean(JNIEnv *env, jboolean j);

jobject boxJInt(JNIEnv *env, jint j);

jobject boxJLong(JNIEnv *env, jlong j);

jobject boxJFloat(JNIEnv *env, jfloat j);

jobject boxJDouble(JNIEnv *env, jdouble j);

jstring boxJString(JNIEnv *env, const char *str);

jmethodID getLambdaMethod(JNIEnv *env, const char *cls, const char *sig);

jbyteArray moveToJByteArray(JNIEnv *env, const byte *bytes, size_t len);

jobjectArray moveToJStringArray(JNIEnv *env, const char **strs, size_t len);