#include <cstring>

#include "JNIUtil.hxx"
#include "../../../../../../build.Android/output/include/DynXX/C/DynXX.h"
#include "../../../../../../build.Android/output/include/DynXX/CXX/Types.hxx"

namespace {

    JavaVM *sVM;
    jclass sJClass;
    jobject sLogCallback;
    jmethodID sLogCallbackMethodId;
    jobject sJsMsgCallback;
    jmethodID sJsMsgCallbackMethodId;

    auto JClassName = "xyz/rinc/dynxx/DynXX$Companion";

#define DECLARE_JNI_FUNC(func, signature) {#func, signature, reinterpret_cast<void *>(func)}

// Engine Callback

    void log_callback(int level, const char *content) {
        if (sVM == nullptr || sLogCallback == nullptr || sLogCallbackMethodId == nullptr ||
            content == nullptr) {
            return;
        }
        auto env = currentEnv(sVM);
        if (env == nullptr) {
            return;
        }
        auto jContent = boxJString(env, content);
        //env->CallVoidMethod(sLogCallback, sLogCallbackMethodId, level, jContent);
        auto jLevel = boxJInt(env, static_cast<jint>(level));
        auto jRet = env->CallObjectMethod(sLogCallback, sLogCallbackMethodId, jLevel, jContent);
        if (jRet) {
            env->DeleteLocalRef(jRet);
        }
        env->DeleteLocalRef(jLevel);
        env->DeleteLocalRef(jContent);
    }

    const char *js_msg_callback(const char *msg) {
        if (sVM == nullptr || sJsMsgCallback == nullptr || sJsMsgCallbackMethodId == nullptr ||
            msg == nullptr) {
            return nullptr;
        }
        auto env = currentEnv(sVM);
        if (env == nullptr) {
            return nullptr;
        }
        auto jMsg = boxJString(env, msg);
        auto jRes = env->CallObjectMethod(sJsMsgCallback, sJsMsgCallbackMethodId, jMsg);
        auto cRes = env->GetStringUTFChars(reinterpret_cast<jstring>(jRes), nullptr);
        auto newRes = dupStr(cRes);
        if (cRes) {
           env->ReleaseStringUTFChars(reinterpret_cast<jstring>(jRes), cRes);
        }
        env->DeleteLocalRef(jMsg);
        env->DeleteLocalRef(jRes);
        return newRes;
    }

// Engine base API

    jstring getVersion(JNIEnv *env, jobject thiz) {
        auto cV = dynxx_get_version();
        auto jStr = boxJString(env, cV);
        freeX(cV);
        return jStr;
    }

    jboolean init(JNIEnv *env, jobject thiz,
                  jstring root) {
        auto cRoot = env->GetStringUTFChars(root, nullptr);
        auto res = dynxx_init(cRoot);
        env->ReleaseStringUTFChars(root, cRoot);
        return res;
    }

    void release(JNIEnv *env, jobject thiz) {
        dynxx_release();
    }

// Log

    void logSetLevel(JNIEnv *env, jobject thiz,
                     jint level) {
        dynxx_log_set_level(level);
    }

    void logSetCallback(JNIEnv *env, jobject thiz,
                        jobject callback) {
        if (callback) {
            dynxx_log_set_callback(log_callback);
            sLogCallback = env->NewGlobalRef(callback);
            /*auto jCallbackClass = env->GetObjectClass(callback);
            if (jCallbackClass)
            {
                sLogCallbackMethodId = env->GetMethodID(jCallbackClass, "invoke", "(I" LJLS_ ")V");
            }*/
            sLogCallbackMethodId = getLambdaMethod(env, KF2, "(" LJLO_ LJLO_ ")" LJLO_);
        }
    }

    void logPrint(JNIEnv *env, jobject thiz,
                  jint level, jstring content) {
        auto cContent = env->GetStringUTFChars(content, nullptr);
        dynxx_log_print(level, cContent);
        env->ReleaseStringUTFChars(content, cContent);
    }

// Net

    jstring netHttpRequest(JNIEnv *env, jobject thiz,
                           jstring url, jstring params,
                           jint method,
                           jobjectArray headerV,
                           jobjectArray formFieldNameV,
                           jobjectArray formFieldMimeV,
                           jobjectArray formFieldDataV,
                           jstring filePath, jlong fileLength,
                           jlong timeout) {
        auto cUrl = env->GetStringUTFChars(url, nullptr);
        auto cParams = env->GetStringUTFChars(params, nullptr);

        auto headerCount = env->GetArrayLength(headerV);

        auto cHeaderV = mallocX<char *>(headerCount);
        auto jStrHeaderV = mallocX<jstring>(headerCount);

        for (int i = 0; i < headerCount; i++) {
            jStrHeaderV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(headerV, i));
            cHeaderV[i] = const_cast<char *>(env->GetStringUTFChars(jStrHeaderV[i], nullptr));
        }

