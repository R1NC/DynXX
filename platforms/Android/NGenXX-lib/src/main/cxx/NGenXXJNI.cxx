#include <jni.h>

#include <string>
#include <cstring>

#include "JNIUtil.hxx"
#include "../../../../../../build.Android/output/include/NGenXX.h"

static JavaVM *sVM;
static jclass sJClass;
static jobject sLogCallback;
static jmethodID sLogCallbackMethodId;
static jobject sJsMsgCallback;
static jmethodID sJsMsgCallbackMethodId;

#pragma mark Engine Callback

static void JNI_NGenXX_log_callback(int level, const char *content)
{
    if (sVM == nullptr || sLogCallback == nullptr || sLogCallbackMethodId == nullptr || content == nullptr) {
        return;
    }
    auto env = currentEnv(sVM);
    auto jContent = env->NewStringUTF(content);
    freeX(content);
    env->CallVoidMethod(sLogCallback, sLogCallbackMethodId, level, jContent);
}

static const char *JNI_NGenXX_js_msg_callback(const char *msg)
{
    if (sVM == nullptr || sJsMsgCallback == nullptr || sJsMsgCallbackMethodId == nullptr || msg == nullptr) {
        return nullptr;
    }
    auto env = currentEnv(sVM);
    auto jMsg = env->NewStringUTF(msg);
    freeX(msg);
    auto jRes = env->CallObjectMethod(sJsMsgCallback, sJsMsgCallbackMethodId, jMsg);
    return env->GetStringUTFChars(reinterpret_cast<jstring>(jRes), nullptr);
}

#pragma mark Engine base API

jstring JNI_NGenXX_getVersion(JNIEnv *env, jobject thiz)
{
    auto cV = ngenxx_get_version();
    auto jStr = env->NewStringUTF(cV ?: "");
    freeX(cV);
    return jStr;
}

jboolean JNI_NGenXX_init(JNIEnv *env, jobject thiz,
                         jstring root)
{
    auto cRoot = env->GetStringUTFChars(root, nullptr);
    auto res = ngenxx_init(cRoot);
    env->ReleaseStringUTFChars(root, cRoot);
    return res;
}

void JNI_NGenXX_release(JNIEnv *env, jobject thiz)
{
    ngenxx_release();
}

#pragma mark Log

void JNI_NGenXX_logSetLevel(JNIEnv *env, jobject thiz,
                            jint level)
{
    ngenxx_log_set_level(level);
}

void JNI_NGenXX_logSetCallback(JNIEnv *env, jobject thiz,
                               jobject callback)
{
    if (callback)
    {
        ngenxx_log_set_callback(JNI_NGenXX_log_callback);
        sLogCallback = env->NewWeakGlobalRef(callback);
        auto jCallbackClass = env->GetObjectClass(callback);
        if (jCallbackClass)
        {
            sLogCallbackMethodId = env->GetMethodID(jCallbackClass, "invoke", "(ILjava/lang/String;)V");
        }
    }
}

void JNI_NGenXX_logPrint(JNIEnv *env, jobject thiz,
                         jint level, jstring content)
{
    auto cContent = env->GetStringUTFChars(content, nullptr);
    ngenxx_log_print(level, cContent);
    env->ReleaseStringUTFChars(content, cContent);
}

#pragma mark Net

jstring JNI_NGenXX_netHttpRequest(JNIEnv *env, jobject thiz,
                                  jstring url, jstring params,
                                  jint method,
                                  jobjectArray headerV,
                                  jobjectArray formFieldNameV,
                                  jobjectArray formFieldMimeV,
                                  jobjectArray formFieldDataV,
                                  jstring filePath, jlong fileLength,
                                  jlong timeout)
{
    auto cUrl = env->GetStringUTFChars(url, nullptr);
    auto cParams = env->GetStringUTFChars(params, nullptr);

    auto headerCount = env->GetArrayLength(headerV);

    auto cHeaderV = mallocX<char *>(headerCount);
    auto jStrHeaderV = mallocX<jstring>(headerCount);

    for (int i = 0; i < headerCount; i++)
    {
        jStrHeaderV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(headerV, i));
        cHeaderV[i] = const_cast<char *>(env->GetStringUTFChars(jStrHeaderV[i], nullptr));
    }

    auto formFieldCount = env->GetArrayLength(formFieldNameV);

    auto cFormFieldNameV = mallocX<char *>(formFieldCount);
    auto jStrFormFieldNameV = mallocX<jstring>(formFieldCount);

    for (int i = 0; i < formFieldCount; i++)
    {
        jStrFormFieldNameV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(formFieldNameV, i));
        cFormFieldNameV[i] = const_cast<char *>(env->GetStringUTFChars(jStrFormFieldNameV[i], nullptr));
    }

    auto cFormFieldMimeV = mallocX<char *>(formFieldCount);
    auto jStrFormFieldMimeV = mallocX<jstring>(formFieldCount);

    for (int i = 0; i < formFieldCount; i++)
    {
        jStrFormFieldMimeV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(formFieldMimeV, i));
        cFormFieldMimeV[i] = const_cast<char *>(env->GetStringUTFChars(jStrFormFieldMimeV[i], nullptr));
    }

    auto cFormFieldDataV = mallocX<char *>(formFieldCount);
    auto jStrFormFieldDataV = mallocX<jstring>(formFieldCount);

    for (int i = 0; i < formFieldCount; i++)
    {
        jStrFormFieldDataV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(formFieldDataV, i));
        cFormFieldDataV[i] = const_cast<char *>(env->GetStringUTFChars(jStrFormFieldDataV[i], nullptr));
    }

    auto cFilePath = filePath ? env->GetStringUTFChars(filePath, nullptr) : "";
    auto cFILE = cFilePath ? std::fopen(cFilePath, "r") : nullptr;

    auto cRsp = ngenxx_net_http_request(cUrl, cParams, (const int)method,
                                               const_cast<const char **>(cHeaderV), headerCount,
                                               const_cast<const char **>(cFormFieldNameV),
                                               const_cast<const char **>(cFormFieldMimeV),
                                               const_cast<const char **>(cFormFieldDataV),
                                               formFieldCount,
                                               static_cast<void *>(cFILE), fileLength,
                                               static_cast<const size_t>(timeout));
    auto jStr = env->NewStringUTF(cRsp ?: "");
    freeX(cRsp);

    for (int i = 0; i < headerCount; i++)
    {
        env->ReleaseStringUTFChars(jStrHeaderV[i], cHeaderV[i]);
        freeX(jStrHeaderV[i]);
    }
    for (int i = 0; i < formFieldCount; i++)
    {
        env->ReleaseStringUTFChars(jStrFormFieldNameV[i], cFormFieldNameV[i]);
        freeX(jStrFormFieldNameV[i]);
        env->ReleaseStringUTFChars(jStrFormFieldMimeV[i], cFormFieldMimeV[i]);
        freeX(jStrFormFieldMimeV[i]);
        env->ReleaseStringUTFChars(jStrFormFieldDataV[i], cFormFieldDataV[i]);
        freeX(jStrFormFieldDataV[i]);
    }

    if (filePath)
    {
        freeX(cFilePath);
    }
    if (cFILE)
    {
        std::fclose(cFILE);
    }
    if (cParams)
    {
        env->ReleaseStringUTFChars(params, cParams);
    }
    env->ReleaseStringUTFChars(url, cUrl);

    return jStr;
}

