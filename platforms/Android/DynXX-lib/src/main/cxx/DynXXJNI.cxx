#include <cstring>

#include "JNIUtil.hxx"
#include "../../../../../../build.Android/output/include/DynXX/C/DynXX.h"

namespace {
    JavaVM *sVM;
    jclass sJClass;
    jobject sLogCallback;
    jmethodID sLogCallbackMethodId;
    jobject sJsMsgCallback;
    jmethodID sJsMsgCallbackMethodId;

    constexpr auto JClassName = "xyz/rinc/dynxx/DynXX$Companion";

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
        auto cRes = readJString(env, jRes);
        auto newRes = dupStr(cRes);
        if (cRes) {
           releaseJString(env, jRes, cRes);
        }
        env->DeleteLocalRef(jMsg);
        env->DeleteLocalRef(jRes);
        return newRes;
    }

// Engine base API

    jstring getVersion(JNIEnv *env, [[maybe_unused]] jobject thiz) {
        auto cV = dynxx_get_version();
        return boxJString(env, cV);
    }

    jboolean init(JNIEnv *env, [[maybe_unused]] jobject thiz,
                  jstring root) {
        auto cRoot = readJString(env, root);
        auto res = dynxx_init(cRoot);
        releaseJString(env, root, cRoot);
        return res;
    }

    void release([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz) {
        dynxx_release();
    }

// Log

    void logSetLevel([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                     jint level) {
        dynxx_log_set_level(static_cast<DynXXLogLevel>(level));
    }

    void logSetCallback(JNIEnv *env, [[maybe_unused]] jobject thiz,
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

    void logPrint(JNIEnv *env, [[maybe_unused]] jobject thiz,
                  jint level, jstring content) {
        auto cContent = readJString(env, content);
        dynxx_log_print(static_cast<DynXXLogLevel>(level), cContent);
        releaseJString(env, content, cContent);
    }

// Net

    jstring netHttpRequest(JNIEnv *env, [[maybe_unused]] jobject thiz,
                           jstring url, jstring params, jint method,
                           jobjectArray headerV, jobjectArray formFieldNameV, jobjectArray formFieldMimeV, jobjectArray formFieldDataV,
                           jstring filePath, jlong fileLength,
                           jlong timeout) {
        auto cUrl = readJString(env, url);
        auto cParams = readJString(env, params);

        auto headerCount = env->GetArrayLength(headerV);

        auto cHeaderV = mallocX<const char *>(headerCount);
        auto jStrHeaderV = mallocX<jstring>(headerCount);

        for (int i = 0; i < headerCount; i++) {
            jStrHeaderV[i] = readJObjectArrayItem<jstring>(env, headerV, i);
            cHeaderV[i] = readJString(env, jStrHeaderV[i]);
        }

        auto formFieldCount = env->GetArrayLength(formFieldNameV);
        auto cFormFieldNameV = mallocX<const char *>(formFieldCount);
        auto jStrFormFieldNameV = mallocX<jstring>(formFieldCount);
        auto cFormFieldMimeV = mallocX<const char *>(formFieldCount);
        auto jStrFormFieldMimeV = mallocX<jstring>(formFieldCount);
        auto cFormFieldDataV = mallocX<const char *>(formFieldCount);
        auto jStrFormFieldDataV = mallocX<jstring>(formFieldCount);

        for (int i = 0; i < formFieldCount; i++) {
            jStrFormFieldNameV[i] = readJObjectArrayItem<jstring>(env, formFieldNameV, i);
            cFormFieldNameV[i] = readJString(env, jStrFormFieldNameV[i]);
            jStrFormFieldMimeV[i] = readJObjectArrayItem<jstring>(env,formFieldMimeV, i);
            cFormFieldMimeV[i] = readJString(env, jStrFormFieldMimeV[i]);
            jStrFormFieldDataV[i] = readJObjectArrayItem<jstring>(env,formFieldDataV, i);
            cFormFieldDataV[i] = readJString(env, jStrFormFieldDataV[i]);
        }

        auto cFilePath = filePath ? readJString(env, filePath) : "";
        auto cFILE = cFilePath ? std::fopen(cFilePath, "r") : nullptr;

        auto cRsp = dynxx_net_http_request(cUrl, cParams, static_cast<DynXXHttpMethod>(method),
                                            cHeaderV, headerCount,
                                            cFormFieldNameV, cFormFieldMimeV, cFormFieldDataV, formFieldCount,
                                            cFILE, fileLength,
                                            static_cast<const size_t>(timeout));
        auto jStr = boxJString(env, cRsp);

        for (int i = 0; i < headerCount; i++) {
            releaseJString(env, jStrHeaderV[i], cHeaderV[i]);
        }
        for (int i = 0; i < formFieldCount; i++) {
            releaseJString(env, jStrFormFieldNameV[i], cFormFieldNameV[i]);
            releaseJString(env, jStrFormFieldMimeV[i], cFormFieldMimeV[i]);
            releaseJString(env, jStrFormFieldDataV[i], cFormFieldDataV[i]);
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
            releaseJString(env, filePath, cFilePath);
        }
        if (cFILE) {
            std::fclose(cFILE);
        }
        if (cParams) {
            releaseJString(env, params, cParams);
        }
        releaseJString(env, url, cUrl);

        return jStr;
    }

// Lua

    jboolean lLoadF(JNIEnv *env, [[maybe_unused]] jobject thiz,
                    jstring file) {
        auto cFile = readJString(env, file);
        auto res = dynxx_lua_loadF(cFile);
        releaseJString(env, file, cFile);
        return res;
    }

    jboolean lLoadS(JNIEnv *env, [[maybe_unused]] jobject thiz,
                    jstring script) {
        auto cScript = readJString(env, script);
        auto res = dynxx_lua_loadS(cScript);
        releaseJString(env, script, cScript);
        return res;
    }

    jstring lCall(JNIEnv *env, [[maybe_unused]] jobject thiz,
                  jstring func, jstring params) {
        auto cFunc = readJString(env, func);
        auto cParams = readJString(env, params);
        auto cRes = dynxx_lua_call(cFunc, cParams);
        auto jStr = boxJString(env, cRes);
        if (cParams) {
            releaseJString(env, params, cParams);
        }
        releaseJString(env, func, cFunc);
        return jStr;
    }

// JS

    jboolean jLoadF(JNIEnv *env, [[maybe_unused]] jobject thiz,
                    jstring file, jboolean is_module) {
        auto cFile = readJString(env, file);
        auto res = dynxx_js_loadF(cFile, is_module);
        releaseJString(env, file, cFile);
        return res;
    }

    jboolean jLoadS(JNIEnv *env, [[maybe_unused]] jobject thiz,
                    jstring script, jstring name, jboolean is_module) {
        auto cScript = readJString(env, script);
        auto cName = readJString(env, name);
        auto res = dynxx_js_loadS(cScript, cName, is_module);
        releaseJString(env, script, cScript);
        releaseJString(env, name, cName);
        return res;
    }

    jboolean jLoadB(JNIEnv *env, [[maybe_unused]] jobject thiz,
                    jbyteArray bytes, jboolean is_module) {
        auto [cIn, inLen] = readJBytes(env, bytes);

        auto res = dynxx_js_loadB(cIn, inLen, is_module);

        releaseJBytes(env, bytes, cIn);
        return res;
    }

    jstring jCall(JNIEnv *env, [[maybe_unused]] jobject thiz,
                  jstring func, jstring params, jboolean await) {
        auto cFunc = readJString(env, func);
        auto cParams = readJString(env, params);
        auto cRes = dynxx_js_call(cFunc, cParams, await);
        auto jStr = boxJString(env, cRes);
        if (cParams) {
            releaseJString(env, params, cParams);
        }
        releaseJString(env, func, cFunc);
        return jStr;
    }

    void jSetMsgCallback(JNIEnv *env, [[maybe_unused]] jobject thiz,
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

    jlong sqliteOpen(JNIEnv *env, [[maybe_unused]] jobject thiz,
                          jstring id) {
        auto cId = readJString(env, id);
        const auto res = dynxx_sqlite_open(cId);
        releaseJString(env, id, cId);
        return res;
    }

    jboolean sqliteExecute(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jlong conn, jstring sql) {
        auto cSql = readJString(env, sql);
        auto res = dynxx_sqlite_execute(conn, cSql);
        releaseJString(env, sql, cSql);
        return res;
    }

    jlong sqliteQueryDo(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jlong conn, jstring sql) {
        auto cSql = readJString(env, sql);
        const auto res = dynxx_sqlite_query_do(conn, cSql);
        releaseJString(env, sql, cSql);
        return res;
    }

    jboolean sqliteQueryReadRow([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                                     jlong query_result) {
        return dynxx_sqlite_query_read_row(query_result);
    }

    jstring sqliteQueryReadColumnText(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                           jlong query_result, jstring column) {
        auto cColumn = readJString(env, column);
        auto cRes = dynxx_sqlite_query_read_column_text(query_result, cColumn);
        auto jStr = boxJString(env, cRes);
        releaseJString(env, column, cColumn);
        return jStr;
    }

    jlong sqliteQueryReadColumnInteger(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                            jlong query_result, jstring column) {
        auto cColumn = readJString(env, column);
        auto res = dynxx_sqlite_query_read_column_integer(query_result, cColumn);
        releaseJString(env, column, cColumn);
        return res;
    }

    jdouble sqliteQueryReadColumnFloat(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                       jlong query_result, jstring column) {
        auto cColumn = readJString(env, column);
        auto res = dynxx_sqlite_query_read_column_float(query_result, cColumn);
        releaseJString(env, column, cColumn);
        return res;
    }

    void sqliteQueryDrop([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                         jlong query_result) {
        dynxx_sqlite_query_drop(query_result);
    }

    void sqliteClose([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                          jlong conn) {
        dynxx_sqlite_close(conn);
    }

// Srore.KV

    jlong kvOpen(JNIEnv *env, [[maybe_unused]] jobject thiz,
                      jstring id) {
        auto cId = readJString(env, id);
        const auto res = dynxx_kv_open(cId);
        releaseJString(env, id, cId);
        return res;
    }

    jstring kvReadString(JNIEnv *env, [[maybe_unused]] jobject thiz,
                              jlong conn, jstring k) {
        auto cK = readJString(env, k);
        auto cRes = dynxx_kv_read_string(conn, cK);
        auto jStr = boxJString(env, cRes);
        releaseJString(env, k, cK);
        return jStr;
    }

    jboolean kvWriteString(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jlong conn, jstring k, jstring v) {
        auto cK = readJString(env, k);
        auto cV = readJString(env, v);
        auto res = dynxx_kv_write_string(conn, cK, cV);
        releaseJString(env, v, cV);
        releaseJString(env, k, cK);
        return res;
    }

    jlong kvReadInteger(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jlong conn, jstring k) {
        auto cK = readJString(env, k);
        auto res = dynxx_kv_read_integer(conn, cK);
        releaseJString(env, k, cK);
        return res;
    }

    jboolean kvWriteInteger(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                 jlong conn, jstring k, jlong v) {
        auto cK = readJString(env, k);
        auto res = dynxx_kv_write_integer(conn, cK, v);
        releaseJString(env, k, cK);
        return res;
    }

    jdouble kvReadFloat(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jlong conn, jstring k) {
        auto cK = readJString(env, k);
        auto res = dynxx_kv_read_float(conn, cK);
        releaseJString(env, k, cK);
        return res;
    }

    jboolean kvWriteFloat(JNIEnv *env, [[maybe_unused]] jobject thiz,
                               jlong conn, jstring k, jdouble v) {
        auto cK = readJString(env, k);
        auto res = dynxx_kv_write_float(conn, cK, v);
        releaseJString(env, k, cK);
        return res;
    }

    jobjectArray kvAllKeys(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jlong conn) {
        size_t len;
        auto cRes = dynxx_kv_all_keys(conn, &len);
        return moveToJStringArray(env, cRes, len);
    }

    jboolean kvContains(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jlong conn, jstring k) {
        auto cK = readJString(env, k);
        auto res = dynxx_kv_contains(conn, cK);
        releaseJString(env, k, cK);
        return res;
    }

    jboolean kvRemove(JNIEnv *env, [[maybe_unused]] jobject thiz,
                           jlong conn, jstring k) {
        auto cK = readJString(env, k);
        auto res = dynxx_kv_remove(conn, cK);
        releaseJString(env, k, cK);
        return res;
    }

    void kvClear([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                      jlong conn) {
        dynxx_kv_clear(conn);
    }

    void kvClose([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                      jlong conn) {
        dynxx_kv_close(conn);
    }

// Device

    jint deviceType([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz) {
        return dynxx_device_type();
    }

    jstring deviceName(JNIEnv *env, [[maybe_unused]] jobject thiz) {
        auto cDN = dynxx_device_name();
        return boxJString(env, cDN);
    }

    jstring deviceManufacturer(JNIEnv *env, [[maybe_unused]] jobject thiz) {
        auto cDM = dynxx_device_manufacturer();
        return boxJString(env, cDM);
    }

    jstring deviceOsVersion(JNIEnv *env, [[maybe_unused]] jobject thiz) {
        auto cDOV = dynxx_device_os_version();
        return boxJString(env, cDOV);
    }

    jint deviceCpuArch([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz) {
        return dynxx_device_cpu_arch();
    }

// Coding

    jstring codingHexBytes2Str(JNIEnv *env, [[maybe_unused]] jobject thiz,
                               jbyteArray bytes) {
        auto [cIn, inLen] = readJBytes(env, bytes);

        auto cRes = dynxx_coding_hex_bytes2str(cIn, inLen);
        auto jStr = boxJString(env, cRes);
        releaseJBytes(env, bytes, cIn);
        return jStr;
    }

    jbyteArray codingHexStr2Bytes(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                  jstring str) {
        auto cStr = readJString(env, str);

        size_t outLen;
        const auto cRes = dynxx_coding_hex_str2bytes(cStr, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJString(env, str, cStr);
        return jba;
    }

// Crypto

    jbyteArray cryptoRandom(JNIEnv *env, [[maybe_unused]] jobject thiz,
                            jint len) {
        const auto cRes = dynxx_crypto_rand(len);
        return moveToJByteArray(env, cRes, len);
    }

    jbyteArray cryptoAesEncrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input, jbyteArray key) {
        auto [cIn, inLen] = readJBytes(env, input);
        auto [cKey, keyLen] = readJBytes(env, key);

        size_t outLen;
        const auto cRes = dynxx_crypto_aes_encrypt(cIn, inLen,
                                                    cKey, keyLen,
                                                    &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, input, cIn);
        releaseJBytes(env, key, cKey);
        return jba;
    }

    jbyteArray cryptoAesDecrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input, jbyteArray key) {
        auto [cIn, inLen] = readJBytes(env, input);
        auto [cKey, keyLen] = readJBytes(env, key);

        size_t outLen;
        const auto cRes = dynxx_crypto_aes_decrypt(cIn, inLen,
                                                    cKey, keyLen,
                                                    &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, input, cIn);
        releaseJBytes(env, key, cKey);
        return jba;
    }

    jbyteArray cryptoAesGcmEncrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                   jbyteArray input, jbyteArray key, jbyteArray init_vector, jbyteArray aad, jint tag_bits) {
        auto [cIn, inLen] = readJBytes(env, input);
        auto [cKey, keyLen] = readJBytes(env, key);
        auto [cIv, ivLen] = readJBytes(env, init_vector);
        auto [cAad, aadLen] = readJBytes(env, aad);

        size_t outLen;
        const auto cRes = dynxx_crypto_aes_gcm_encrypt(cIn, inLen,
                                                        cKey, keyLen,
                                                        cIv, ivLen,
                                                        cAad, aadLen,
                                                        tag_bits, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, input, cIn);
        releaseJBytes(env, key, cKey);
        releaseJBytes(env, init_vector, cIv);
        if (cAad) {
            releaseJBytes(env, aad, cAad);
        }
        return jba;
    }

    jbyteArray cryptoAesGcmDecrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                   jbyteArray input, jbyteArray key, jbyteArray init_vector, jbyteArray aad, jint tag_bits) {
        auto [cIn, inLen] = readJBytes(env, input);
        auto [cKey, keyLen] = readJBytes(env, key);
        auto [cIv, ivLen] = readJBytes(env, init_vector);
        auto [cAad, aadLen] = readJBytes(env, aad);

        size_t outLen;
        const auto cRes = dynxx_crypto_aes_gcm_decrypt(cIn, inLen,
                                                        cKey, keyLen,
                                                        cIv, ivLen,
                                                        cAad, aadLen,
                                                        tag_bits, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, input, cIn);
        releaseJBytes(env, key, cKey);
        releaseJBytes(env, init_vector, cIv);
        if (cAad) {
            releaseJBytes(env, aad, cAad);
        }
        return jba;
    }

    jstring cryptoRsaGenKey(JNIEnv *env, [[maybe_unused]] jobject thiz,
                            jstring base64, jboolean is_public) {
        auto cBase64 = readJString(env, base64);

        auto cRes = dynxx_crypto_rsa_gen_key(cBase64, is_public);
        auto jRes = boxJString(env, cRes);

        releaseJString(env, base64, cBase64);
        return jRes;
    }

    jbyteArray cryptoRsaEncrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input, jbyteArray key, jint padding) {
        auto [cIn, inLen] = readJBytes(env, input);
        auto [cKey, keyLen] = readJBytes(env, key);

        size_t outLen;
        const auto cRes = dynxx_crypto_rsa_encrypt(cIn, inLen,
                                                    cKey, keyLen,
                                                    static_cast<DynXXCryptoRSAPadding>(padding), &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, input, cIn);
        releaseJBytes(env, key, cKey);
        return jba;
    }

    jbyteArray cryptoRsaDecrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input, jbyteArray key, jint padding) {
        auto [cIn, inLen] = readJBytes(env, input);
        auto [cKey, keyLen] = readJBytes(env, key);

        size_t outLen;
        const auto cRes = dynxx_crypto_rsa_decrypt(cIn, inLen,
                                                    cKey, keyLen,
                                                    static_cast<DynXXCryptoRSAPadding>(padding), &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, input, cIn);
        releaseJBytes(env, key, cKey);
        return jba;
    }

    jbyteArray cryptoHashMd5(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jbyteArray input) {
        auto [cIn, inLen] = readJBytes(env, input);

        size_t outLen;
        const auto cRes = dynxx_crypto_hash_md5(cIn, inLen, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, input, cIn);
        return jba;
    }

    jbyteArray cryptoHashSha1(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input) {
        auto [cIn, inLen] = readJBytes(env, input);

        size_t outLen;
        const auto cRes = dynxx_crypto_hash_sha1(cIn, inLen, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, input, cIn);
        return jba;
    }

    jbyteArray cryptoHashSha256(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input) {
        auto [cIn, inLen] = readJBytes(env, input);

        size_t outLen;
        const auto cRes = dynxx_crypto_hash_sha256(cIn, inLen, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, input, cIn);
        return jba;
    }

    jbyteArray cryptoBase64Encode(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                  jbyteArray input, jboolean noNewLines) {
        auto [cIn, inLen] = readJBytes(env, input);

        size_t outLen;
        const auto cRes = dynxx_crypto_base64_encode(cIn, inLen, noNewLines, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, input, cIn);
        return jba;
    }

    jbyteArray cryptoBase64Decode(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                  jbyteArray input, jboolean noNewLines) {
        auto [cIn, inLen] = readJBytes(env, input);

        size_t outLen;
        const auto cRes = dynxx_crypto_base64_decode(cIn, inLen, noNewLines, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, input, cIn);
        return jba;
    }

// JsonDecoder

    jint jsonReadType([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                      jlong node) {
        return dynxx_json_node_read_type(node);
    }

    jlong jsonDecoderInit(JNIEnv *env, [[maybe_unused]] jobject thiz,
                          jstring json) {
        auto cJson = readJString(env, json);
        const auto res = dynxx_json_decoder_init(cJson);
        releaseJString(env, json, cJson);
        return res;
    }

    jlong jsonDecoderReadNode(JNIEnv *env, [[maybe_unused]] jobject thiz,
                              jlong decoder, jlong node, jstring k) {
        auto cK = readJString(env, k);
        const auto res = dynxx_json_decoder_read_node(decoder, node, cK);
        releaseJString(env, k, cK);
        return res;
    }

    jstring jsonDecoderReadString(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                  jlong decoder, jlong node) {
        auto cRes = dynxx_json_decoder_read_string(decoder, node);
        auto jStr = boxJString(env, cRes);
        return jStr;
    }

    jlong jsonDecoderReadInteger([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                                  jlong decoder, jlong node) {
        return dynxx_json_decoder_read_integer(decoder, node);
    }

    jdouble jsonDecoderReadFloat([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                                 jlong decoder, jlong node) {
        return dynxx_json_decoder_read_float(decoder, node);
    }

    jlong jsonDecoderReadChild([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                               jlong decoder, jlong node) {
        return dynxx_json_decoder_read_child(decoder, node);
    }

    jlong jsonDecoderReadNext([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                              jlong decoder, jlong node) {
        return dynxx_json_decoder_read_next(decoder, node);
    }

    void jsonDecoderRelease([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                            jlong decoder) {
        dynxx_json_decoder_release(decoder);
    }

// Zip

    jlong zZipInit([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                   jint mode, jlong bufferSize, jint format) {
        return dynxx_z_zip_init(static_cast<DynXXZipCompressMode>(mode), bufferSize, static_cast<DynXXZFormat>(format));
    }

    jlong zZipInput(JNIEnv *env, [[maybe_unused]] jobject thiz,
                    jlong zip, jbyteArray inBytes, jboolean inFinish) {
        auto [cIn, inLen] = readJBytes(env, inBytes);

        auto ret = dynxx_z_zip_input(zip, cIn, inLen, inFinish);

        releaseJBytes(env, inBytes, cIn);

        return static_cast<jlong>(ret);
    }

    jbyteArray zZipProcessDo(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jlong zip) {
        size_t outLen = 0;
        const auto cRes = dynxx_z_zip_process_do(zip, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jboolean zZipProcessFinished([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                                 jlong zip) {
        return dynxx_z_zip_process_finished(zip);
    }

    void zZipRelease([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                     jlong zip) {
        dynxx_z_zip_release(zip);
    }

    jlong zUnZipInit([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                     jlong bufferSize, jint format) {
        return dynxx_z_unzip_init(bufferSize, static_cast<DynXXZFormat>(format));
    }

    jlong zUnZipInput(JNIEnv *env, [[maybe_unused]] jobject thiz,
                      jlong unzip, jbyteArray inBytes, jboolean inFinish) {
        auto [cIn, inLen] = readJBytes(env, inBytes);

        auto ret = dynxx_z_unzip_input(unzip, cIn, inLen, inFinish);

        releaseJBytes(env, inBytes, cIn);

        return static_cast<jlong>(ret);
    }

    jbyteArray zUnZipProcessDo(JNIEnv *env, [[maybe_unused]] jobject thiz,
                               jlong unzip) {
        size_t outLen = 0;
        const auto cRes = dynxx_z_unzip_process_do(unzip, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jboolean zUnZipProcessFinished([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                                   jlong unzip) {
        return dynxx_z_unzip_process_finished(unzip);
    }

    void zUnZipRelease([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                       jlong unzip) {
        dynxx_z_unzip_release(unzip);
    }

    jbyteArray zZipBytes(JNIEnv *env, [[maybe_unused]] jobject thiz,
                         jint mode, jlong buffer_size, jint format, jbyteArray bytes) {
        auto [cIn, inLen] = readJBytes(env, bytes);

        size_t outLen;
        const auto cRes = dynxx_z_bytes_zip(static_cast<DynXXZipCompressMode>(mode), buffer_size, static_cast<DynXXZFormat>(format),
                                             cIn, inLen, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, bytes, cIn);
        return jba;
    }

    jbyteArray zUnZipBytes(JNIEnv *env, [[maybe_unused]] jobject thiz,
                           jlong buffer_size, jint format, jbyteArray bytes) {
        auto [cIn, inLen] = readJBytes(env, bytes);

        size_t outLen;
        const auto cRes = dynxx_z_bytes_unzip(buffer_size, static_cast<DynXXZFormat>(format),
                                               cIn, inLen, &outLen);
        auto jba = moveToJByteArray(env, cRes, outLen);

        releaseJBytes(env, bytes, cIn);
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
            DECLARE_JNI_FUNC(kvAllKeys, "(J)[" LJLS_),
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
            DECLARE_JNI_FUNC(cryptoRsaGenKey, "(" LJLS_ "Z)" LJLS_),
            DECLARE_JNI_FUNC(cryptoRsaEncrypt, "([B[BI)[B"),
            DECLARE_JNI_FUNC(cryptoRsaDecrypt, "([B[BI)[B"),

            DECLARE_JNI_FUNC(cryptoHashMd5, "([B)[B"),
            DECLARE_JNI_FUNC(cryptoHashSha1, "([B)[B"),
            DECLARE_JNI_FUNC(cryptoHashSha256, "([B)[B"),
            DECLARE_JNI_FUNC(cryptoBase64Encode, "([BZ)[B"),
            DECLARE_JNI_FUNC(cryptoBase64Decode, "([BZ)[B"),

            DECLARE_JNI_FUNC(jsonReadType, "(J)I"),
            DECLARE_JNI_FUNC(jsonDecoderInit, "(" LJLS_ ")J"),
            DECLARE_JNI_FUNC(jsonDecoderReadNode, "(JJ" LJLS_ ")J"),
            DECLARE_JNI_FUNC(jsonDecoderReadString, "(JJ)" LJLS_),
            DECLARE_JNI_FUNC(jsonDecoderReadInteger, "(JJ)J"),
            DECLARE_JNI_FUNC(jsonDecoderReadFloat, "(JJ)D"),
            DECLARE_JNI_FUNC(jsonDecoderReadChild, "(JJ)J"),
            DECLARE_JNI_FUNC(jsonDecoderReadNext, "(JJ)J"),
            DECLARE_JNI_FUNC(jsonDecoderRelease, "(J)V"),

            DECLARE_JNI_FUNC(zZipInit, "(IJI)J"),
            DECLARE_JNI_FUNC(zZipInput, "(J[BZ)J"),
            DECLARE_JNI_FUNC(zZipProcessDo, "(J)[B"),
            DECLARE_JNI_FUNC(zZipProcessFinished, "(J)Z"),
            DECLARE_JNI_FUNC(zZipRelease, "(J)V"),
            DECLARE_JNI_FUNC(zUnZipInit, "(JI)J"),
            DECLARE_JNI_FUNC(zUnZipInput, "(J[BZ)J"),
            DECLARE_JNI_FUNC(zUnZipProcessDo, "(J)[B"),
            DECLARE_JNI_FUNC(zUnZipProcessFinished, "(J)Z"),
            DECLARE_JNI_FUNC(zUnZipRelease, "(J)V"),
            DECLARE_JNI_FUNC(zZipBytes, "(IJI[B)[B"),
            DECLARE_JNI_FUNC(zUnZipBytes, "(JI[B)[B"),
    };

}

int JNI_OnLoad(JavaVM *vm, [[maybe_unused]] void *reserved) {
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

void JNI_OnUnload(JavaVM *vm, [[maybe_unused]] void *reserved) {
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