        auto formFieldCount = env->GetArrayLength(formFieldNameV);
        auto cFormFieldNameV = mallocX<char *>(formFieldCount);
        auto jStrFormFieldNameV = mallocX<jstring>(formFieldCount);
        auto cFormFieldMimeV = mallocX<char *>(formFieldCount);
        auto jStrFormFieldMimeV = mallocX<jstring>(formFieldCount);
        auto cFormFieldDataV = mallocX<char *>(formFieldCount);
        auto jStrFormFieldDataV = mallocX<jstring>(formFieldCount);

        for (int i = 0; i < formFieldCount; i++) {
            jStrFormFieldNameV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(
                    formFieldNameV, i));
            cFormFieldNameV[i] = const_cast<char *>(env->GetStringUTFChars(jStrFormFieldNameV[i],
                                                                           nullptr));
            jStrFormFieldMimeV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(
                    formFieldMimeV, i));
            cFormFieldMimeV[i] = const_cast<char *>(env->GetStringUTFChars(jStrFormFieldMimeV[i],
                                                                           nullptr));
            jStrFormFieldDataV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(
                    formFieldDataV, i));
            cFormFieldDataV[i] = const_cast<char *>(env->GetStringUTFChars(jStrFormFieldDataV[i],
                                                                           nullptr));
        }

        auto cFilePath = filePath ? env->GetStringUTFChars(filePath, nullptr) : "";
        auto cFILE = cFilePath ? std::fopen(cFilePath, "r") : nullptr;

        auto cRsp = dynxx_net_http_request(cUrl, cParams, static_cast<const int>(method),
                                            const_cast<const char **>(cHeaderV), headerCount,
                                            const_cast<const char **>(cFormFieldNameV),
                                            const_cast<const char **>(cFormFieldMimeV),
                                            const_cast<const char **>(cFormFieldDataV),
                                            formFieldCount,
                                            static_cast<void *>(cFILE), fileLength,
                                            static_cast<const size_t>(timeout));
        auto jStr = boxJString(env, cRsp);
        freeX(cRsp);

        for (int i = 0; i < headerCount; i++) {
            env->ReleaseStringUTFChars(jStrHeaderV[i], cHeaderV[i]);
        }
        for (int i = 0; i < formFieldCount; i++) {
            env->ReleaseStringUTFChars(jStrFormFieldNameV[i], cFormFieldNameV[i]);
            env->ReleaseStringUTFChars(jStrFormFieldMimeV[i], cFormFieldMimeV[i]);
            env->ReleaseStringUTFChars(jStrFormFieldDataV[i], cFormFieldDataV[i]);
        }

        freeX(cHeaderV);
        freeX(cFormFieldNameV);
        freeX(cFormFieldMimeV);
        freeX(cFormFieldDataV);
        freeX(jStrHeaderV);
        freeX(jStrFormFieldNameV);
        freeX(jStrFormFieldMimeV);
        freeX(jStrFormFieldDataV);

        if (filePath) {
            env->ReleaseStringUTFChars(filePath, cFilePath);
        }
        if (cFILE) {
            std::fclose(cFILE);
        }
        if (cParams) {
            env->ReleaseStringUTFChars(params, cParams);
        }
        env->ReleaseStringUTFChars(url, cUrl);

        return jStr;
    }

// Lua

    jboolean lLoadF(JNIEnv *env, jobject thiz,
                    jstring file) {
        auto cFile = env->GetStringUTFChars(file, nullptr);
        auto res = dynxx_lua_loadF(cFile);
        env->ReleaseStringUTFChars(file, cFile);
        return res;
    }

    jboolean lLoadS(JNIEnv *env, jobject thiz,
                    jstring script) {
        auto cScript = env->GetStringUTFChars(script, nullptr);
        auto res = dynxx_lua_loadS(cScript);
        env->ReleaseStringUTFChars(script, cScript);
        return res;
    }

    jstring lCall(JNIEnv *env, jobject thiz,
                  jstring func, jstring params) {
        auto cFunc = env->GetStringUTFChars(func, nullptr);
        auto cParams = params ? env->GetStringUTFChars(params, nullptr) : nullptr;
        auto cRes = dynxx_lua_call(cFunc, cParams);
        auto jStr = boxJString(env, cRes);
        freeX(cRes);
        if (cParams) {
            env->ReleaseStringUTFChars(params, cParams);
        }
        env->ReleaseStringUTFChars(func, cFunc);
        return jStr;
    }

