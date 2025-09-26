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

JNIEnv *getEnv(JavaVM *vm, int ver);

JNIEnv *attachEnv(JavaVM *vm);

jclass findClassInCache(JNIEnv *env, const char *cls);

void releaseCachedClass(JNIEnv *env);

// read c types from jni object

const char* readJString(JNIEnv *env, jobject jStr);

void releaseJString(JNIEnv *env, jobject jStr, const char* cStr);

struct JArg {
    JNIEnv *env;
    JArg() = delete;
    explicit JArg(JNIEnv *env) : env(env) {}
    virtual ~JArg() = default;
    JArg(const JArg&) = delete;
    JArg& operator=(const JArg&) = delete;
    JArg(JArg&&) = delete;
    JArg& operator=(JArg&&) = delete;
};

struct JStringArg final : public JArg {
public:
    const char *data;

    explicit JStringArg(JNIEnv *env, jobject jStr) : JArg(env), jStr(jStr) {
        data = readJString(env, jStr);
    }
    ~JStringArg() override {
        releaseJString(this->env, this->jStr, this->data);
    }

private:
    jobject jStr;
};

std::tuple<byte*, size_t> readJByteArray(JNIEnv *env, jbyteArray jbArr);

void releaseJByteArray(JNIEnv *env, jbyteArray jbArr, byte *cBytes);

struct JByteArrayArg final : public JArg {
public:
    byte *data{nullptr};
    size_t size{0};

    explicit JByteArrayArg(JNIEnv *env, jbyteArray jbArr) : JArg(env), jbArr(jbArr) {
        std::tie(data, size) = readJByteArray(env, jbArr);
    }
    ~JByteArrayArg() override {
        releaseJByteArray(this->env, this->jbArr, this->data);
    }

private:
    jbyteArray jbArr;
};

std::tuple<const jobject*, const char**, size_t> readJStringArray(JNIEnv *env, jobjectArray joArr);

void releaseJStringArray(JNIEnv *env, jobjectArray joArr, const jobject *jStrArr, const char **cStrArr);

struct JStringArrayArg final : public JArg {
public:
    const char **data{nullptr};
    size_t size{0};
    
    explicit JStringArrayArg(JNIEnv *env, jobjectArray joArr) : JArg(env), joArr(joArr) {
        std::tie(jStrs, data, size) = readJStringArray(env, joArr);
    }
    ~JStringArrayArg() override {
        releaseJStringArray(this->env, this->joArr, this->jStrs, this->data);
    }

private:
    jobjectArray joArr;
    const jobject *jStrs{nullptr};
};

// wrap jni object with C types

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