#include <jni.h>
#include <cstdlib>
#include <string>
#include "../../../../../../build.Android/output/include/NGenXX.h"
#include "JNIUtil.hxx"

static JavaVM *sVM;
static jclass sJClass;
static jobject sLogCallback;
static jmethodID sLogCallbackMethodId;
static jobject sJsMsgCallback;
static jmethodID sJsMsgCallbackMethodId;

#pragma mark Engine Callback

static void ngenxx_jni_log_callback(int level, const char *content)
{
    if (sVM == nullptr || sLogCallback == nullptr || sLogCallbackMethodId == nullptr || content == nullptr)
        return;
    runInCurrentEnv(sVM, [level, &content](JNIEnv *env) -> void *
                    {
        jstring jContent = env->NewStringUTF(content);
        free(static_cast<void *>(const_cast<char *>(content)));
        env->CallVoidMethod(sLogCallback, sLogCallbackMethodId, level, jContent);
        return nullptr; });
}

static const char *ngenxx_jni_js_msg_callback(const char *msg)
{
    if (sVM == nullptr || sJsMsgCallback == nullptr || sJsMsgCallbackMethodId == nullptr || msg == nullptr)
        return nullptr;
    auto t = runInCurrentEnv(sVM, [&msg](JNIEnv *env) -> void *
                             {
        jstring jMsg = env->NewStringUTF(msg);
        free(static_cast<void *>(const_cast<char *>(msg)));
        jobject jRes = env->CallObjectMethod(sJsMsgCallback, sJsMsgCallbackMethodId, jMsg);
        auto c = env->GetStringUTFChars(reinterpret_cast<jstring>(jRes), nullptr);
        return reinterpret_cast<void *>(const_cast<char *>(c)); });
    return reinterpret_cast<const char *>(t);
}

#pragma mark Engine base API

jstring NGenXX_JNI_getVersion(JNIEnv *env, jobject thiz)
{
    const char *cV = ngenxx_get_version();
    jstring jstr = env->NewStringUTF(cV ?: "");
    free(static_cast<void *>(const_cast<char *>(cV)));
    return jstr;
}

jboolean NGenXX_JNI_init(JNIEnv *env, jobject thiz,
                         jstring root)
{
    const char *cRoot = env->GetStringUTFChars(root, nullptr);
    jboolean res = ngenxx_init(cRoot);
    env->ReleaseStringUTFChars(root, cRoot);
    return res;
}

void NGenXX_JNI_release(JNIEnv *env, jobject thiz)
{
    ngenxx_release();
}

#pragma mark Log

void NGenXX_JNI_logSetLevel(JNIEnv *env, jobject thiz,
                            jint level)
{
    ngenxx_log_set_level(level);
}

void NGenXX_JNI_logSetCallback(JNIEnv *env, jobject thiz,
                               jobject callback)
{
    if (callback)
    {
        ngenxx_log_set_callback(ngenxx_jni_log_callback);
        sLogCallback = env->NewWeakGlobalRef(callback);
        jclass jcallback_class = env->GetObjectClass(callback);
        if (jcallback_class)
        {
            sLogCallbackMethodId = env->GetMethodID(jcallback_class, "invoke", "(ILjava/lang/String;)V");
        }
    }
}

void NGenXX_JNI_logPrint(JNIEnv *env, jobject thiz,
                         jint level, jstring content)
{
    const char *cContent = env->GetStringUTFChars(content, nullptr);
    ngenxx_log_print(level, cContent);
    env->ReleaseStringUTFChars(content, cContent);
}

#pragma mark Net