// JS

    jboolean jLoadF(JNIEnv *env, jobject thiz,
                    jstring file, jboolean is_module) {
        auto cFile = env->GetStringUTFChars(file, nullptr);
        auto res = dynxx_js_loadF(cFile, is_module);
        env->ReleaseStringUTFChars(file, cFile);
        return res;
    }

    jboolean jLoadS(JNIEnv *env, jobject thiz,
                    jstring script, jstring name, jboolean is_module) {
        auto cScript = env->GetStringUTFChars(script, nullptr);
        auto cName = env->GetStringUTFChars(name, nullptr);
        auto res = dynxx_js_loadS(cScript, cName, is_module);
        env->ReleaseStringUTFChars(script, cScript);
        env->ReleaseStringUTFChars(name, cName);
        return res;
    }

    jboolean jLoadB(JNIEnv *env, jobject thiz,
                    jbyteArray bytes, jboolean is_module) {
        auto cIn = env->GetByteArrayElements(bytes, nullptr);
        auto inLen = env->GetArrayLength(bytes);

        auto res = dynxx_js_loadB(reinterpret_cast<const byte *>(cIn), inLen, is_module);

        env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
        return res;
    }

    jstring jCall(JNIEnv *env, jobject thiz,
                  jstring func, jstring params, jboolean await) {
        auto cFunc = env->GetStringUTFChars(func, nullptr);
        auto cParams = params ? env->GetStringUTFChars(params, nullptr) : nullptr;
        auto cRes = dynxx_js_call(cFunc, cParams, await);
        auto jStr = boxJString(env, cRes);
        freeX(cRes);
        if (cParams) {
            env->ReleaseStringUTFChars(params, cParams);
        }
        env->ReleaseStringUTFChars(func, cFunc);
        return jStr;
    }

    void jSetMsgCallback(JNIEnv *env, jobject thiz,
                         jobject callback) {
        if (callback) {
            dynxx_js_set_msg_callback(js_msg_callback);
            sJsMsgCallback = env->NewGlobalRef(callback);
            /*if (auto jCallbackClass = env->GetObjectClass(callback))
            {
                sJsMsgCallbackMethodId = env->GetMethodID(jCallbackClass, "invoke", "(" LJLS_ ")" LJLS_);
            }*/
            sJsMsgCallbackMethodId = getLambdaMethod(env, KF1, "(" LJLO_ ")" LJLO_);
        }
    }

// SQLite

    jlong sqliteOpen(JNIEnv *env, jobject thiz,
                          jstring id) {
        auto cId = env->GetStringUTFChars(id, nullptr);
        const auto res = dynxx_sqlite_open(cId);
        env->ReleaseStringUTFChars(id, cId);
        return ptr2addr(res);
    }

    jboolean sqliteExecute(JNIEnv *env, jobject thiz,
                                jlong conn,
                                jstring sql) {
        auto cSql = env->GetStringUTFChars(sql, nullptr);
        auto res = dynxx_sqlite_execute(addr2ptr(conn), cSql);
        env->ReleaseStringUTFChars(sql, cSql);
        return res;
    }

    jlong sqliteQueryDo(JNIEnv *env, jobject thiz,
                             jlong conn,
                             jstring sql) {
        auto cSql = env->GetStringUTFChars(sql, nullptr);
        const auto res = dynxx_sqlite_query_do(addr2ptr(conn), cSql);
        env->ReleaseStringUTFChars(sql, cSql);
        return ptr2addr(res);
    }

    jboolean sqliteQueryReadRow(JNIEnv *env, jobject thiz,
                                     jlong query_result) {
        return dynxx_sqlite_query_read_row(addr2ptr(query_result));
    }

    jstring sqliteQueryReadColumnText(JNIEnv *env, jobject thiz,
                                           jlong query_result,
                                           jstring column) {
        auto cColumn = env->GetStringUTFChars(column, nullptr);
        auto cRes = dynxx_sqlite_query_read_column_text(addr2ptr(query_result), cColumn);
        auto jStr = boxJString(env, cRes);
        freeX(cRes);
        env->ReleaseStringUTFChars(column, cColumn);
        return jStr;
    }

    jlong sqliteQueryReadColumnInteger(JNIEnv *env, jobject thiz,
                                            jlong query_result,
                                            jstring column) {
        auto cColumn = env->GetStringUTFChars(column, nullptr);
        auto res = dynxx_sqlite_query_read_column_integer(addr2ptr(query_result), cColumn);
        env->ReleaseStringUTFChars(column, cColumn);
        return res;
    }

    jdouble sqliteQueryReadColumnFloat(JNIEnv *env, jobject thiz,
                                            jlong query_result,
                                            jstring column) {
        auto cColumn = env->GetStringUTFChars(column, nullptr);
        auto res = dynxx_sqlite_query_read_column_float(addr2ptr(query_result), cColumn);
        env->ReleaseStringUTFChars(column, cColumn);
        return res;
    }

    void sqliteQueryDrop(JNIEnv *env, jobject thiz,
                              jlong query_result) {
        dynxx_sqlite_query_drop(addr2ptr(query_result));
    }

    void sqliteClose(JNIEnv *env, jobject thiz,
                          jlong conn) {
        dynxx_sqlite_close(addr2ptr(conn));
    }