#pragma mark Lua

jboolean JNI_NGenXX_lLoadF(JNIEnv *env, jobject thiz,
                           jstring file)
{
    auto cFile = env->GetStringUTFChars(file, nullptr);
    auto res = ngenxx_lua_loadF(cFile);
    env->ReleaseStringUTFChars(file, cFile);
    return res;
}

jboolean JNI_NGenXX_lLoadS(JNIEnv *env, jobject thiz,
                           jstring script)
{
    auto cScript = env->GetStringUTFChars(script, nullptr);
    auto res = ngenxx_lua_loadS(cScript);
    env->ReleaseStringUTFChars(script, cScript);
    return res;
}

jstring JNI_NGenXX_lCall(JNIEnv *env, jobject thiz,
                         jstring func, jstring params)
{
    auto cFunc = env->GetStringUTFChars(func, nullptr);
    auto cParams = params ? env->GetStringUTFChars(params, nullptr) : nullptr;
    auto cRes = ngenxx_lua_call(cFunc, cParams);
    auto jStr = env->NewStringUTF(cRes ?: "");
    freeX(cRes);
    if (cParams) {
        env->ReleaseStringUTFChars(params, cParams);
    }
    env->ReleaseStringUTFChars(func, cFunc);
    return jStr;
}

#pragma mark JS

jboolean JNI_NGenXX_jLoadF(JNIEnv *env, jobject thiz,
                           jstring file, jboolean is_module)
{
    auto cFile = env->GetStringUTFChars(file, nullptr);
    auto res = ngenxx_js_loadF(cFile, is_module);
    env->ReleaseStringUTFChars(file, cFile);
    return res;
}

jboolean JNI_NGenXX_jLoadS(JNIEnv *env, jobject thiz,
                           jstring script, jstring name, jboolean is_module)
{
    auto cScript = env->GetStringUTFChars(script, nullptr);
    auto cName = env->GetStringUTFChars(name, nullptr);
    auto res = ngenxx_js_loadS(cScript, cName, is_module);
    env->ReleaseStringUTFChars(script, cScript);
    env->ReleaseStringUTFChars(name, cName);
    return res;
}

jboolean JNI_NGenXX_jLoadB(JNIEnv *env, jobject thiz,
                           jbyteArray bytes, jboolean is_module)
{
    auto cIn = env->GetByteArrayElements(bytes, nullptr);
    auto inLen = env->GetArrayLength(bytes);

    auto res = ngenxx_js_loadB(reinterpret_cast<const byte *>(cIn), inLen, is_module);

    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return res;
}

jstring JNI_NGenXX_jCall(JNIEnv *env, jobject thiz,
                         jstring func, jstring params, jboolean await)
{
    auto cFunc = env->GetStringUTFChars(func, nullptr);
    auto cParams = params ? env->GetStringUTFChars(params, nullptr) : nullptr;
    auto cRes = ngenxx_js_call(cFunc, cParams, await);
    auto jStr = env->NewStringUTF(cRes ?: "");
    freeX(cRes);
    if (cParams) {
        env->ReleaseStringUTFChars(params, cParams);
    }
    env->ReleaseStringUTFChars(func, cFunc);
    return jStr;
}

void JNI_NGenXX_jSetMsgCallback(JNIEnv *env, jobject thiz,
                                jobject callback)
{
    if (callback)
    {
        ngenxx_js_set_msg_callback(JNI_NGenXX_js_msg_callback);
        sJsMsgCallback = env->NewWeakGlobalRef(callback);
        if (auto jCallbackClass = env->GetObjectClass(callback))
        {
            sJsMsgCallbackMethodId = env->GetMethodID(jCallbackClass, "invoke", "(Ljava/lang/String;)Ljava/lang/String;");
        }
    }
}