jstring NGenXX_JNI_netHttpRequest(JNIEnv *env, jobject thiz,
                                  jstring url, jstring params,
                                  jint method,
                                  jobjectArray headerV,
                                  jobjectArray formFieldNameV,
                                  jobjectArray formFieldMimeV,
                                  jobjectArray formFieldDataV,
                                  jstring filePath, jlong fileLength,
                                  jlong timeout)
{
    const char *cUrl = env->GetStringUTFChars(url, nullptr);
    const char *cParams = env->GetStringUTFChars(params, nullptr);

    const unsigned int headerCount = env->GetArrayLength(headerV);
    char **cHeaderV = reinterpret_cast<char **>(malloc(headerCount * sizeof(char *)));
    auto *jstrHeaderV = reinterpret_cast<jstring *>(malloc(sizeof(jstring)));
    for (int i = 0; i < headerCount; i++)
    {
        jstrHeaderV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(headerV, i));
        cHeaderV[i] = const_cast<char *>(env->GetStringUTFChars(jstrHeaderV[i], nullptr));
    }

    const unsigned int formFieldCount = env->GetArrayLength(formFieldNameV);
    char **cFormFieldNameV = reinterpret_cast<char **>(malloc(formFieldCount * sizeof(char *)));
    auto *jstrFormFieldNameV = reinterpret_cast<jstring *>(malloc(sizeof(jstring)));
    for (int i = 0; i < formFieldCount; i++)
    {
        jstrFormFieldNameV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(formFieldNameV, i));
        cFormFieldNameV[i] = const_cast<char *>(env->GetStringUTFChars(jstrFormFieldNameV[i], nullptr));
    }

    char **cFormFieldMimeV = reinterpret_cast<char **>(malloc(formFieldCount * sizeof(char *)));
    auto *jstrFormFieldMimeV = reinterpret_cast<jstring *>(malloc(sizeof(jstring)));
    for (int i = 0; i < formFieldCount; i++)
    {
        jstrFormFieldMimeV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(formFieldMimeV, i));
        cFormFieldMimeV[i] = const_cast<char *>(env->GetStringUTFChars(jstrFormFieldMimeV[i], nullptr));
    }

    char **cFormFieldDataV = reinterpret_cast<char **>(malloc(formFieldCount * sizeof(char *)));
    auto *jstrFormFieldDataV = reinterpret_cast<jstring *>(malloc(sizeof(jstring)));
    for (int i = 0; i < formFieldCount; i++)
    {
        jstrFormFieldDataV[i] = reinterpret_cast<jstring>(env->GetObjectArrayElement(formFieldDataV, i));
        cFormFieldDataV[i] = const_cast<char *>(env->GetStringUTFChars(jstrFormFieldDataV[i], nullptr));
    }

    const char *cFilePath = env->GetStringUTFChars(filePath, nullptr);
    FILE *cFILE = cFilePath ? std::fopen(cFilePath, "r") : nullptr;

    const char *cRsp = ngenxx_net_http_request(cUrl, cParams, (const int)method,
                                               const_cast<const char **>(cHeaderV), headerCount,
                                               const_cast<const char **>(cFormFieldNameV),
                                               const_cast<const char **>(cFormFieldMimeV),
                                               const_cast<const char **>(cFormFieldDataV),
                                               formFieldCount,
                                               reinterpret_cast<void *>(cFILE), fileLength,
                                               static_cast<const unsigned long>(timeout));
    jstring jstr = env->NewStringUTF(cRsp ?: "");
    free(static_cast<void *>(const_cast<char *>(cRsp)));

    for (int i = 0; i < headerCount; i++)
    {
        env->ReleaseStringUTFChars(jstrHeaderV[i], cHeaderV[i]);
        free(static_cast<void *>(jstrHeaderV[i]));
    }
    for (int i = 0; i < formFieldCount; i++)
    {
        env->ReleaseStringUTFChars(jstrFormFieldNameV[i], cFormFieldNameV[i]);
        free(static_cast<void *>(jstrFormFieldNameV[i]));
        env->ReleaseStringUTFChars(jstrFormFieldMimeV[i], cFormFieldMimeV[i]);
        free(static_cast<void *>(jstrFormFieldMimeV[i]));
        env->ReleaseStringUTFChars(jstrFormFieldDataV[i], cFormFieldDataV[i]);
        free(static_cast<void *>(jstrFormFieldDataV[i]));
    }

    if (cFilePath)
    {
        free(static_cast<void *>(const_cast<char *>(cFilePath)));
    }
    if (cParams)
        env->ReleaseStringUTFChars(params, cParams);
    env->ReleaseStringUTFChars(url, cUrl);

    return jstr;
}

#pragma mark Lua

jboolean NGenXX_JNI_lLoadF(JNIEnv *env, jobject thiz,
                           jstring file)
{
    const char *cFile = env->GetStringUTFChars(file, nullptr);
    jboolean res = ngenxx_lua_loadF(cFile);
    env->ReleaseStringUTFChars(file, cFile);
    return res;
}

jboolean NGenXX_JNI_lLoadS(JNIEnv *env, jobject thiz,
                           jstring script)
{
    const char *cScript = env->GetStringUTFChars(script, nullptr);
    jboolean res = ngenxx_lua_loadS(cScript);
    env->ReleaseStringUTFChars(script, cScript);
    return res;
}

jstring NGenXX_JNI_lCall(JNIEnv *env, jobject thiz,
                         jstring func, jstring params)
{
    const char *cFunc = env->GetStringUTFChars(func, nullptr);
    const char *cParams = params ? env->GetStringUTFChars(params, nullptr) : nullptr;
    const char *cRes = ngenxx_lua_call(cFunc, cParams);
    jstring jstr = env->NewStringUTF(cRes ?: "");
    free(static_cast<void *>(const_cast<char *>(cRes)));
    if (cParams)
        env->ReleaseStringUTFChars(params, cParams);
    env->ReleaseStringUTFChars(func, cFunc);
    return jstr;
}

#pragma mark JS

jboolean NGenXX_JNI_jLoadF(JNIEnv *env, jobject thiz,
                           jstring file)
{
    const char *cFile = env->GetStringUTFChars(file, nullptr);
    jboolean res = ngenxx_js_loadF(cFile);
    env->ReleaseStringUTFChars(file, cFile);
    return res;
}

jboolean NGenXX_JNI_jLoadS(JNIEnv *env, jobject thiz,
                           jstring script, jstring name)
{
    const char *cScript = env->GetStringUTFChars(script, nullptr);
    const char *cName = env->GetStringUTFChars(name, nullptr);
    jboolean res = ngenxx_js_loadS(cScript, cName);
    env->ReleaseStringUTFChars(script, cScript);
    env->ReleaseStringUTFChars(name, cName);
    return res;
}

jboolean NGenXX_JNI_jLoadB(JNIEnv *env, jobject thiz,
                           jbyteArray bytes)
{
    jbyte *cIn = env->GetByteArrayElements(bytes, nullptr);
    size_t inLen = env->GetArrayLength(bytes);

    bool res = ngenxx_js_loadB(reinterpret_cast<const byte *>(cIn), inLen);

    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return res;
}