// Srore.KV

    jlong kvOpen(JNIEnv *env, jobject thiz,
                      jstring id) {
        auto cId = env->GetStringUTFChars(id, nullptr);
        const auto res = dynxx_kv_open(cId);
        env->ReleaseStringUTFChars(id, cId);
        return ptr2addr(res);
    }

    jstring kvReadString(JNIEnv *env, jobject thiz,
                              jlong conn,
                              jstring k) {
        auto cK = env->GetStringUTFChars(k, nullptr);
        auto cRes = dynxx_kv_read_string(addr2ptr(conn), cK);
        auto jStr = boxJString(env, cRes);
        freeX(cRes);
        env->ReleaseStringUTFChars(k, cK);
        return jStr;
    }

    jboolean kvWriteString(JNIEnv *env, jobject thiz,
                                jlong conn,
                                jstring k, jstring v) {
        auto cK = env->GetStringUTFChars(k, nullptr);
        auto cV = env->GetStringUTFChars(v, nullptr);
        auto res = dynxx_kv_write_string(addr2ptr(conn), cK, cV);
        env->ReleaseStringUTFChars(v, cV);
        env->ReleaseStringUTFChars(k, cK);
        return res;
    }

    jlong kvReadInteger(JNIEnv *env, jobject thiz,
                             jlong conn,
                             jstring k) {
        auto cK = env->GetStringUTFChars(k, nullptr);
        auto res = dynxx_kv_read_integer(addr2ptr(conn), cK);
        env->ReleaseStringUTFChars(k, cK);
        return res;
    }

    jboolean kvWriteInteger(JNIEnv *env, jobject thiz,
                                 jlong conn,
                                 jstring k, jlong v) {
        auto cK = env->GetStringUTFChars(k, nullptr);
        auto res = dynxx_kv_write_integer(addr2ptr(conn), cK, v);
        env->ReleaseStringUTFChars(k, cK);
        return res;
    }

    jdouble kvReadFloat(JNIEnv *env, jobject thiz,
                             jlong conn,
                             jstring k) {
        auto cK = env->GetStringUTFChars(k, nullptr);
        auto res = dynxx_kv_read_float(addr2ptr(conn), cK);
        env->ReleaseStringUTFChars(k, cK);
        return res;
    }

    jboolean kvWriteFloat(JNIEnv *env, jobject thiz,
                               jlong conn,
                               jstring k, jdouble v) {
        auto cK = env->GetStringUTFChars(k, nullptr);
        auto res = dynxx_kv_write_float(addr2ptr(conn), cK, v);
        env->ReleaseStringUTFChars(k, cK);
        return res;
    }

    jboolean kvContains(JNIEnv *env, jobject thiz,
                             jlong conn,
                             jstring k) {
        auto cK = env->GetStringUTFChars(k, nullptr);
        auto res = dynxx_kv_contains(addr2ptr(conn), cK);
        env->ReleaseStringUTFChars(k, cK);
        return res;
    }

    jboolean kvRemove(JNIEnv *env, jobject thiz,
                           jlong conn,
                           jstring k) {
        auto cK = env->GetStringUTFChars(k, nullptr);
        auto res = dynxx_kv_remove(addr2ptr(conn), cK);
        env->ReleaseStringUTFChars(k, cK);
        return res;
    }

    void kvClear(JNIEnv *env, jobject thiz,
                      jlong conn) {
        dynxx_kv_clear(addr2ptr(conn));
    }

    void kvClose(JNIEnv *env, jobject thiz,
                      jlong conn) {
        dynxx_kv_close(addr2ptr(conn));
    }

// Device

    jint deviceType(JNIEnv *env, jobject thiz) {
        return dynxx_device_type();
    }

    jstring deviceName(JNIEnv *env, jobject thiz) {
        auto cDN = dynxx_device_name();
        auto jStr = boxJString(env, cDN);
        freeX(cDN);
        return jStr;
    }

    jstring deviceManufacturer(JNIEnv *env, jobject thiz) {
        auto cDM = dynxx_device_manufacturer();
        auto jStr = boxJString(env, cDM);
        freeX(cDM);
        return jStr;
    }

    jstring deviceOsVersion(JNIEnv *env, jobject thiz) {
        auto cDOV = dynxx_device_os_version();
        auto jStr = boxJString(env, cDOV);
        freeX(cDOV);
        return jStr;
    }

    jint deviceCpuArch(JNIEnv *env, jobject thiz) {
        return dynxx_device_cpu_arch();
    }