#pragma mark Store.SQLite

jlong JNI_NGenXX_storeSQLiteOpen(JNIEnv *env, jobject thiz,
                                 jstring id)
{
    auto cId = env->GetStringUTFChars(id, nullptr);
    const auto res = ngenxx_store_sqlite_open(cId);
    env->ReleaseStringUTFChars(id, cId);
    return ptr2addr(res);
}

jboolean JNI_NGenXX_storeSQLiteExecute(JNIEnv *env, jobject thiz,
                                       jlong conn,
                                       jstring sql)
{
    auto cSql = env->GetStringUTFChars(sql, nullptr);
    auto res = ngenxx_store_sqlite_execute(addr2ptr(conn), cSql);
    env->ReleaseStringUTFChars(sql, cSql);
    return res;
}

jlong JNI_NGenXX_storeSQLiteQueryDo(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring sql)
{
    auto cSql = env->GetStringUTFChars(sql, nullptr);
    const auto res = ngenxx_store_sqlite_query_do(addr2ptr(conn), cSql);
    env->ReleaseStringUTFChars(sql, cSql);
    return ptr2addr(res);
}

jboolean JNI_NGenXX_storeSQLiteQueryReadRow(JNIEnv *env, jobject thiz,
                                            jlong query_result)
{
    return ngenxx_store_sqlite_query_read_row(addr2ptr(query_result));
}

jstring JNI_NGenXX_storeSQLiteQueryReadColumnText(JNIEnv *env, jobject thiz,
                                                  jlong query_result,
                                                  jstring column)
{
    auto cColumn = env->GetStringUTFChars(column, nullptr);
    auto cRes = ngenxx_store_sqlite_query_read_column_text(addr2ptr(query_result), cColumn);
    auto jStr = env->NewStringUTF(cRes ?: "");
    freeX(cRes);
    env->ReleaseStringUTFChars(column, cColumn);
    return jStr;
}

jlong JNI_NGenXX_storeSQLiteQueryReadColumnInteger(JNIEnv *env, jobject thiz,
                                                   jlong query_result,
                                                   jstring column)
{
    auto cColumn = env->GetStringUTFChars(column, nullptr);
    auto res = ngenxx_store_sqlite_query_read_column_integer(addr2ptr(query_result), cColumn);
    env->ReleaseStringUTFChars(column, cColumn);
    return res;
}

jdouble JNI_NGenXX_storeSQLiteQueryReadColumnFloat(JNIEnv *env, jobject thiz,
                                                   jlong query_result,
                                                   jstring column)
{
    auto cColumn = env->GetStringUTFChars(column, nullptr);
    auto res = ngenxx_store_sqlite_query_read_column_float(addr2ptr(query_result), cColumn);
    env->ReleaseStringUTFChars(column, cColumn);
    return res;
}

void JNI_NGenXX_storeSQLiteQueryDrop(JNIEnv *env, jobject thiz,
                                     jlong query_result)
{
    ngenxx_store_sqlite_query_drop(addr2ptr(query_result));
}

void JNI_NGenXX_storeSQLiteClose(JNIEnv *env, jobject thiz,
                                 jlong conn)
{
    ngenxx_store_sqlite_close(addr2ptr(conn));
}

#pragma mark Srore.KV

jlong JNI_NGenXX_storeKVOpen(JNIEnv *env, jobject thiz,
                             jstring id)
{
    auto cId = env->GetStringUTFChars(id, nullptr);
    const auto res = ngenxx_store_kv_open(cId);
    env->ReleaseStringUTFChars(id, cId);
    return ptr2addr(res);
}

jstring JNI_NGenXX_storeKVReadString(JNIEnv *env, jobject thiz,
                                     jlong conn,
                                     jstring k)
{
    auto cK = env->GetStringUTFChars(k, nullptr);
    auto cRes = ngenxx_store_kv_read_string(addr2ptr(conn), cK);
    auto jStr = env->NewStringUTF(cRes ?: "");
    freeX(cRes);
    env->ReleaseStringUTFChars(k, cK);
    return jStr;
}