jstring NGenXX_JNI_jCall(JNIEnv *env, jobject thiz,
                         jstring func, jstring params)
{
    const char *cFunc = env->GetStringUTFChars(func, nullptr);
    const char *cParams = params ? env->GetStringUTFChars(params, nullptr) : nullptr;
    const char *cRes = ngenxx_js_call(cFunc, cParams);
    jstring jstr = env->NewStringUTF(cRes ?: "");
    free(static_cast<void *>(const_cast<char *>(cRes)));
    if (cParams)
        env->ReleaseStringUTFChars(params, cParams);
    env->ReleaseStringUTFChars(func, cFunc);
    return jstr;
}

void NGenXX_JNI_jSetMsgCallback(JNIEnv *env, jobject thiz,
                                jobject callback)
{
    if (callback)
    {
        ngenxx_js_set_msg_callback(ngenxx_jni_js_msg_callback);
        sJsMsgCallback = env->NewWeakGlobalRef(callback);
        jclass jcallback_class = env->GetObjectClass(callback);
        if (jcallback_class)
        {
            sJsMsgCallbackMethodId = env->GetMethodID(jcallback_class, "invoke", "(Ljava/lang/String;)Ljava/lang/String;");
        }
    }
}

#pragma mark Store.SQLite

jlong NGenXX_JNI_storeSQLiteOpen(JNIEnv *env, jobject thiz,
                                 jstring id)
{
    const char *cId = env->GetStringUTFChars(id, nullptr);
    auto res = ngenxx_store_sqlite_open(cId);
    env->ReleaseStringUTFChars(id, cId);
    return reinterpret_cast<jlong>(res);
}

jboolean NGenXX_JNI_storeSQLiteExecute(JNIEnv *env, jobject thiz,
                                       jlong conn,
                                       jstring sql)
{
    const char *cSql = env->GetStringUTFChars(sql, nullptr);
    jboolean res = ngenxx_store_sqlite_execute(reinterpret_cast<void *>(conn), cSql);
    env->ReleaseStringUTFChars(sql, cSql);
    return res;
}

jlong NGenXX_JNI_storeSQLiteQueryDo(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring sql)
{
    const char *cSql = env->GetStringUTFChars(sql, nullptr);
    auto res = ngenxx_store_sqlite_query_do(reinterpret_cast<void *>(conn), cSql);
    env->ReleaseStringUTFChars(sql, cSql);
    return reinterpret_cast<jlong>(res);
}

jboolean NGenXX_JNI_storeSQLiteQueryReadRow(JNIEnv *env, jobject thiz,
                                            jlong query_result)
{
    jboolean res = ngenxx_store_sqlite_query_read_row(reinterpret_cast<void *>(query_result));
    return res;
}

jstring NGenXX_JNI_storeSQLiteQueryReadColumnText(JNIEnv *env, jobject thiz,
                                                  jlong query_result,
                                                  jstring column)
{
    const char *cColumn = env->GetStringUTFChars(column, nullptr);
    const char *cRes = ngenxx_store_sqlite_query_read_column_text(reinterpret_cast<void *>(query_result), cColumn);
    jstring jstr = env->NewStringUTF(cRes ?: "");
    free(static_cast<void *>(const_cast<char *>(cRes)));
    env->ReleaseStringUTFChars(column, cColumn);
    return jstr;
}

jlong NGenXX_JNI_storeSQLiteQueryReadColumnInteger(JNIEnv *env, jobject thiz,
                                                   jlong query_result,
                                                   jstring column)
{
    const char *cColumn = env->GetStringUTFChars(column, nullptr);
    jlong res = ngenxx_store_sqlite_query_read_column_integer(reinterpret_cast<void *>(query_result), cColumn);
    env->ReleaseStringUTFChars(column, cColumn);
    return res;
}

jdouble NGenXX_JNI_storeSQLiteQueryReadColumnFloat(JNIEnv *env, jobject thiz,
                                                   jlong query_result,
                                                   jstring column)
{
    const char *cColumn = env->GetStringUTFChars(column, nullptr);
    jdouble res = ngenxx_store_sqlite_query_read_column_float(reinterpret_cast<void *>(query_result), cColumn);
    env->ReleaseStringUTFChars(column, cColumn);
    return res;
}

void NGenXX_JNI_storeSQLiteQueryDrop(JNIEnv *env, jobject thiz,
                                     jlong query_result)
{
    ngenxx_store_sqlite_query_drop(reinterpret_cast<void *>(query_result));
}

void NGenXX_JNI_storeSQLiteClose(JNIEnv *env, jobject thiz,
                                 jlong conn)
{
    ngenxx_store_sqlite_close(reinterpret_cast<void *>(conn));
}

#pragma mark Srore.KV

jlong NGenXX_JNI_storeKVOpen(JNIEnv *env, jobject thiz,
                             jstring id)
{
    const char *cId = env->GetStringUTFChars(id, nullptr);
    auto res = ngenxx_store_kv_open(cId);
    env->ReleaseStringUTFChars(id, cId);
    return reinterpret_cast<jlong>(res);
}

jstring NGenXX_JNI_storeKVReadString(JNIEnv *env, jobject thiz,
                                     jlong conn,
                                     jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    const char *cRes = ngenxx_store_kv_read_string(reinterpret_cast<void *>(conn), cK);
    jstring jstr = env->NewStringUTF(cRes ?: "");
    free(static_cast<void *>(const_cast<char *>(cRes)));
    env->ReleaseStringUTFChars(k, cK);
    return jstr;
}

