#include "JNIUtil.hxx"

#include <limits>
#include <mutex>
#include <vector>

#include <DynXX/CXX/Memory.hxx>

namespace {
#if defined(__cpp_lib_generic_unordered_lookup)
    using JClassCache = std::unordered_map<std::string, jclass, TransparentStringHash, TransparentEqual>;
#else
    using JClassCache = std::unordered_map<std::string, jclass>;
#endif
    JClassCache sJClassCache;
    std::mutex sJClassCacheMutex;
}

JNIEnv *getEnv(JavaVM *vm, int ver) {
    if (vm == nullptr) {
        return nullptr;
    }
    JNIEnv *env = nullptr;
    if (auto ret = vm->GetEnv(reinterpret_cast<void **>(&env), ver); ret != JNI_OK) {
        return nullptr;
    }
    return env;
}

JNIEnv *attachEnv(JavaVM *vm) {
    JNIEnv *env = nullptr;
    if (vm != nullptr) {
        vm->AttachCurrentThread(&env, nullptr);
    }
    if (env != nullptr && env->ExceptionCheck() == JNI_TRUE) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    return env;
}

jclass findClassInCache(JNIEnv *env, const char *cls) {
    const auto lock = std::scoped_lock(sJClassCacheMutex);
    if (const auto it = sJClassCache.find(cls); it != sJClassCache.end()) {
        return it->second;
    }
    const auto jcls = env->FindClass(cls);
    sJClassCache[cls] = static_cast<jclass>(env->NewGlobalRef(jcls));
    return sJClassCache.at(cls);
}

void releaseCachedClass(JNIEnv *env) {
    const auto lock = std::scoped_lock(sJClassCacheMutex);
    for (auto &[k, v] : sJClassCache) {
        env->DeleteGlobalRef(v);
    }
    sJClassCache.clear();
}

const char* readJString(JNIEnv *env, jobject jStr) {
    if (env == nullptr || jStr == nullptr) {
        return nullptr;
    }
    return env->GetStringUTFChars(static_cast<jstring>(jStr), nullptr);
}

void releaseJString(JNIEnv *env, jobject jStr, const char* cStr) {
    if (env == nullptr || jStr == nullptr || cStr == nullptr) {
        return;
    }
    env->ReleaseStringUTFChars(static_cast<jstring>(jStr), cStr);
}

std::tuple<byte*, size_t> readJByteArray(JNIEnv *env, jbyteArray jbArr) {
    if (env == nullptr || jbArr == nullptr) {
        return {};
    }
    const auto len = env->GetArrayLength(jbArr);
    const auto jo = env->GetByteArrayElements(jbArr, nullptr);
    auto cBytes = reinterpret_cast<byte *>(jo);
    return std::make_tuple(cBytes, len);
}

void releaseJByteArray(JNIEnv *env, jbyteArray jbArr, byte *cBytes) {
    if (env == nullptr || jbArr == nullptr || cBytes == nullptr) {
        return;
    }
    env->ReleaseByteArrayElements(jbArr, reinterpret_cast<jbyte*>(cBytes), JNI_ABORT);
}

std::tuple<const jobject*, const char**, size_t> readJStringArray(JNIEnv *env, jobjectArray joArr) {
    if (env == nullptr || joArr == nullptr) {
        return {};
    }
    const auto size = env->GetArrayLength(joArr);
    const auto jStrArr = new jobject[size];
    const auto cStrArr = new const char*[size];
    for (jsize i = 0; i < size; i++) {
        jStrArr[i] = env->GetObjectArrayElement(joArr, i);
        cStrArr[i] = readJString(env, jStrArr[i]);
    }
    return std::make_tuple(jStrArr, cStrArr, size);
}

void releaseJStringArray(JNIEnv *env, jobjectArray joArr, const jobject *jStrArr, const char **cStrArr) {
    if (env == nullptr || joArr == nullptr || jStrArr == nullptr || cStrArr == nullptr) {
        return;
    }
    const auto size = env->GetArrayLength(joArr);
    for (jsize i = 0; i < size; i++) {
        releaseJString(env, jStrArr[i], cStrArr[i]);
        env->DeleteLocalRef(jStrArr[i]);
    }
    delete[] jStrArr;
    delete[] cStrArr;
}

jobject boxJNum(JNIEnv *env, const char *cls, const char *sig, ...) {
    const auto jClass = findClassInCache(env, cls);
    const auto jValueOf = env->GetStaticMethodID(jClass, "valueOf", sig);

    va_list args;
    va_start(args, sig);
    const auto ret = env->CallStaticObjectMethod(jClass, jValueOf, args);
    va_end(args);

    return ret;
}

jobject boxJBoolean(JNIEnv *env, jboolean j) {
    return boxJNum(env, JNumCls(Boolean), JNumSig(Boolean, Z), j);
}

jobject boxJInt(JNIEnv *env, jint j) {
    return boxJNum(env, JNumCls(Integer), JNumSig(Integer, I), j);
}