jboolean JNI_NGenXX_storeKVWriteString(JNIEnv *env, jobject thiz,
                                       jlong conn,
                                       jstring k, jstring v)
{
    auto cK = env->GetStringUTFChars(k, nullptr);
    auto cV = env->GetStringUTFChars(v, nullptr);
    auto res = ngenxx_store_kv_write_string(addr2ptr(conn), cK, cV);
    env->ReleaseStringUTFChars(v, cV);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jlong JNI_NGenXX_storeKVReadInteger(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring k)
{
    auto cK = env->GetStringUTFChars(k, nullptr);
    auto res = ngenxx_store_kv_read_integer(addr2ptr(conn), cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jboolean JNI_NGenXX_storeKVWriteInteger(JNIEnv *env, jobject thiz,
                                        jlong conn,
                                        jstring k, jlong v)
{
    auto cK = env->GetStringUTFChars(k, nullptr);
    auto res = ngenxx_store_kv_write_integer(addr2ptr(conn), cK, v);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jdouble JNI_NGenXX_storeKVReadFloat(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring k)
{
    auto cK = env->GetStringUTFChars(k, nullptr);
    auto res = ngenxx_store_kv_read_float(addr2ptr(conn), cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jboolean JNI_NGenXX_storeKVWriteFloat(JNIEnv *env, jobject thiz,
                                      jlong conn,
                                      jstring k, jdouble v)
{
    auto cK = env->GetStringUTFChars(k, nullptr);
    auto res = ngenxx_store_kv_write_float(addr2ptr(conn), cK, v);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jboolean JNI_NGenXX_storeKVContains(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring k)
{
    auto cK = env->GetStringUTFChars(k, nullptr);
    auto res = ngenxx_store_kv_contains(addr2ptr(conn), cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jboolean JNI_NGenXX_storeKVRemove(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring k)
{
    auto cK = env->GetStringUTFChars(k, nullptr);
    auto res = ngenxx_store_kv_remove(addr2ptr(conn), cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

void JNI_NGenXX_storeKVClear(JNIEnv *env, jobject thiz,
                             jlong conn)
{
    ngenxx_store_kv_clear(addr2ptr(conn));
}

void JNI_NGenXX_storeKVClose(JNIEnv *env, jobject thiz,
                             jlong conn)
{
    ngenxx_store_kv_close(addr2ptr(conn));
}

#pragma mark Device

jint JNI_NGenXX_deviceType(JNIEnv *env, jobject thiz)
{
    return ngenxx_device_type();
}

jstring JNI_NGenXX_deviceName(JNIEnv *env, jobject thiz)
{
    auto cDN = ngenxx_device_name();
    auto jStr = env->NewStringUTF(cDN ?: "");
    freeX(cDN);
    return jStr;
}

jstring JNI_NGenXX_deviceManufacturer(JNIEnv *env, jobject thiz)
{
    auto cDM = ngenxx_device_manufacturer();
    auto jStr = env->NewStringUTF(cDM ?: "");
    freeX(cDM);
    return jStr;
}

jstring JNI_NGenXX_deviceOsVersion(JNIEnv *env, jobject thiz)
{
    auto cDOV = ngenxx_device_os_version();
    auto jStr = env->NewStringUTF(cDOV ?: "");
    freeX(cDOV);
    return jStr;
}

jint JNI_NGenXX_deviceCpuArch(JNIEnv *env, jobject thiz)
{
    return ngenxx_device_cpu_arch();
}

#pragma mark Coding

jstring JNI_NGenXX_codingHexBytes2Str(JNIEnv *env, jobject thiz,
                                      jbyteArray bytes)
{
    auto cIn = env->GetByteArrayElements(bytes, nullptr);
    auto inLen = env->GetArrayLength(bytes);

    auto cRes = ngenxx_coding_hex_bytes2str(reinterpret_cast<const byte *>(cIn), inLen);
    auto jStr = env->NewStringUTF(cRes ?: "");

    freeX(cRes);
    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return jStr;
}

jbyteArray JNI_NGenXX_codingHexStr2Bytes(JNIEnv *env,
                                         jobject thiz,
                                         jstring str)
{
    auto cStr = env->GetStringUTFChars(str, nullptr);

    size_t outLen;
    const auto cRes = ngenxx_coding_hex_str2bytes(cStr, &outLen);
    auto jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseStringUTFChars(str, cStr);
    return jba;
}

#pragma mark Crypto

jbyteArray JNI_NGenXX_cryptoRandom(JNIEnv *env, jobject thiz,
                                   jint len)
{
    byte out[len];
    std::memset(out, 0, len);
    ngenxx_crypto_rand(len, out);
    return moveToJByteArray(env, out, len, false);
}

jbyteArray JNI_NGenXX_cryptoAesEncrypt(JNIEnv *env, jobject thiz,
                                       jbyteArray input,
                                       jbyteArray key)
{
    auto cIn = env->GetByteArrayElements(input, nullptr);
    auto inLen = env->GetArrayLength(input);
    auto cKey = env->GetByteArrayElements(key, nullptr);
    auto keyLen = env->GetArrayLength(key);

    size_t outLen;
    const auto cRes = ngenxx_crypto_aes_encrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                          reinterpret_cast<const byte *>(cKey), keyLen,
                                          &outLen);
    auto jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
    return jba;
}

jbyteArray JNI_NGenXX_cryptoAesDecrypt(JNIEnv *env, jobject thiz,
                                       jbyteArray input,
                                       jbyteArray key)
{
    auto cIn = env->GetByteArrayElements(input, nullptr);
    auto inLen = env->GetArrayLength(input);
    auto cKey = env->GetByteArrayElements(key, nullptr);
    auto keyLen = env->GetArrayLength(key);

    size_t outLen;
    const auto cRes = ngenxx_crypto_aes_decrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                          reinterpret_cast<const byte *>(cKey), keyLen,
                                          &outLen);
    auto jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
    return jba;
}

jbyteArray JNI_NGenXX_cryptoAesGcmEncrypt(JNIEnv *env, jobject thiz,
                                          jbyteArray input,
                                          jbyteArray key,
                                          jbyteArray init_vector,
                                          jbyteArray aad,
                                          jint tag_bits)
{
    auto cIn = env->GetByteArrayElements(input, nullptr);
    auto inLen = env->GetArrayLength(input);
    auto cKey = env->GetByteArrayElements(key, nullptr);
    auto keyLen = env->GetArrayLength(key);
    auto cIv = env->GetByteArrayElements(init_vector, nullptr);
    auto ivLen = env->GetArrayLength(init_vector);
    auto cAad = aad ? env->GetByteArrayElements(aad, nullptr) : nullptr;
    auto aadLen = aad ? env->GetArrayLength(aad) : 0;

    size_t outLen;
    const auto cRes = ngenxx_crypto_aes_gcm_encrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                              reinterpret_cast<const byte *>(cKey), keyLen,
                                              reinterpret_cast<const byte *>(cIv), ivLen,
                                              reinterpret_cast<const byte *>(cAad), aadLen,
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

jbyteArray JNI_NGenXX_cryptoAesGcmDecrypt(JNIEnv *env, jobject thiz,
                                          jbyteArray input,
                                          jbyteArray key,
                                          jbyteArray init_vector,
                                          jbyteArray aad,
                                          jint tag_bits)
{
    auto cIn = env->GetByteArrayElements(input, nullptr);
    auto inLen = env->GetArrayLength(input);
    auto cKey = env->GetByteArrayElements(key, nullptr);
    auto keyLen = env->GetArrayLength(key);
    auto cIv = env->GetByteArrayElements(init_vector, nullptr);
    auto ivLen = env->GetArrayLength(init_vector);
    auto cAad = aad ? env->GetByteArrayElements(aad, nullptr) : nullptr;
    auto aadLen = aad ? env->GetArrayLength(aad) : 0;

    size_t outLen;
    const auto cRes = ngenxx_crypto_aes_gcm_decrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                              reinterpret_cast<const byte *>(cKey), keyLen,
                                              reinterpret_cast<const byte *>(cIv), ivLen,
                                              reinterpret_cast<const byte *>(cAad), aadLen,
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

jbyteArray JNI_NGenXX_cryptoHashMd5(JNIEnv *env, jobject thiz,
                                    jbyteArray input)
{
    auto cIn = env->GetByteArrayElements(input, nullptr);
    auto inLen = env->GetArrayLength(input);

    size_t outLen;
    const auto cRes = ngenxx_crypto_hash_md5(reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    auto jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

jbyteArray JNI_NGenXX_cryptoHashSha256(JNIEnv *env, jobject thiz,
                                       jbyteArray input)
{
    auto cIn = env->GetByteArrayElements(input, nullptr);
    auto inLen = env->GetArrayLength(input);

    size_t outLen;
    const auto cRes = ngenxx_crypto_hash_sha256(reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    auto jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

jbyteArray JNI_NGenXX_cryptoBase64Encode(JNIEnv *env, jobject thiz,
                                         jbyteArray input)
{
    auto cIn = env->GetByteArrayElements(input, nullptr);
    auto inLen = env->GetArrayLength(input);

    size_t outLen;
    const auto cRes = ngenxx_crypto_base64_encode(reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    auto jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

jbyteArray JNI_NGenXX_cryptoBase64Decode(JNIEnv *env, jobject thiz,
                                         jbyteArray input)
{
    auto cIn = env->GetByteArrayElements(input, nullptr);
    auto inLen = env->GetArrayLength(input);

    size_t outLen;
    const auto cRes = ngenxx_crypto_base64_decode(reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    auto jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

#pragma mark JsonDecoder

jlong JNI_NGenXX_jsonDecoderInit(JNIEnv *env, jobject thiz,
                                 jstring json)
{
    auto cJson = env->GetStringUTFChars(json, nullptr);
    const auto res = ngenxx_json_decoder_init(cJson);
    env->ReleaseStringUTFChars(json, cJson);
    return ptr2addr(res);
}

jboolean JNI_NGenXX_jsonDecoderIsArray(JNIEnv *env, jobject thiz,
                                       jlong decoder,
                                       jlong node)
{
    return ngenxx_json_decoder_is_array(addr2ptr(decoder), addr2ptr(node));
}

jboolean JNI_NGenXX_jsonDecoderIsObject(JNIEnv *env, jobject thiz,
                                        jlong decoder,
                                        jlong node)
{
    return ngenxx_json_decoder_is_object(addr2ptr(decoder), addr2ptr(node));
}

jlong JNI_NGenXX_jsonDecoderReadNode(JNIEnv *env, jobject thiz,
                                     jlong decoder,
                                     jlong node,
                                     jstring k)
{
    auto cK = env->GetStringUTFChars(k, nullptr);
    const auto res = ngenxx_json_decoder_read_node(addr2ptr(decoder), addr2ptr(node), cK);
    env->ReleaseStringUTFChars(k, cK);
    return ptr2addr(res);
}

jstring JNI_NGenXX_jsonDecoderReadString(JNIEnv *env, jobject thiz,
                                         jlong decoder,
                                         jlong node)
{
    auto res = ngenxx_json_decoder_read_string(addr2ptr(decoder), addr2ptr(node));
    auto jStr = env->NewStringUTF(res ?: "");
    freeX(res);
    return jStr;
}

jdouble JNI_NGenXX_jsonDecoderReadNumber(JNIEnv *env, jobject thiz,
                                         jlong decoder,
                                         jlong node)
{
    return ngenxx_json_decoder_read_number(addr2ptr(decoder), addr2ptr(node));
}

jlong JNI_NGenXX_jsonDecoderReadChild(JNIEnv *env, jobject thiz,
                                      jlong decoder,
                                      jlong node)
{
    return ptr2addr(ngenxx_json_decoder_read_child(addr2ptr(decoder), addr2ptr(node)));
}

jlong JNI_NGenXX_jsonDecoderReadNext(JNIEnv *env, jobject thiz,
                                     jlong decoder,
                                     jlong node)
{
    return ptr2addr(ngenxx_json_decoder_read_next(addr2ptr(decoder), addr2ptr(node)));
}

void JNI_NGenXX_jsonDecoderRelease(JNIEnv *env, jobject thiz,
                                   jlong decoder)
{
    ngenxx_json_decoder_release(addr2ptr(decoder));
}

#pragma mark Zip

jlong JNI_NGenXX_zZipInit(JNIEnv *env, jobject thiz,
                          jint mode, jlong bufferSize, jint format)
{
    return ptr2addr(ngenxx_z_zip_init(mode, bufferSize, format));
}

jlong JNI_NGenXX_zZipInput(JNIEnv *env, jobject thiz,
                           jlong zip,
                           jbyteArray inBytes, jint inLen, jboolean inFinish)
{
    auto cIn = env->GetByteArrayElements(inBytes, nullptr);
    // auto inLen = env->GetArrayLength(input);

    auto ret = ngenxx_z_zip_input(addr2ptr(zip), reinterpret_cast<const byte *>(cIn), inLen, inFinish);

    env->ReleaseByteArrayElements(inBytes, cIn, JNI_ABORT);

    return ret;
}

jbyteArray JNI_NGenXX_zZipProcessDo(JNIEnv *env, jobject thiz, jlong zip)
{
    size_t outLen = 0;
    const auto cRes = ngenxx_z_zip_process_do(addr2ptr(zip), &outLen);
    auto jba = moveToJByteArray(env, cRes, outLen, true);

    return jba;
}

jboolean JNI_NGenXX_zZipProcessFinished(JNIEnv *env, jobject thiz,
                                        jlong zip)
{
    return ngenxx_z_zip_process_finished(addr2ptr(zip));
}

void JNI_NGenXX_zZipRelease(JNIEnv *env, jobject thiz,
                            jlong zip)
{
    ngenxx_z_zip_release(addr2ptr(zip));
}

jlong JNI_NGenXX_zUnZipInit(JNIEnv *env, jobject thiz,
                            jlong bufferSize, jint format)
{
    return ptr2addr(ngenxx_z_unzip_init(bufferSize, format));
}

jlong JNI_NGenXX_zUnZipInput(JNIEnv *env, jobject thiz,
                             jlong unzip,
                             jbyteArray inBytes, jint inLen, jboolean inFinish)
{
    auto cIn = env->GetByteArrayElements(inBytes, nullptr);
    // auto inLen = env->GetArrayLength(input);

    auto ret = ngenxx_z_unzip_input(addr2ptr(unzip), reinterpret_cast<const byte *>(cIn), inLen, inFinish);

    env->ReleaseByteArrayElements(inBytes, cIn, JNI_ABORT);

    return ret;
}

jbyteArray JNI_NGenXX_zUnZipProcessDo(JNIEnv *env, jobject thiz, jlong unzip) {
    size_t outLen = 0;
    const auto cRes = ngenxx_z_unzip_process_do(addr2ptr(unzip), &outLen);
    auto jba = moveToJByteArray(env, cRes, outLen, true);

    return jba;
}

jboolean JNI_NGenXX_zUnZipProcessFinished(JNIEnv *env, jobject thiz,
                                          jlong unzip)
{
    return ngenxx_z_unzip_process_finished(addr2ptr(unzip));
}

void JNI_NGenXX_zUnZipRelease(JNIEnv *env, jobject thiz,
                              jlong unzip)
{
    ngenxx_z_unzip_release(addr2ptr(unzip));
}

jbyteArray JNI_NGenXX_zZipBytes(JNIEnv *env, jobject thiz,
                                jint mode,
                                jlong buffer_size, jint format, jbyteArray bytes)
{
    auto cIn = env->GetByteArrayElements(bytes, nullptr);
    auto inLen = env->GetArrayLength(bytes);

    size_t outLen;
    const auto cRes = ngenxx_z_bytes_zip(mode, buffer_size, format, reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    auto jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return jba;
}

jbyteArray JNI_NGenXX_zUnZipBytes(JNIEnv *env, jobject thiz,
                                  jlong buffer_size, jint format, jbyteArray bytes)
{
    auto cIn = env->GetByteArrayElements(bytes, nullptr);
    auto inLen = env->GetArrayLength(bytes);

    size_t outLen;
    const auto cRes = ngenxx_z_bytes_unzip(buffer_size, format, reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    auto jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return jba;
}

static auto JClassName = "xyz/rinc/ngenxx/NGenXX$Companion";

#define DECLARE_JNI_FUNC(name, signature, funcPtr) {name, signature, reinterpret_cast<void *>(funcPtr)}

static const JNINativeMethod JCFuncList[] = {
    DECLARE_JNI_FUNC("getVersion", "()Ljava/lang/String;", JNI_NGenXX_getVersion),
    DECLARE_JNI_FUNC("init", "(Ljava/lang/String;)Z", JNI_NGenXX_init),
    DECLARE_JNI_FUNC("release", "()V", JNI_NGenXX_release),

    DECLARE_JNI_FUNC("logSetLevel", "(I)V", JNI_NGenXX_logSetLevel),
    DECLARE_JNI_FUNC("logSetCallback", "(Lkotlin/jvm/functions/Function2;)V", JNI_NGenXX_logSetCallback),
    DECLARE_JNI_FUNC("logPrint", "(ILjava/lang/String;)V", JNI_NGenXX_logPrint),

    DECLARE_JNI_FUNC("netHttpRequest", "(Ljava/lang/String;Ljava/lang/String;I[Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;JJ)Ljava/lang/String;", JNI_NGenXX_netHttpRequest),

    DECLARE_JNI_FUNC("lLoadF", "(Ljava/lang/String;)Z", JNI_NGenXX_lLoadF),
    DECLARE_JNI_FUNC("lLoadS", "(Ljava/lang/String;)Z", JNI_NGenXX_lLoadS),
    DECLARE_JNI_FUNC("lCall", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", JNI_NGenXX_lCall),

    DECLARE_JNI_FUNC("jLoadF", "(Ljava/lang/String;Z)Z", JNI_NGenXX_jLoadF),
    DECLARE_JNI_FUNC("jLoadS", "(Ljava/lang/String;Ljava/lang/String;Z)Z", JNI_NGenXX_jLoadS),
    DECLARE_JNI_FUNC("jLoadB", "([BZ)Z", JNI_NGenXX_jLoadB),
    DECLARE_JNI_FUNC("jCall", "(Ljava/lang/String;Ljava/lang/String;Z)Ljava/lang/String;", JNI_NGenXX_jCall),
    DECLARE_JNI_FUNC("jSetMsgCallback", "(Lkotlin/jvm/functions/Function1;)V", JNI_NGenXX_jSetMsgCallback),

    DECLARE_JNI_FUNC("storeSQLiteOpen", "(Ljava/lang/String;)J", JNI_NGenXX_storeSQLiteOpen),
    DECLARE_JNI_FUNC("storeSQLiteExecute", "(JLjava/lang/String;)Z", JNI_NGenXX_storeSQLiteExecute),
    DECLARE_JNI_FUNC("storeSQLiteQueryDo", "(JLjava/lang/String;)J", JNI_NGenXX_storeSQLiteQueryDo),
    DECLARE_JNI_FUNC("storeSQLiteQueryReadRow", "(J)Z", JNI_NGenXX_storeSQLiteQueryReadRow),
    DECLARE_JNI_FUNC("storeSQLiteQueryReadColumnText", "(JLjava/lang/String;)Ljava/lang/String;", JNI_NGenXX_storeSQLiteQueryReadColumnText),
    DECLARE_JNI_FUNC("storeSQLiteQueryReadColumnInteger", "(JLjava/lang/String;)J", JNI_NGenXX_storeSQLiteQueryReadColumnInteger),
    DECLARE_JNI_FUNC("storeSQLiteQueryReadColumnFloat", "(JLjava/lang/String;)D", JNI_NGenXX_storeSQLiteQueryReadColumnFloat),
    DECLARE_JNI_FUNC("storeSQLiteQueryDrop", "(J)V", JNI_NGenXX_storeSQLiteQueryDrop),
    DECLARE_JNI_FUNC("storeSQLiteClose", "(J)V", JNI_NGenXX_storeSQLiteClose),

    DECLARE_JNI_FUNC("storeKVOpen", "(Ljava/lang/String;)J", JNI_NGenXX_storeKVOpen),
    DECLARE_JNI_FUNC("storeKVReadString", "(JLjava/lang/String;)Ljava/lang/String;", JNI_NGenXX_storeKVReadString),
    DECLARE_JNI_FUNC("storeKVWriteString", "(JLjava/lang/String;Ljava/lang/String;)Z", JNI_NGenXX_storeKVWriteString),
    DECLARE_JNI_FUNC("storeKVReadInteger", "(JLjava/lang/String;)J", JNI_NGenXX_storeKVReadInteger),
    DECLARE_JNI_FUNC("storeKVWriteInteger", "(JLjava/lang/String;J)Z", JNI_NGenXX_storeKVWriteInteger),
    DECLARE_JNI_FUNC("storeKVReadFloat", "(JLjava/lang/String;)D", JNI_NGenXX_storeKVReadFloat),
    DECLARE_JNI_FUNC("storeKVWriteFloat", "(JLjava/lang/String;D)Z", JNI_NGenXX_storeKVWriteFloat),
    DECLARE_JNI_FUNC("storeKVContains", "(JLjava/lang/String;)Z", JNI_NGenXX_storeKVContains),
    DECLARE_JNI_FUNC("storeKVRemove", "(JLjava/lang/String;)Z", JNI_NGenXX_storeKVRemove),
    DECLARE_JNI_FUNC("storeKVClear", "(J)V", JNI_NGenXX_storeKVClear),
    DECLARE_JNI_FUNC("storeKVClose", "(J)V", JNI_NGenXX_storeKVClose),

    DECLARE_JNI_FUNC("deviceType", "()I", JNI_NGenXX_deviceType),
    DECLARE_JNI_FUNC("deviceName", "()Ljava/lang/String;", JNI_NGenXX_deviceName),
    DECLARE_JNI_FUNC("deviceManufacturer", "()Ljava/lang/String;", JNI_NGenXX_deviceManufacturer),
    DECLARE_JNI_FUNC("deviceOsVersion", "()Ljava/lang/String;", JNI_NGenXX_deviceOsVersion),
    DECLARE_JNI_FUNC("deviceCpuArch", "()I", JNI_NGenXX_deviceCpuArch),

    DECLARE_JNI_FUNC("codingHexBytes2Str", "([B)Ljava/lang/String;", JNI_NGenXX_codingHexBytes2Str),
    DECLARE_JNI_FUNC("codingHexStr2Bytes", "(Ljava/lang/String;)[B", JNI_NGenXX_codingHexStr2Bytes),

    DECLARE_JNI_FUNC("cryptoRandom", "(I)[B", JNI_NGenXX_cryptoRandom),
    DECLARE_JNI_FUNC("cryptoAesEncrypt", "([B[B)[B", JNI_NGenXX_cryptoAesEncrypt),
    DECLARE_JNI_FUNC("cryptoAesDecrypt", "([B[B)[B", JNI_NGenXX_cryptoAesDecrypt),
    DECLARE_JNI_FUNC("cryptoAesGcmEncrypt", "([B[B[B[BI)[B", JNI_NGenXX_cryptoAesGcmEncrypt),
    DECLARE_JNI_FUNC("cryptoAesGcmDecrypt", "([B[B[B[BI)[B", JNI_NGenXX_cryptoAesGcmDecrypt),
    DECLARE_JNI_FUNC("cryptoHashMd5", "([B)[B", JNI_NGenXX_cryptoHashMd5),
    DECLARE_JNI_FUNC("cryptoHashSha256", "([B)[B", JNI_NGenXX_cryptoHashSha256),
    DECLARE_JNI_FUNC("cryptoBase64Encode", "([B)[B", JNI_NGenXX_cryptoBase64Encode),
    DECLARE_JNI_FUNC("cryptoBase64Decode", "([B)[B", JNI_NGenXX_cryptoBase64Decode),

    DECLARE_JNI_FUNC("jsonDecoderInit", "(Ljava/lang/String;)J", JNI_NGenXX_jsonDecoderInit),
    DECLARE_JNI_FUNC("jsonDecoderIsArray", "(JJ)Z", JNI_NGenXX_jsonDecoderIsArray),
    DECLARE_JNI_FUNC("jsonDecoderIsObject", "(JJ)Z", JNI_NGenXX_jsonDecoderIsObject),
    DECLARE_JNI_FUNC("jsonDecoderReadNode", "(JJLjava/lang/String;)J", JNI_NGenXX_jsonDecoderReadNode),
    DECLARE_JNI_FUNC("jsonDecoderReadString", "(JJ)Ljava/lang/String;", JNI_NGenXX_jsonDecoderReadString),
    DECLARE_JNI_FUNC("jsonDecoderReadNumber", "(JJ)D", JNI_NGenXX_jsonDecoderReadNumber),
    DECLARE_JNI_FUNC("jsonDecoderReadChild", "(JJ)J", JNI_NGenXX_jsonDecoderReadChild),
    DECLARE_JNI_FUNC("jsonDecoderReadNext", "(JJ)J", JNI_NGenXX_jsonDecoderReadNext),
    DECLARE_JNI_FUNC("jsonDecoderRelease", "(J)V", JNI_NGenXX_jsonDecoderRelease),

    DECLARE_JNI_FUNC("zZipInit", "(IJI)J", JNI_NGenXX_zZipInit),
    DECLARE_JNI_FUNC("zZipInput", "(J[BIZ)J", JNI_NGenXX_zZipInput),
    DECLARE_JNI_FUNC("zZipProcessDo", "(J)[B", JNI_NGenXX_zZipProcessDo),
    DECLARE_JNI_FUNC("zZipProcessFinished", "(J)Z", JNI_NGenXX_zZipProcessFinished),
    DECLARE_JNI_FUNC("zZipRelease", "(J)V", JNI_NGenXX_zZipRelease),
    DECLARE_JNI_FUNC("zUnZipInit", "(JI)J", JNI_NGenXX_zUnZipInit),
    DECLARE_JNI_FUNC("zUnZipInput", "(J[BIZ)J", JNI_NGenXX_zUnZipInput),
    DECLARE_JNI_FUNC("zUnZipProcessDo", "(J)[B", JNI_NGenXX_zUnZipProcessDo),
    DECLARE_JNI_FUNC("zUnZipProcessFinished", "(J)Z", JNI_NGenXX_zUnZipProcessFinished),
    DECLARE_JNI_FUNC("zUnZipRelease", "(J)V", JNI_NGenXX_zUnZipRelease),
    DECLARE_JNI_FUNC("zZipBytes", "(IJI[B)[B", JNI_NGenXX_zZipBytes),
    DECLARE_JNI_FUNC("zUnZipBytes", "(JI[B)[B", JNI_NGenXX_zUnZipBytes),
};

int JNI_OnLoad(JavaVM *vm, void *reserved)
{
    sVM = vm;
    JNIEnv *env;
    auto v = JNI_VERSION_1_6;
    auto ret = vm->GetEnv(reinterpret_cast<void **>(&env), v);
    if (ret != JNI_OK)
    {
        return JNI_ERR;
    }

    sJClass = env->FindClass(JClassName);
    if (sJClass == nullptr)
    {
        return JNI_ERR;
    }
    ret = env->RegisterNatives(sJClass, JCFuncList, sizeof(JCFuncList) / sizeof(JNINativeMethod));
    if (ret != JNI_OK)
    {
        return JNI_ERR;
    }
    return v;
}

void JNI_OnUnload(JavaVM *vm, void *reserved)
{
    ngenxx_log_set_callback(nullptr);
    ngenxx_js_set_msg_callback(nullptr);

    auto env = currentEnv(vm);
    env->UnregisterNatives(sJClass);
    env->DeleteWeakGlobalRef(sLogCallback);
    env->DeleteWeakGlobalRef(sJsMsgCallback);

    sVM = nullptr;
    sLogCallback = nullptr;
    sJsMsgCallback = nullptr;
}