// Coding

    jstring codingHexBytes2Str(JNIEnv *env, jobject thiz,
                               jbyteArray bytes) {
        auto cIn = env->GetByteArrayElements(bytes, nullptr);
        auto inLen = env->GetArrayLength(bytes);

        auto cRes = dynxx_coding_hex_bytes2str(reinterpret_cast<const byte *>(cIn), inLen);
        auto jStr = boxJString(env, cRes);

        freeX(cRes);
        env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
        return jStr;
    }

    jbyteArray codingHexStr2Bytes(JNIEnv *env,
                                  jobject thiz,
                                  jstring str) {
        auto cStr = env->GetStringUTFChars(str, nullptr);

        size_t outLen;
        const auto cRes = dynxx_coding_hex_str2bytes(cStr, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseStringUTFChars(str, cStr);
        return jba;
    }

// Crypto

    jbyteArray cryptoRandom(JNIEnv *env, jobject thiz,
                            jint len) {
        const auto cRes = dynxx_crypto_rand(len);
        return moveToJByteArray(env, cRes, len, false);
    }

    jbyteArray cryptoAesEncrypt(JNIEnv *env, jobject thiz,
                                jbyteArray input,
                                jbyteArray key) {
        auto cIn = env->GetByteArrayElements(input, nullptr);
        auto inLen = env->GetArrayLength(input);
        auto cKey = env->GetByteArrayElements(key, nullptr);
        auto keyLen = env->GetArrayLength(key);

        size_t outLen;
        const auto cRes = dynxx_crypto_aes_encrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                                    reinterpret_cast<const byte *>(cKey), keyLen,
                                                    &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
        env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
        return jba;
    }

    jbyteArray cryptoAesDecrypt(JNIEnv *env, jobject thiz,
                                jbyteArray input,
                                jbyteArray key) {
        auto cIn = env->GetByteArrayElements(input, nullptr);
        auto inLen = env->GetArrayLength(input);
        auto cKey = env->GetByteArrayElements(key, nullptr);
        auto keyLen = env->GetArrayLength(key);

        size_t outLen;
        const auto cRes = dynxx_crypto_aes_decrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                                    reinterpret_cast<const byte *>(cKey), keyLen,
                                                    &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
        env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
        return jba;
    }

    jbyteArray cryptoAesGcmEncrypt(JNIEnv *env, jobject thiz,
                                   jbyteArray input,
                                   jbyteArray key,
                                   jbyteArray init_vector,
                                   jbyteArray aad,
                                   jint tag_bits) {
        auto cIn = env->GetByteArrayElements(input, nullptr);
        auto inLen = env->GetArrayLength(input);
        auto cKey = env->GetByteArrayElements(key, nullptr);
        auto keyLen = env->GetArrayLength(key);
        auto cIv = env->GetByteArrayElements(init_vector, nullptr);
        auto ivLen = env->GetArrayLength(init_vector);
        auto cAad = aad ? env->GetByteArrayElements(aad, nullptr) : nullptr;
        auto aadLen = aad ? env->GetArrayLength(aad) : 0;

        size_t outLen;
        const auto cRes = dynxx_crypto_aes_gcm_encrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                                        reinterpret_cast<const byte *>(cKey),
                                                        keyLen,
                                                        reinterpret_cast<const byte *>(cIv), ivLen,
                                                        reinterpret_cast<const byte *>(cAad),
                                                        aadLen,
                                                        tag_bits, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
        env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
        env->ReleaseByteArrayElements(init_vector, cIv, JNI_ABORT);
        if (cAad) {
            env->ReleaseByteArrayElements(aad, cAad, JNI_ABORT);
        }
        return jba;
    }

    jbyteArray cryptoAesGcmDecrypt(JNIEnv *env, jobject thiz,
                                   jbyteArray input,
                                   jbyteArray key,
                                   jbyteArray init_vector,
                                   jbyteArray aad,
                                   jint tag_bits) {
        auto cIn = env->GetByteArrayElements(input, nullptr);
        auto inLen = env->GetArrayLength(input);
        auto cKey = env->GetByteArrayElements(key, nullptr);
        auto keyLen = env->GetArrayLength(key);
        auto cIv = env->GetByteArrayElements(init_vector, nullptr);
        auto ivLen = env->GetArrayLength(init_vector);
        auto cAad = aad ? env->GetByteArrayElements(aad, nullptr) : nullptr;
        auto aadLen = aad ? env->GetArrayLength(aad) : 0;

        size_t outLen;
        const auto cRes = dynxx_crypto_aes_gcm_decrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                                        reinterpret_cast<const byte *>(cKey),
                                                        keyLen,
                                                        reinterpret_cast<const byte *>(cIv), ivLen,
                                                        reinterpret_cast<const byte *>(cAad),
                                                        aadLen,
                                                        tag_bits, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
        env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
        env->ReleaseByteArrayElements(init_vector, cIv, JNI_ABORT);
        if (cAad) {
            env->ReleaseByteArrayElements(aad, cAad, JNI_ABORT);
        }
        return jba;
    }

    jbyteArray cryptoHashMd5(JNIEnv *env, jobject thiz,
                             jbyteArray input) {
        auto cIn = env->GetByteArrayElements(input, nullptr);
        auto inLen = env->GetArrayLength(input);

        size_t outLen;
        const auto cRes = dynxx_crypto_hash_md5(reinterpret_cast<const byte *>(cIn), inLen,
                                                 &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
        return jba;
    }

    jbyteArray cryptoHashSha1(JNIEnv *env, jobject thiz,
                                jbyteArray input) {
        auto cIn = env->GetByteArrayElements(input, nullptr);
        auto inLen = env->GetArrayLength(input);

        size_t outLen;
        const auto cRes = dynxx_crypto_hash_sha1(reinterpret_cast<const byte *>(cIn), inLen,
                                                   &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
        return jba;
    }

    jbyteArray cryptoHashSha256(JNIEnv *env, jobject thiz,
                                jbyteArray input) {
        auto cIn = env->GetByteArrayElements(input, nullptr);
        auto inLen = env->GetArrayLength(input);

        size_t outLen;
        const auto cRes = dynxx_crypto_hash_sha256(reinterpret_cast<const byte *>(cIn), inLen,
                                                    &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
        return jba;
    }

    jbyteArray cryptoBase64Encode(JNIEnv *env, jobject thiz,
                                  jbyteArray input,
                                  jboolean noNewLines) {
        auto cIn = env->GetByteArrayElements(input, nullptr);
        auto inLen = env->GetArrayLength(input);

        size_t outLen;
        const auto cRes = dynxx_crypto_base64_encode(reinterpret_cast<const byte *>(cIn), inLen,
                                                      noNewLines, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
        return jba;
    }

    jbyteArray cryptoBase64Decode(JNIEnv *env, jobject thiz,
                                  jbyteArray input,
                                  jboolean noNewLines) {
        auto cIn = env->GetByteArrayElements(input, nullptr);
        auto inLen = env->GetArrayLength(input);

        size_t outLen;
        const auto cRes = dynxx_crypto_base64_decode(reinterpret_cast<const byte *>(cIn), inLen,
                                                      noNewLines, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
        return jba;
    }

// JsonDecoder

    jint jsonReadType(JNIEnv *env, jobject thiz,
                      jlong node) {
        return dynxx_json_read_type(addr2ptr(node));
    }

    jlong jsonDecoderInit(JNIEnv *env, jobject thiz,
                          jstring json) {
        auto cJson = env->GetStringUTFChars(json, nullptr);
        const auto res = dynxx_json_decoder_init(cJson);
        env->ReleaseStringUTFChars(json, cJson);
        return ptr2addr(res);
    }

    jlong jsonDecoderReadNode(JNIEnv *env, jobject thiz,
                              jlong decoder,
                              jlong node,
                              jstring k) {
        auto cK = env->GetStringUTFChars(k, nullptr);
        const auto res = dynxx_json_decoder_read_node(addr2ptr(decoder), addr2ptr(node), cK);
        env->ReleaseStringUTFChars(k, cK);
        return ptr2addr(res);
    }

    jstring jsonDecoderReadString(JNIEnv *env, jobject thiz,
                                  jlong decoder,
                                  jlong node) {
        auto cRes = dynxx_json_decoder_read_string(addr2ptr(decoder), addr2ptr(node));
        auto jStr = boxJString(env, cRes);
        freeX(cRes);
        return jStr;
    }

    jdouble jsonDecoderReadNumber(JNIEnv *env, jobject thiz,
                                  jlong decoder,
                                  jlong node) {
        return dynxx_json_decoder_read_number(addr2ptr(decoder), addr2ptr(node));
    }

    jlong jsonDecoderReadChild(JNIEnv *env, jobject thiz,
                               jlong decoder,
                               jlong node) {
        return ptr2addr(dynxx_json_decoder_read_child(addr2ptr(decoder), addr2ptr(node)));
    }

    jlong jsonDecoderReadNext(JNIEnv *env, jobject thiz,
                              jlong decoder,
                              jlong node) {
        return ptr2addr(dynxx_json_decoder_read_next(addr2ptr(decoder), addr2ptr(node)));
    }

    void jsonDecoderRelease(JNIEnv *env, jobject thiz,
                            jlong decoder) {
        dynxx_json_decoder_release(addr2ptr(decoder));
    }

// Zip

    jlong zZipInit(JNIEnv *env, jobject thiz,
                   jint mode, jlong bufferSize, jint format) {
        return ptr2addr(dynxx_z_zip_init(mode, bufferSize, format));
    }

    jlong zZipInput(JNIEnv *env, jobject thiz,
                    jlong zip,
                    jbyteArray inBytes, jint inLen, jboolean inFinish) {
        auto cIn = env->GetByteArrayElements(inBytes, nullptr);
        // auto inLen = env->GetArrayLength(input);

        auto ret = dynxx_z_zip_input(addr2ptr(zip), reinterpret_cast<const byte *>(cIn), inLen,
                                      inFinish);

        env->ReleaseByteArrayElements(inBytes, cIn, JNI_ABORT);

        return static_cast<jlong>(ret);
    }

    jbyteArray zZipProcessDo(JNIEnv *env, jobject thiz, jlong zip) {
        size_t outLen = 0;
        const auto cRes = dynxx_z_zip_process_do(addr2ptr(zip), &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        return jba;
    }

    jboolean zZipProcessFinished(JNIEnv *env, jobject thiz,
                                 jlong zip) {
        return dynxx_z_zip_process_finished(addr2ptr(zip));
    }

    void zZipRelease(JNIEnv *env, jobject thiz,
                     jlong zip) {
        dynxx_z_zip_release(addr2ptr(zip));
    }

    jlong zUnZipInit(JNIEnv *env, jobject thiz,
                     jlong bufferSize, jint format) {
        return ptr2addr(dynxx_z_unzip_init(bufferSize, format));
    }

    jlong zUnZipInput(JNIEnv *env, jobject thiz,
                      jlong unzip,
                      jbyteArray inBytes, jint inLen, jboolean inFinish) {
        auto cIn = env->GetByteArrayElements(inBytes, nullptr);
        // auto inLen = env->GetArrayLength(input);

        auto ret = dynxx_z_unzip_input(addr2ptr(unzip), reinterpret_cast<const byte *>(cIn), inLen,
                                        inFinish);

        env->ReleaseByteArrayElements(inBytes, cIn, JNI_ABORT);

        return static_cast<jlong>(ret);
    }

    jbyteArray zUnZipProcessDo(JNIEnv *env, jobject thiz, jlong unzip) {
        size_t outLen = 0;
        const auto cRes = dynxx_z_unzip_process_do(addr2ptr(unzip), &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        return jba;
    }

    jboolean zUnZipProcessFinished(JNIEnv *env, jobject thiz,
                                   jlong unzip) {
        return dynxx_z_unzip_process_finished(addr2ptr(unzip));
    }

    void zUnZipRelease(JNIEnv *env, jobject thiz,
                       jlong unzip) {
        dynxx_z_unzip_release(addr2ptr(unzip));
    }

    jbyteArray zZipBytes(JNIEnv *env, jobject thiz,
                         jint mode,
                         jlong buffer_size, jint format, jbyteArray bytes) {
        auto cIn = env->GetByteArrayElements(bytes, nullptr);
        auto inLen = env->GetArrayLength(bytes);

        size_t outLen;
        const auto cRes = dynxx_z_bytes_zip(mode, buffer_size, format,
                                             reinterpret_cast<const byte *>(cIn), inLen, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
        return jba;
    }

    jbyteArray zUnZipBytes(JNIEnv *env, jobject thiz,
                           jlong buffer_size, jint format, jbyteArray bytes) {
        auto cIn = env->GetByteArrayElements(bytes, nullptr);
        auto inLen = env->GetArrayLength(bytes);

        size_t outLen;
        const auto cRes = dynxx_z_bytes_unzip(buffer_size, format,
                                               reinterpret_cast<const byte *>(cIn), inLen, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen, true);

        env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
        return jba;
    }

    const JNINativeMethod JCFuncList[] = {
            DECLARE_JNI_FUNC(getVersion, "()" LJLS_),
            DECLARE_JNI_FUNC(init, "(" LJLS_ ")Z"),
            DECLARE_JNI_FUNC(release, "()V"),

            DECLARE_JNI_FUNC(logSetLevel, "(I)V"),
            DECLARE_JNI_FUNC(logSetCallback, "(" LKF2_ ")V"),
            DECLARE_JNI_FUNC(logPrint, "(I" LJLS_ ")V"),

            DECLARE_JNI_FUNC(netHttpRequest,
                             "(" LJLS_ LJLS_ "I[" LJLS_ "[" LJLS_ "[" LJLS_ "[" LJLS_ LJLS_ "JJ)" LJLS_),

            DECLARE_JNI_FUNC(lLoadF, "(" LJLS_ ")Z"),
            DECLARE_JNI_FUNC(lLoadS, "(" LJLS_ ")Z"),
            DECLARE_JNI_FUNC(lCall, "(" LJLS_ LJLS_ ")" LJLS_),

            DECLARE_JNI_FUNC(jLoadF, "(" LJLS_ "Z)Z"),
            DECLARE_JNI_FUNC(jLoadS, "(" LJLS_ LJLS_ "Z)Z"),
            DECLARE_JNI_FUNC(jLoadB, "([BZ)Z"),
            DECLARE_JNI_FUNC(jCall, "(" LJLS_ LJLS_ "Z)" LJLS_),
            DECLARE_JNI_FUNC(jSetMsgCallback, "(" LKF1_ ")V"),

            DECLARE_JNI_FUNC(sqliteOpen, "(" LJLS_ ")J"),
            DECLARE_JNI_FUNC(sqliteExecute, "(J" LJLS_ ")Z"),
            DECLARE_JNI_FUNC(sqliteQueryDo, "(J" LJLS_ ")J"),
            DECLARE_JNI_FUNC(sqliteQueryReadRow, "(J)Z"),
            DECLARE_JNI_FUNC(sqliteQueryReadColumnText, "(J" LJLS_ ")" LJLS_),
            DECLARE_JNI_FUNC(sqliteQueryReadColumnInteger, "(J" LJLS_ ")J"),
            DECLARE_JNI_FUNC(sqliteQueryReadColumnFloat, "(J" LJLS_ ")D"),
            DECLARE_JNI_FUNC(sqliteQueryDrop, "(J)V"),
            DECLARE_JNI_FUNC(sqliteClose, "(J)V"),

            DECLARE_JNI_FUNC(kvOpen, "(" LJLS_ ")J"),
            DECLARE_JNI_FUNC(kvReadString, "(J" LJLS_ ")" LJLS_),
            DECLARE_JNI_FUNC(kvWriteString, "(J" LJLS_ LJLS_ ")Z"),
            DECLARE_JNI_FUNC(kvReadInteger, "(J" LJLS_ ")J"),
            DECLARE_JNI_FUNC(kvWriteInteger, "(J" LJLS_ "J)Z"),
            DECLARE_JNI_FUNC(kvReadFloat, "(J" LJLS_ ")D"),
            DECLARE_JNI_FUNC(kvWriteFloat, "(J" LJLS_ "D)Z"),
            DECLARE_JNI_FUNC(kvContains, "(J" LJLS_ ")Z"),
            DECLARE_JNI_FUNC(kvRemove, "(J" LJLS_ ")Z"),
            DECLARE_JNI_FUNC(kvClear, "(J)V"),
            DECLARE_JNI_FUNC(kvClose, "(J)V"),

            DECLARE_JNI_FUNC(deviceType, "()I"),
            DECLARE_JNI_FUNC(deviceName, "()" LJLS_),
            DECLARE_JNI_FUNC(deviceManufacturer, "()" LJLS_),
            DECLARE_JNI_FUNC(deviceOsVersion, "()" LJLS_),
            DECLARE_JNI_FUNC(deviceCpuArch, "()I"),

            DECLARE_JNI_FUNC(codingHexBytes2Str, "([B)" LJLS_),
            DECLARE_JNI_FUNC(codingHexStr2Bytes, "(" LJLS_ ")[B"),

            DECLARE_JNI_FUNC(cryptoRandom, "(I)[B"),
            DECLARE_JNI_FUNC(cryptoAesEncrypt, "([B[B)[B"),
            DECLARE_JNI_FUNC(cryptoAesDecrypt, "([B[B)[B"),
            DECLARE_JNI_FUNC(cryptoAesGcmEncrypt, "([B[B[B[BI)[B"),
            DECLARE_JNI_FUNC(cryptoAesGcmDecrypt, "([B[B[B[BI)[B"),
            DECLARE_JNI_FUNC(cryptoHashMd5, "([B)[B"),
            DECLARE_JNI_FUNC(cryptoHashSha1, "([B)[B"),
            DECLARE_JNI_FUNC(cryptoHashSha256, "([B)[B"),
            DECLARE_JNI_FUNC(cryptoBase64Encode, "([BZ)[B"),
            DECLARE_JNI_FUNC(cryptoBase64Decode, "([BZ)[B"),

            DECLARE_JNI_FUNC(jsonReadType, "(J)I"),
            DECLARE_JNI_FUNC(jsonDecoderInit, "(" LJLS_ ")J"),
            DECLARE_JNI_FUNC(jsonDecoderReadNode, "(JJ" LJLS_ ")J"),
            DECLARE_JNI_FUNC(jsonDecoderReadString, "(JJ)" LJLS_),
            DECLARE_JNI_FUNC(jsonDecoderReadNumber, "(JJ)D"),
            DECLARE_JNI_FUNC(jsonDecoderReadChild, "(JJ)J"),
            DECLARE_JNI_FUNC(jsonDecoderReadNext, "(JJ)J"),
            DECLARE_JNI_FUNC(jsonDecoderRelease, "(J)V"),

            DECLARE_JNI_FUNC(zZipInit, "(IJI)J"),
            DECLARE_JNI_FUNC(zZipInput, "(J[BIZ)J"),
            DECLARE_JNI_FUNC(zZipProcessDo, "(J)[B"),
            DECLARE_JNI_FUNC(zZipProcessFinished, "(J)Z"),
            DECLARE_JNI_FUNC(zZipRelease, "(J)V"),
            DECLARE_JNI_FUNC(zUnZipInit, "(JI)J"),
            DECLARE_JNI_FUNC(zUnZipInput, "(J[BIZ)J"),
            DECLARE_JNI_FUNC(zUnZipProcessDo, "(J)[B"),
            DECLARE_JNI_FUNC(zUnZipProcessFinished, "(J)Z"),
            DECLARE_JNI_FUNC(zUnZipRelease, "(J)V"),
            DECLARE_JNI_FUNC(zZipBytes, "(IJI[B)[B"),
            DECLARE_JNI_FUNC(zUnZipBytes, "(JI[B)[B"),
    };

}

int JNI_OnLoad(JavaVM *vm, void *reserved) {
    sVM = vm;
    JNIEnv *env;
    auto v = JNI_VERSION_1_6;
    auto ret = vm->GetEnv(reinterpret_cast<void **>(&env), v);
    if (ret != JNI_OK) {
        return JNI_ERR;
    }

    sJClass = env->FindClass(JClassName);
    if (sJClass == nullptr) {
        return JNI_ERR;
    }
    ret = env->RegisterNatives(sJClass, JCFuncList, sizeof(JCFuncList) / sizeof(JNINativeMethod));
    if (ret != JNI_OK) {
        return JNI_ERR;
    }
    return v;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
    dynxx_log_set_callback(nullptr);
    dynxx_js_set_msg_callback(nullptr);

    auto env = currentEnv(vm);
    if (env) {
        env->UnregisterNatives(sJClass);
        env->DeleteGlobalRef(sLogCallback);
        env->DeleteGlobalRef(sJsMsgCallback);
    }

    sVM = nullptr;
    sLogCallback = nullptr;
    sJsMsgCallback = nullptr;
}