jboolean NGenXX_JNI_storeKVWriteString(JNIEnv *env, jobject thiz,
                                       jlong conn,
                                       jstring k, jstring v)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    const char *cV = env->GetStringUTFChars(v, nullptr);
    jboolean res = ngenxx_store_kv_write_string(reinterpret_cast<void *>(conn), cK, cV);
    env->ReleaseStringUTFChars(v, cV);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jlong NGenXX_JNI_storeKVReadInteger(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jlong res = ngenxx_store_kv_read_integer(reinterpret_cast<void *>(conn), cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jboolean NGenXX_JNI_storeKVWriteInteger(JNIEnv *env, jobject thiz,
                                        jlong conn,
                                        jstring k, jlong v)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jboolean res = ngenxx_store_kv_write_integer(reinterpret_cast<void *>(conn), cK, v);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jdouble NGenXX_JNI_storeKVReadFloat(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jdouble res = ngenxx_store_kv_read_float(reinterpret_cast<void *>(conn), cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jboolean NGenXX_JNI_storeKVWriteFloat(JNIEnv *env, jobject thiz,
                                      jlong conn,
                                      jstring k, jdouble v)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jboolean res = ngenxx_store_kv_write_float(reinterpret_cast<void *>(conn), cK, v);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jboolean NGenXX_JNI_storeKVContains(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jboolean res = ngenxx_store_kv_contains(reinterpret_cast<void *>(conn), cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

void NGenXX_JNI_storeKVClear(JNIEnv *env, jobject thiz,
                             jlong conn)
{
    ngenxx_store_kv_clear(reinterpret_cast<void *>(conn));
}

void NGenXX_JNI_storeKVClose(JNIEnv *env, jobject thiz,
                             jlong conn)
{
    ngenxx_store_kv_close(reinterpret_cast<void *>(conn));
}

#pragma mark Device

jint NGenXX_JNI_deviceType(JNIEnv *env, jobject thiz)
{
    return ngenxx_device_type();
}

jstring NGenXX_JNI_deviceName(JNIEnv *env, jobject thiz)
{
    const char *cDN = ngenxx_device_name();
    jstring jstr = env->NewStringUTF(cDN ?: "");
    free(static_cast<void *>(const_cast<char *>(cDN)));
    return jstr;
}

jstring NGenXX_JNI_deviceManufacturer(JNIEnv *env, jobject thiz)
{
    const char *cDM = ngenxx_device_manufacturer();
    jstring jstr = env->NewStringUTF(cDM ?: "");
    free(static_cast<void *>(const_cast<char *>(cDM)));
    return jstr;
}

jstring NGenXX_JNI_deviceOsVersion(JNIEnv *env, jobject thiz)
{
    const char *cDOV = ngenxx_device_os_version();
    jstring jstr = env->NewStringUTF(cDOV ?: "");
    free(static_cast<void *>(const_cast<char *>(cDOV)));
    return jstr;
}

jint NGenXX_JNI_deviceCpuArch(JNIEnv *env, jobject thiz)
{
    return ngenxx_device_cpu_arch();
}

#pragma mark Coding

jstring NGenXX_JNI_codingHexBytes2Str(JNIEnv *env, jobject thiz,
                                      jbyteArray bytes)
{
    jbyte *cIn = env->GetByteArrayElements(bytes, nullptr);
    size_t inLen = env->GetArrayLength(bytes);

    auto cRes = ngenxx_coding_hex_bytes2str(reinterpret_cast<const byte *>(cIn), inLen);
    jstring jstr = env->NewStringUTF(cRes ?: "");

    free(static_cast<void *>(const_cast<char *>(cRes)));
    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return jstr;
}

jbyteArray NGenXX_JNI_codingHexStr2Bytes(JNIEnv *env,
                                         jobject thiz,
                                         jstring str)
{
    const char *cStr = env->GetStringUTFChars(str, nullptr);

    size_t outLen;
    auto cRes = ngenxx_coding_hex_str2bytes(cStr, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseStringUTFChars(str, cStr);
    return jba;
}

#pragma mark Crypto

jbyteArray NGenXX_JNI_cryptoRandom(JNIEnv *env, jobject thiz,
                                   jint len)
{
    byte out[len];
    std::memset(out, 0, len);
    ngenxx_crypto_rand(len, out);
    ngenxx_crypto_rand(len, out);
    return moveToJByteArray(env, out, len, false);
}

jbyteArray NGenXX_JNI_cryptoAesEncrypt(JNIEnv *env, jobject thiz,
                                       jbyteArray input,
                                       jbyteArray key)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size_t inLen = env->GetArrayLength(input);
    jbyte *cKey = env->GetByteArrayElements(key, nullptr);
    size_t keyLen = env->GetArrayLength(key);

    size_t outLen;
    auto cRes = ngenxx_crypto_aes_encrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                          reinterpret_cast<const byte *>(cKey), keyLen,
                                          &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_cryptoAesDecrypt(JNIEnv *env, jobject thiz,
                                       jbyteArray input,
                                       jbyteArray key)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size_t inLen = env->GetArrayLength(input);
    jbyte *cKey = env->GetByteArrayElements(key, nullptr);
    size_t keyLen = env->GetArrayLength(key);

    size_t outLen;
    auto cRes = ngenxx_crypto_aes_decrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                          reinterpret_cast<const byte *>(cKey), keyLen,
                                          &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_cryptoAesGcmEncrypt(JNIEnv *env, jobject thiz,
                                          jbyteArray input,
                                          jbyteArray key,
                                          jbyteArray init_vector,
                                          jbyteArray aad,
                                          jint tag_bits)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size_t inLen = env->GetArrayLength(input);
    jbyte *cKey = env->GetByteArrayElements(key, nullptr);
    size_t keyLen = env->GetArrayLength(key);
    jbyte *cIv = env->GetByteArrayElements(init_vector, nullptr);
    size_t ivLen = env->GetArrayLength(init_vector);
    jbyte *cAad = aad ? env->GetByteArrayElements(aad, nullptr) : nullptr;
    int aadLen = aad ? env->GetArrayLength(aad) : 0;

    size_t outLen;
    auto cRes = ngenxx_crypto_aes_gcm_encrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                              reinterpret_cast<const byte *>(cKey), keyLen,
                                              reinterpret_cast<const byte *>(cIv), ivLen,
                                              reinterpret_cast<const byte *>(cAad), aadLen,
                                              tag_bits, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
    env->ReleaseByteArrayElements(init_vector, cIv, JNI_ABORT);
    if (cAad)
        env->ReleaseByteArrayElements(aad, cAad, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_cryptoAesGcmDecrypt(JNIEnv *env, jobject thiz,
                                          jbyteArray input,
                                          jbyteArray key,
                                          jbyteArray init_vector,
                                          jbyteArray aad,
                                          jint tag_bits)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size_t inLen = env->GetArrayLength(input);
    jbyte *cKey = env->GetByteArrayElements(key, nullptr);
    size_t keyLen = env->GetArrayLength(key);
    jbyte *cIv = env->GetByteArrayElements(init_vector, nullptr);
    size_t ivLen = env->GetArrayLength(init_vector);
    jbyte *cAad = aad ? env->GetByteArrayElements(aad, nullptr) : nullptr;
    int aadLen = aad ? env->GetArrayLength(aad) : 0;

    size_t outLen;
    auto cRes = ngenxx_crypto_aes_gcm_decrypt(reinterpret_cast<const byte *>(cIn), inLen,
                                              reinterpret_cast<const byte *>(cKey), keyLen,
                                              reinterpret_cast<const byte *>(cIv), ivLen,
                                              reinterpret_cast<const byte *>(cAad), aadLen,
                                              tag_bits, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
    env->ReleaseByteArrayElements(init_vector, cIv, JNI_ABORT);
    if (cAad)
        env->ReleaseByteArrayElements(aad, cAad, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_cryptoHashMd5(JNIEnv *env, jobject thiz,
                                    jbyteArray input)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size_t inLen = env->GetArrayLength(input);

    size_t outLen;
    auto cRes = ngenxx_crypto_hash_md5(reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_cryptoHashSha256(JNIEnv *env, jobject thiz,
                                       jbyteArray input)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size_t inLen = env->GetArrayLength(input);

    size_t outLen;
    auto cRes = ngenxx_crypto_hash_sha256(reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_cryptoBase64Encode(JNIEnv *env, jobject thiz,
                                         jbyteArray input)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size_t inLen = env->GetArrayLength(input);

    size_t outLen;
    auto cRes = ngenxx_crypto_base64_encode(reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_cryptoBase64Decode(JNIEnv *env, jobject thiz,
                                         jbyteArray input)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size_t inLen = env->GetArrayLength(input);

    size_t outLen;
    auto cRes = ngenxx_crypto_base64_decode(reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

#pragma mark JsonDecoder

jlong NGenXX_JNI_jsonDecoderInit(JNIEnv *env, jobject thiz,
                                 jstring json)
{
    const char *cJson = env->GetStringUTFChars(json, nullptr);
    auto res = ngenxx_json_decoder_init(cJson);
    env->ReleaseStringUTFChars(json, cJson);
    return reinterpret_cast<jlong>(res);
}

jboolean NGenXX_JNI_jsonDecoderIsArray(JNIEnv *env, jobject thiz,
                                       jlong decoder,
                                       jlong node)
{
    return ngenxx_json_decoder_is_array(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node));
}

jboolean NGenXX_JNI_jsonDecoderIsObject(JNIEnv *env, jobject thiz,
                                        jlong decoder,
                                        jlong node)
{
    return ngenxx_json_decoder_is_object(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node));
}

jlong NGenXX_JNI_jsonDecoderReadNode(JNIEnv *env, jobject thiz,
                                     jlong decoder,
                                     jlong node,
                                     jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    auto res = ngenxx_json_decoder_read_node(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node), cK);
    env->ReleaseStringUTFChars(k, cK);
    return reinterpret_cast<jlong>(res);
}

jstring NGenXX_JNI_jsonDecoderReadString(JNIEnv *env, jobject thiz,
                                         jlong decoder,
                                         jlong node)
{
    const char *res = ngenxx_json_decoder_read_string(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node));
    jstring jstr = env->NewStringUTF(res ?: "");
    free(static_cast<void *>(const_cast<char *>(res)));
    return jstr;
}

jdouble NGenXX_JNI_jsonDecoderReadNumber(JNIEnv *env, jobject thiz,
                                         jlong decoder,
                                         jlong node)
{
    return ngenxx_json_decoder_read_number(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node));
}

jlong NGenXX_JNI_jsonDecoderReadChild(JNIEnv *env, jobject thiz,
                                      jlong decoder,
                                      jlong node)
{
    return reinterpret_cast<jlong>(ngenxx_json_decoder_read_child(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node)));
}

jlong NGenXX_JNI_jsonDecoderReadNext(JNIEnv *env, jobject thiz,
                                     jlong decoder,
                                     jlong node)
{
    return reinterpret_cast<jlong>(ngenxx_json_decoder_read_next(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node)));
}

void NGenXX_JNI_jsonDecoderRelease(JNIEnv *env, jobject thiz,
                                   jlong decoder)
{
    ngenxx_json_decoder_release(reinterpret_cast<void *>(decoder));
}

#pragma mark Zip

jlong NGenXX_JNI_zZipInit(JNIEnv *env, jobject thiz,
                          jint mode, jlong bufferSize, jint format)
{
    return reinterpret_cast<jlong>(ngenxx_z_zip_init(mode, bufferSize, format));
}

jlong NGenXX_JNI_zZipInput(JNIEnv *env, jobject thiz,
                           jlong zip,
                           jbyteArray inBytes, jint inLen, jboolean inFinish)
{
    jbyte *cIn = env->GetByteArrayElements(inBytes, nullptr);
    // size_t inLen = env->GetArrayLength(input);

    size_t ret = ngenxx_z_zip_input(reinterpret_cast<void *>(zip), reinterpret_cast<const byte *>(cIn), inLen, inFinish);

    env->ReleaseByteArrayElements(inBytes, cIn, JNI_ABORT);

    return ret;
}

jbyteArray NGenXX_JNI_zZipProcessDo(JNIEnv *env, jobject thiz, jlong zip)
{
    size_t outLen = 0;
    auto cRes = ngenxx_z_zip_process_do(reinterpret_cast<void *>(zip), &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    return jba;
}

jboolean NGenXX_JNI_zZipProcessFinished(JNIEnv *env, jobject thiz,
                                        jlong zip)
{
    return ngenxx_z_zip_process_finished(reinterpret_cast<void *>(zip));
}

void NGenXX_JNI_zZipRelease(JNIEnv *env, jobject thiz,
                            jlong zip)
{
    ngenxx_z_zip_release(reinterpret_cast<void *>(zip));
}

jlong NGenXX_JNI_zUnZipInit(JNIEnv *env, jobject thiz,
                            jlong bufferSize, jint format)
{
    return reinterpret_cast<jlong>(ngenxx_z_unzip_init(bufferSize, format));
}

jlong NGenXX_JNI_zUnZipInput(JNIEnv *env, jobject thiz,
                             jlong unzip,
                             jbyteArray inBytes, jint inLen, jboolean inFinish)
{
    jbyte *cIn = env->GetByteArrayElements(inBytes, nullptr);
    // size_t inLen = env->GetArrayLength(input);

    size_t ret = ngenxx_z_unzip_input(reinterpret_cast<void *>(unzip), reinterpret_cast<const byte *>(cIn), inLen, inFinish);

    env->ReleaseByteArrayElements(inBytes, cIn, JNI_ABORT);

    return ret;
}

jbyteArray NGenXX_JNI_zUnZipProcessDo(JNIEnv *env, jobject thiz, jlong unzip)
{
    size_t outLen = 0;
    auto cRes = ngenxx_z_unzip_process_do(reinterpret_cast<void *>(unzip), &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    return jba;
}

jboolean NGenXX_JNI_zUnZipProcessFinished(JNIEnv *env, jobject thiz,
                                          jlong unzip)
{
    return ngenxx_z_unzip_process_finished(reinterpret_cast<void *>(unzip));
}

void NGenXX_JNI_zUnZipRelease(JNIEnv *env, jobject thiz,
                              jlong unzip)
{
    ngenxx_z_unzip_release(reinterpret_cast<void *>(unzip));
}

jbyteArray NGenXX_JNI_zZipBytes(JNIEnv *env, jobject thiz,
                                jint mode,
                                jlong buffer_size, jint format, jbyteArray bytes)
{
    jbyte *cIn = env->GetByteArrayElements(bytes, nullptr);
    size_t inLen = env->GetArrayLength(bytes);

    size_t outLen;
    auto cRes = ngenxx_z_bytes_zip(mode, buffer_size, format, reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_zUnZipBytes(JNIEnv *env, jobject thiz,
                                  jlong buffer_size, jint format, jbyteArray bytes)
{
    jbyte *cIn = env->GetByteArrayElements(bytes, nullptr);
    size_t inLen = env->GetArrayLength(bytes);

    size_t outLen;
    auto cRes = ngenxx_z_bytes_unzip(buffer_size, format, reinterpret_cast<const byte *>(cIn), inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return jba;
}

static const char *JClassName = "xyz/rinc/ngenxx/NGenXX$Companion";

#define DECLARE_JNI_FUNC(name, signature, funcPtr) {name, signature, reinterpret_cast<void *>(funcPtr)}

static const JNINativeMethod JCFuncList[] = {
    DECLARE_JNI_FUNC("getVersion", "()Ljava/lang/String;", NGenXX_JNI_getVersion),
    DECLARE_JNI_FUNC("init", "(Ljava/lang/String;)Z", NGenXX_JNI_init),
    DECLARE_JNI_FUNC("release", "()V", NGenXX_JNI_release),

    DECLARE_JNI_FUNC("logSetLevel", "(I)V", NGenXX_JNI_logSetLevel),
    DECLARE_JNI_FUNC("logSetCallback", "(Lkotlin/jvm/functions/Function2;)V", NGenXX_JNI_logSetCallback),
    DECLARE_JNI_FUNC("logPrint", "(ILjava/lang/String;)V", NGenXX_JNI_logPrint),

    DECLARE_JNI_FUNC("netHttpRequest", "(Ljava/lang/String;Ljava/lang/String;I[Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;JJ)Ljava/lang/String;", NGenXX_JNI_netHttpRequest),

    DECLARE_JNI_FUNC("lLoadF", "(Ljava/lang/String;)Z", NGenXX_JNI_lLoadF),
    DECLARE_JNI_FUNC("lLoadS", "(Ljava/lang/String;)Z", NGenXX_JNI_lLoadS),
    DECLARE_JNI_FUNC("lCall", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", NGenXX_JNI_lCall),

    DECLARE_JNI_FUNC("jLoadF", "(Ljava/lang/String;)Z", NGenXX_JNI_jLoadF),
    DECLARE_JNI_FUNC("jLoadS", "(Ljava/lang/String;Ljava/lang/String;)Z", NGenXX_JNI_jLoadS),
    DECLARE_JNI_FUNC("jLoadB", "([B)Z", NGenXX_JNI_jLoadB),
    DECLARE_JNI_FUNC("jCall", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", NGenXX_JNI_jCall),
    DECLARE_JNI_FUNC("jSetMsgCallback", "(Lkotlin/jvm/functions/Function1;)V", NGenXX_JNI_jSetMsgCallback),

    DECLARE_JNI_FUNC("storeSQLiteOpen", "(Ljava/lang/String;)J", NGenXX_JNI_storeSQLiteOpen),
    DECLARE_JNI_FUNC("storeSQLiteExecute", "(JLjava/lang/String;)Z", NGenXX_JNI_storeSQLiteExecute),
    DECLARE_JNI_FUNC("storeSQLiteQueryDo", "(JLjava/lang/String;)J", NGenXX_JNI_storeSQLiteQueryDo),
    DECLARE_JNI_FUNC("storeSQLiteQueryReadRow", "(J)Z", NGenXX_JNI_storeSQLiteQueryReadRow),
    DECLARE_JNI_FUNC("storeSQLiteQueryReadColumnText", "(JLjava/lang/String;)Ljava/lang/String;", NGenXX_JNI_storeSQLiteQueryReadColumnText),
    DECLARE_JNI_FUNC("storeSQLiteQueryReadColumnInteger", "(JLjava/lang/String;)J", NGenXX_JNI_storeSQLiteQueryReadColumnInteger),
    DECLARE_JNI_FUNC("storeSQLiteQueryReadColumnFloat", "(JLjava/lang/String;)D", NGenXX_JNI_storeSQLiteQueryReadColumnFloat),
    DECLARE_JNI_FUNC("storeSQLiteQueryDrop", "(J)V", NGenXX_JNI_storeSQLiteQueryDrop),
    DECLARE_JNI_FUNC("storeSQLiteClose", "(J)V", NGenXX_JNI_storeSQLiteClose),

    DECLARE_JNI_FUNC("storeKVOpen", "(Ljava/lang/String;)J", NGenXX_JNI_storeKVOpen),
    DECLARE_JNI_FUNC("storeKVReadString", "(JLjava/lang/String;)Ljava/lang/String;", NGenXX_JNI_storeKVReadString),
    DECLARE_JNI_FUNC("storeKVWriteString", "(JLjava/lang/String;Ljava/lang/String;)Z", NGenXX_JNI_storeKVWriteString),
    DECLARE_JNI_FUNC("storeKVReadInteger", "(JLjava/lang/String;)J", NGenXX_JNI_storeKVReadInteger),
    DECLARE_JNI_FUNC("storeKVWriteInteger", "(JLjava/lang/String;J)Z", NGenXX_JNI_storeKVWriteInteger),
    DECLARE_JNI_FUNC("storeKVReadFloat", "(JLjava/lang/String;)D", NGenXX_JNI_storeKVReadFloat),
    DECLARE_JNI_FUNC("storeKVWriteFloat", "(JLjava/lang/String;D)Z", NGenXX_JNI_storeKVWriteFloat),
    DECLARE_JNI_FUNC("storeKVContains", "(JLjava/lang/String;)Z", NGenXX_JNI_storeKVContains),
    DECLARE_JNI_FUNC("storeKVClear", "(J)V", NGenXX_JNI_storeKVClear),
    DECLARE_JNI_FUNC("storeKVClose", "(J)V", NGenXX_JNI_storeKVClose),

    DECLARE_JNI_FUNC("deviceType", "()I", NGenXX_JNI_deviceType),
    DECLARE_JNI_FUNC("deviceName", "()Ljava/lang/String;", NGenXX_JNI_deviceName),
    DECLARE_JNI_FUNC("deviceManufacturer", "()Ljava/lang/String;", NGenXX_JNI_deviceManufacturer),
    DECLARE_JNI_FUNC("deviceOsVersion", "()Ljava/lang/String;", NGenXX_JNI_deviceOsVersion),
    DECLARE_JNI_FUNC("deviceCpuArch", "()I", NGenXX_JNI_deviceCpuArch),

    DECLARE_JNI_FUNC("codingHexBytes2Str", "([B)Ljava/lang/String;", NGenXX_JNI_codingHexBytes2Str),
    DECLARE_JNI_FUNC("codingHexStr2Bytes", "(Ljava/lang/String;)[B", NGenXX_JNI_codingHexStr2Bytes),

    DECLARE_JNI_FUNC("cryptoRandom", "(I)[B", NGenXX_JNI_cryptoRandom),
    DECLARE_JNI_FUNC("cryptoAesEncrypt", "([B[B)[B", NGenXX_JNI_cryptoAesEncrypt),
    DECLARE_JNI_FUNC("cryptoAesDecrypt", "([B[B)[B", NGenXX_JNI_cryptoAesDecrypt),
    DECLARE_JNI_FUNC("cryptoAesGcmEncrypt", "([B[B[B[BI)[B", NGenXX_JNI_cryptoAesGcmEncrypt),
    DECLARE_JNI_FUNC("cryptoAesGcmDecrypt", "([B[B[B[BI)[B", NGenXX_JNI_cryptoAesGcmDecrypt),
    DECLARE_JNI_FUNC("cryptoHashMd5", "([B)[B", NGenXX_JNI_cryptoHashMd5),
    DECLARE_JNI_FUNC("cryptoHashSha256", "([B)[B", NGenXX_JNI_cryptoHashSha256),
    DECLARE_JNI_FUNC("cryptoBase64Encode", "([B)[B", NGenXX_JNI_cryptoBase64Encode),
    DECLARE_JNI_FUNC("cryptoBase64Decode", "([B)[B", NGenXX_JNI_cryptoBase64Decode),

    DECLARE_JNI_FUNC("jsonDecoderInit", "(Ljava/lang/String;)J", NGenXX_JNI_jsonDecoderInit),
    DECLARE_JNI_FUNC("jsonDecoderIsArray", "(JJ)Z", NGenXX_JNI_jsonDecoderIsArray),
    DECLARE_JNI_FUNC("jsonDecoderIsObject", "(JJ)Z", NGenXX_JNI_jsonDecoderIsObject),
    DECLARE_JNI_FUNC("jsonDecoderReadNode", "(JJLjava/lang/String;)J", NGenXX_JNI_jsonDecoderReadNode),
    DECLARE_JNI_FUNC("jsonDecoderReadString", "(JJ)Ljava/lang/String;", NGenXX_JNI_jsonDecoderReadString),
    DECLARE_JNI_FUNC("jsonDecoderReadNumber", "(JJ)D", NGenXX_JNI_jsonDecoderReadNumber),
    DECLARE_JNI_FUNC("jsonDecoderReadChild", "(JJ)J", NGenXX_JNI_jsonDecoderReadChild),
    DECLARE_JNI_FUNC("jsonDecoderReadNext", "(JJ)J", NGenXX_JNI_jsonDecoderReadNext),
    DECLARE_JNI_FUNC("jsonDecoderRelease", "(J)V", NGenXX_JNI_jsonDecoderRelease),

    DECLARE_JNI_FUNC("zZipInit", "(IJI)J", NGenXX_JNI_zZipInit),
    DECLARE_JNI_FUNC("zZipInput", "(J[BIZ)J", NGenXX_JNI_zZipInput),
    DECLARE_JNI_FUNC("zZipProcessDo", "(J)[B", NGenXX_JNI_zZipProcessDo),
    DECLARE_JNI_FUNC("zZipProcessFinished", "(J)Z", NGenXX_JNI_zZipProcessFinished),
    DECLARE_JNI_FUNC("zZipRelease", "(J)V", NGenXX_JNI_zZipRelease),
    DECLARE_JNI_FUNC("zUnZipInit", "(JI)J", NGenXX_JNI_zUnZipInit),
    DECLARE_JNI_FUNC("zUnZipInput", "(J[BIZ)J", NGenXX_JNI_zUnZipInput),
    DECLARE_JNI_FUNC("zUnZipProcessDo", "(J)[B", NGenXX_JNI_zUnZipProcessDo),
    DECLARE_JNI_FUNC("zUnZipProcessFinished", "(J)Z", NGenXX_JNI_zUnZipProcessFinished),
    DECLARE_JNI_FUNC("zUnZipRelease", "(J)V", NGenXX_JNI_zUnZipRelease),
    DECLARE_JNI_FUNC("zZipBytes", "(IJI[B)[B", NGenXX_JNI_zZipBytes),
    DECLARE_JNI_FUNC("zUnZipBytes", "(JI[B)[B", NGenXX_JNI_zUnZipBytes),
};

int JNI_OnLoad(JavaVM *vm, void *reserved)
{
    sVM = vm;
    JNIEnv *env;
    int v = JNI_VERSION_1_6;
    int ret = vm->GetEnv(reinterpret_cast<void **>(&env), v);
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

    runInCurrentEnv(vm, [](JNIEnv *env) -> void *
                    {
        env->UnregisterNatives(sJClass);
        env->DeleteWeakGlobalRef(sLogCallback);
        env->DeleteWeakGlobalRef(sJsMsgCallback);
        return nullptr; });

    sVM = nullptr;
    sLogCallback = nullptr;
    sJsMsgCallback = nullptr;
}