jobject boxJLong(JNIEnv *env, jlong j) {
    return boxJNum(env, JNumCls(Long), JNumSig(Long, J), j);
}

jobject boxJFloat(JNIEnv *env, jfloat j) {
    return boxJNum(env, JNumCls(Float), JNumSig(Float, F), j);
}

jobject boxJDouble(JNIEnv *env, jdouble j) {
    return boxJNum(env, JNumCls(Double), JNumSig(Double, D), j);
}

jstring boxJString(JNIEnv *env, const char *str) {
    if (str == nullptr) {
        return boxJString(env, std::string_view{});
    }
    const auto ret = boxJString(env, std::string_view{str});
    freeX(str);
    return ret;
}

jstring boxJString(JNIEnv *env, std::string_view str) {
    const auto rawLen = str.size();
    if (rawLen > static_cast<size_t>(std::numeric_limits<jsize>::max())) {
        return env->NewStringUTF("");
    }
    const auto sLen = static_cast<jsize>(rawLen);
    auto p = reinterpret_cast<const unsigned char *>(str.data());
    
    std::string fixedStr;
    fixedStr.reserve(sLen);
    
    jsize i = 0;
    while (i < sLen) {
        if (p[i] < 0x80) {
            // 1-byte character
            fixedStr.push_back(static_cast<char>(p[i]));
            i++;
        } else if ((p[i] >> 5) == 0b110 &&
                   i + 1 < sLen &&
                   (p[i + 1] >> 6) == 0b10) {
            // valid 2-byte character
            fixedStr.push_back(static_cast<char>(p[i]));
            fixedStr.push_back(static_cast<char>(p[i + 1]));
            i += 2;
        } else if ((p[i] >> 4) == 0b1110 &&
                   i + 2 < sLen &&
                   (p[i + 1] >> 6) == 0b10 &&
                   (p[i + 2] >> 6) == 0b10) {
            // valid 3-byte character
            fixedStr.push_back(static_cast<char>(p[i]));
            fixedStr.push_back(static_cast<char>(p[i + 1]));
            fixedStr.push_back(static_cast<char>(p[i + 2]));
            i += 3;
        } else if ((p[i] >> 3) == 0b11110 &&
                   i + 3 < sLen &&
                   (p[i + 1] >> 6) == 0b10 &&
                   (p[i + 2] >> 6) == 0b10 &&
                   (p[i + 3] >> 6) == 0b10) {
            // valid 4-byte character
            fixedStr.push_back(static_cast<char>(p[i]));
            fixedStr.push_back(static_cast<char>(p[i + 1]));
            fixedStr.push_back(static_cast<char>(p[i + 2]));
            fixedStr.push_back(static_cast<char>(p[i + 3]));
            i += 4;
        } else [[unlikely]] {
            // invalid UTF-8 sequence
            fixedStr.push_back('?');
            i++;
        }
    }
    
    auto fixedLen = static_cast<jsize>(fixedStr.length());
    auto strBytes = env->NewByteArray(fixedLen);
    env->SetByteArrayRegion(strBytes, 0, fixedLen, reinterpret_cast<const jbyte *>(fixedStr.c_str()));
    auto strClass = findClassInCache(env, JLS);
    auto strConstructor = env->GetMethodID(strClass, "<init>", "([B" LJLS_ ")V");
    auto charsetName = env->NewStringUTF("UTF-8");
    auto jStr = static_cast<jstring>(env->NewObject(strClass, strConstructor, strBytes, charsetName));
    env->DeleteLocalRef(strBytes);
    env->DeleteLocalRef(charsetName);

    return jStr;
}

jmethodID getLambdaMethod(JNIEnv *env, const char *cls, const char *sig) {
    const auto function1Class = findClassInCache(env, cls);
    const auto ret = env->GetMethodID(function1Class, "invoke", sig);
    return ret;
}

jbyteArray moveToJByteArray(JNIEnv *env, const byte *bytes, size_t len) {
    if (bytes == nullptr || len == 0) {
        return env->NewByteArray(0);
    }
    jbyteArray jba = env->NewByteArray(static_cast<jsize>(len));
    std::vector<jbyte> jBytes(len);
    for (size_t i = 0; i < len; i++) {
        jBytes[i] = static_cast<jbyte>(bytes[i]);
    }
    env->SetByteArrayRegion(jba, 0, static_cast<jsize>(len),
                                jBytes.data());
    freeX(bytes);
    return jba;
}

jobjectArray moveToJStringArray(JNIEnv *env, const char **strs, size_t len) {
    if (strs == nullptr || len  == 0) {
        return env->NewObjectArray(0, findClassInCache(env, JLS), nullptr);
    }
    auto jStrs = env->NewObjectArray(static_cast<jsize>(len), findClassInCache(env, JLS), nullptr);
    for (size_t i = 0; i < len; i++) {
        env->SetObjectArrayElement(jStrs, static_cast<jsize>(i), boxJString(env, strs[i]));
        freeX(strs[i]);
    }
    freeX(strs);
    return jStrs;
}
