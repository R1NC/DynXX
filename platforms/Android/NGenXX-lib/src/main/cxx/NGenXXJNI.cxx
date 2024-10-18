#include <jni.h>
#include <cstdlib>
#include <string>
#include "../../../../../../build.Android/output/include/NGenXX.h"
#include "JNIUtil.hxx"

jstring NGenXX_JNI_getVersion(JNIEnv *env, jobject thiz)
{
    const char *cV = ngenxx_get_version();
    jstring jstr = env->NewStringUTF(cV ?: "");
    free((void *)cV);
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

static JNIEnv *sEnv;
static jobject sLogCallback;
static jmethodID sLogCallbackMethodId;

static void ngenxx_jni_log_callback(int level, const char *content)
{
    if (sEnv && sLogCallback && sLogCallbackMethodId && content)
    {
        jstring jContent = sEnv->NewStringUTF(content);
        free((void *)content);
        sEnv->CallVoidMethod(sLogCallback, sLogCallbackMethodId, level, jContent);
    }
}

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
        sEnv = env;
        sLogCallback = env->NewWeakGlobalRef(callback);
        jclass jcallback_class = env->GetObjectClass(callback);
        if (jcallback_class)
        {
            sLogCallbackMethodId = env->GetMethodID(jcallback_class, "invoke", "(ILjava/lang/String;)V");
        }
    }
    else
    {
        ngenxx_log_set_callback(nullptr);
        sEnv = nullptr;
        env->DeleteWeakGlobalRef(sLogCallback);
        sLogCallback = nullptr;
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
    char **cHeaderV = (char **)malloc(headerCount * sizeof(char *));
    auto *jstrHeaderV = (jstring *)malloc(sizeof(jstring));
    for (int i = 0; i < headerCount; i++)
    {
        jstrHeaderV[i] = (jstring)(env->GetObjectArrayElement(headerV, i));
        cHeaderV[i] = (char *)env->GetStringUTFChars(jstrHeaderV[i], nullptr);
    }

    const unsigned int formFieldCount = env->GetArrayLength(formFieldNameV);
    char **cFormFieldNameV = (char **)malloc(formFieldCount * sizeof(char *));
    auto *jstrFormFieldNameV = (jstring *)malloc(sizeof(jstring));
    for (int i = 0; i < formFieldCount; i++)
    {
        jstrFormFieldNameV[i] = (jstring)(env->GetObjectArrayElement(formFieldNameV, i));
        cFormFieldNameV[i] = (char *)env->GetStringUTFChars(jstrFormFieldNameV[i], nullptr);
    }

    char **cFormFieldMimeV = (char **)malloc(formFieldCount * sizeof(char *));
    auto *jstrFormFieldMimeV = (jstring *)malloc(sizeof(jstring));
    for (int i = 0; i < formFieldCount; i++)
    {
        jstrFormFieldMimeV[i] = (jstring)(env->GetObjectArrayElement(formFieldMimeV, i));
        cFormFieldMimeV[i] = (char *)env->GetStringUTFChars(jstrFormFieldMimeV[i], nullptr);
    }

    char **cFormFieldDataV = (char **)malloc(formFieldCount * sizeof(char *));
    auto *jstrFormFieldDataV = (jstring *)malloc(sizeof(jstring));
    for (int i = 0; i < formFieldCount; i++)
    {
        jstrFormFieldDataV[i] = (jstring)(env->GetObjectArrayElement(formFieldDataV, i));
        cFormFieldDataV[i] = (char *)env->GetStringUTFChars(jstrFormFieldDataV[i], nullptr);
    }

    const char *cFilePath = env->GetStringUTFChars(filePath, nullptr);
    FILE *cFILE = cFilePath ? std::fopen(cFilePath, "r") : nullptr;

    const char *cRsp = ngenxx_net_http_request(cUrl, cParams, (const int)method,
                                               (const char **)cHeaderV, headerCount,
                                               (const char **)cFormFieldNameV,
                                               (const char **)cFormFieldMimeV,
                                               (const char **)cFormFieldDataV,
                                               formFieldCount,
                                               (void *)cFILE, fileLength,
                                               (const unsigned long)timeout);
    jstring jstr = env->NewStringUTF(cRsp ?: "");
    free((void *)cRsp);

    for (int i = 0; i < headerCount; i++)
    {
        env->ReleaseStringUTFChars(jstrHeaderV[i], cHeaderV[i]);
        free((void *)jstrHeaderV[i]);
    }
    for (int i = 0; i < formFieldCount; i++)
    {
        env->ReleaseStringUTFChars(jstrFormFieldNameV[i], cFormFieldNameV[i]);
        free((void *)jstrFormFieldNameV[i]);
        env->ReleaseStringUTFChars(jstrFormFieldMimeV[i], cFormFieldMimeV[i]);
        free((void *)jstrFormFieldMimeV[i]);
        env->ReleaseStringUTFChars(jstrFormFieldDataV[i], cFormFieldDataV[i]);
        free((void *)jstrFormFieldDataV[i]);
    }

    if (cFilePath)
    {
        free((void *)cFilePath);
    }
    env->ReleaseStringUTFChars(params, cParams);
    env->ReleaseStringUTFChars(url, cUrl);

    return jstr;
}

#pragma mark Lua

jboolean NGenXX_JNI_lLoadF(JNIEnv *env, jobject thiz,
                           jstring file)
{
    const char *cFile = env->GetStringUTFChars(file, nullptr);
    jboolean res = ngenxx_L_loadF(cFile);
    env->ReleaseStringUTFChars(file, cFile);
    return res;
}

jboolean NGenXX_JNI_lLoadS(JNIEnv *env, jobject thiz,
                           jstring script)
{
    const char *cScript = env->GetStringUTFChars(script, nullptr);
    jboolean res = ngenxx_L_loadS(cScript);
    env->ReleaseStringUTFChars(script, cScript);
    return res;
}

jstring NGenXX_JNI_lCall(JNIEnv *env, jobject thiz,
                         jstring func, jstring params)
{
    const char *cFunc = env->GetStringUTFChars(func, nullptr);
    const char *cParams = params ? env->GetStringUTFChars(params, nullptr) : nullptr;
    const char *cRes = ngenxx_L_call(cFunc, cParams);
    jstring jstr = env->NewStringUTF(cRes ?: "");
    free((void *)cRes);
    env->ReleaseStringUTFChars(params, cParams);
    env->ReleaseStringUTFChars(func, cFunc);
    return jstr;
}

#pragma mark JS

jboolean NGenXX_JNI_jLoadF(JNIEnv *env, jobject thiz,
                           jstring file)
{
    const char *cFile = env->GetStringUTFChars(file, nullptr);
    jboolean res = ngenxx_J_loadF(cFile);
    env->ReleaseStringUTFChars(file, cFile);
    return res;
}

jboolean NGenXX_JNI_jLoadS(JNIEnv *env, jobject thiz,
                           jstring script, jstring name)
{
    const char *cScript = env->GetStringUTFChars(script, nullptr);
    const char *cName = env->GetStringUTFChars(name, nullptr);
    jboolean res = ngenxx_J_loadS(cScript, cName);
    env->ReleaseStringUTFChars(script, cScript);
    env->ReleaseStringUTFChars(name, cName);
    return res;
}

jboolean NGenXX_JNI_jLoadB(JNIEnv *env, jobject thiz,
                           jbyteArray bytes)
{
    jbyte *cIn = env->GetByteArrayElements(bytes, nullptr);
    size inLen = env->GetArrayLength(bytes);

    bool res = ngenxx_J_loadB((const byte *)cIn, inLen);

    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return res;
}

jstring NGenXX_JNI_jCall(JNIEnv *env, jobject thiz,
                         jstring func, jstring params)
{
    const char *cFunc = env->GetStringUTFChars(func, nullptr);
    const char *cParams = params ? env->GetStringUTFChars(params, nullptr) : nullptr;
    const char *cRes = ngenxx_J_call(cFunc, cParams);
    jstring jstr = env->NewStringUTF(cRes ?: "");
    free((void *)cRes);
    env->ReleaseStringUTFChars(params, cParams);
    env->ReleaseStringUTFChars(func, cFunc);
    return jstr;
}

#pragma mark Store.SQLite

jlong NGenXX_JNI_storeSQLiteOpen(JNIEnv *env, jobject thiz,
                                 jstring id)
{
    const char *cId = env->GetStringUTFChars(id, nullptr);
    auto res = (jlong)ngenxx_store_sqlite_open(cId);
    env->ReleaseStringUTFChars(id, cId);
    return res;
}

jboolean NGenXX_JNI_storeSQLiteExecute(JNIEnv *env, jobject thiz,
                                       jlong conn,
                                       jstring sql)
{
    const char *cSql = env->GetStringUTFChars(sql, nullptr);
    jboolean res = ngenxx_store_sqlite_execute((void *)conn, cSql);
    env->ReleaseStringUTFChars(sql, cSql);
    return res;
}

jlong NGenXX_JNI_storeSQLiteQueryDo(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring sql)
{
    const char *cSql = env->GetStringUTFChars(sql, nullptr);
    auto res = (jlong)ngenxx_store_sqlite_query_do((void *)conn, cSql);
    env->ReleaseStringUTFChars(sql, cSql);
    return res;
}

jboolean NGenXX_JNI_storeSQLiteQueryReadRow(JNIEnv *env, jobject thiz,
                                            jlong query_result)
{
    jboolean res = ngenxx_store_sqlite_query_read_row((void *)query_result);
    return res;
}

jstring NGenXX_JNI_storeSQLiteQueryReadColumnText(JNIEnv *env, jobject thiz,
                                                  jlong query_result,
                                                  jstring column)
{
    const char *cColumn = env->GetStringUTFChars(column, nullptr);
    const char *cRes = ngenxx_store_sqlite_query_read_column_text((void *)query_result, cColumn);
    jstring jstr = env->NewStringUTF(cRes ?: "");
    free((void *)cRes);
    env->ReleaseStringUTFChars(column, cColumn);
    return jstr;
}

jlong NGenXX_JNI_storeSQLiteQueryReadColumnInteger(JNIEnv *env, jobject thiz,
                                                   jlong query_result,
                                                   jstring column)
{
    const char *cColumn = env->GetStringUTFChars(column, nullptr);
    jlong res = ngenxx_store_sqlite_query_read_column_integer((void *)query_result, cColumn);
    env->ReleaseStringUTFChars(column, cColumn);
    return res;
}

jdouble NGenXX_JNI_storeSQLiteQueryReadColumnFloat(JNIEnv *env, jobject thiz,
                                                   jlong query_result,
                                                   jstring column)
{
    const char *cColumn = env->GetStringUTFChars(column, nullptr);
    jdouble res = ngenxx_store_sqlite_query_read_column_float((void *)query_result, cColumn);
    env->ReleaseStringUTFChars(column, cColumn);
    return res;
}

void NGenXX_JNI_storeSQLiteQueryDrop(JNIEnv *env, jobject thiz,
                                     jlong query_result)
{
    ngenxx_store_sqlite_query_drop((void *)query_result);
}

void NGenXX_JNI_storeSQLiteClose(JNIEnv *env, jobject thiz,
                                 jlong conn)
{
    ngenxx_store_sqlite_close((void *)conn);
}

#pragma mark Srore.KV

jlong NGenXX_JNI_storeKVOpen(JNIEnv *env, jobject thiz,
                             jstring id)
{
    const char *cId = env->GetStringUTFChars(id, nullptr);
    auto res = (jlong)ngenxx_store_kv_open(cId);
    env->ReleaseStringUTFChars(id, cId);
    return res;
}

jstring NGenXX_JNI_storeKVReadString(JNIEnv *env, jobject thiz,
                                     jlong conn,
                                     jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    const char *cRes = ngenxx_store_kv_read_string((void *)conn, cK);
    jstring jstr = env->NewStringUTF(cRes ?: "");
    free((void *)cRes);
    env->ReleaseStringUTFChars(k, cK);
    return jstr;
}

jboolean NGenXX_JNI_storeKVWriteString(JNIEnv *env, jobject thiz,
                                       jlong conn,
                                       jstring k, jstring v)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    const char *cV = env->GetStringUTFChars(v, nullptr);
    jboolean res = ngenxx_store_kv_write_string((void *)conn, cK, cV);
    env->ReleaseStringUTFChars(v, cV);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jlong NGenXX_JNI_storeKVReadInteger(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jlong res = ngenxx_store_kv_read_integer((void *)conn, cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jboolean NGenXX_JNI_storeKVWriteInteger(JNIEnv *env, jobject thiz,
                                        jlong conn,
                                        jstring k, jlong v)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jboolean res = ngenxx_store_kv_write_integer((void *)conn, cK, v);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jdouble NGenXX_JNI_storeKVReadFloat(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jdouble res = ngenxx_store_kv_read_float((void *)conn, cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jboolean NGenXX_JNI_storeKVWriteFloat(JNIEnv *env, jobject thiz,
                                      jlong conn,
                                      jstring k, jdouble v)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jboolean res = ngenxx_store_kv_write_float((void *)conn, cK, v);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jboolean NGenXX_JNI_storeKVContains(JNIEnv *env, jobject thiz,
                                    jlong conn,
                                    jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jboolean res = ngenxx_store_kv_contains((void *)conn, cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

void NGenXX_JNI_storeKVClear(JNIEnv *env, jobject thiz,
                             jlong conn)
{
    ngenxx_store_kv_clear((void *)conn);
}

void NGenXX_JNI_storeKVClose(JNIEnv *env, jobject thiz,
                             jlong conn)
{
    ngenxx_store_kv_close((void *)conn);
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
    free((void *)cDN);
    return jstr;
}

jstring NGenXX_JNI_deviceManufacturer(JNIEnv *env, jobject thiz)
{
    const char *cDM = ngenxx_device_manufacturer();
    jstring jstr = env->NewStringUTF(cDM ?: "");
    free((void *)cDM);
    return jstr;
}

jstring NGenXX_JNI_deviceOsVersion(JNIEnv *env, jobject thiz)
{
    const char *cDOV = ngenxx_device_os_version();
    jstring jstr = env->NewStringUTF(cDOV ?: "");
    free((void *)cDOV);
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
    size inLen = env->GetArrayLength(bytes);

    auto cRes = ngenxx_coding_hex_bytes2str((const byte *)cIn, inLen);
    jstring jstr = env->NewStringUTF(cRes ?: "");

    free((void *)cRes);
    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return jstr;
}

jbyteArray NGenXX_JNI_codingHexStr2Bytes(JNIEnv *env,
                                         jobject thiz,
                                         jstring str)
{
    const char *cStr = env->GetStringUTFChars(str, nullptr);

    size outLen;
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
    return moveToJByteArray(env, out, len, false);
}

jbyteArray NGenXX_JNI_cryptoAesEncrypt(JNIEnv *env, jobject thiz,
                                       jbyteArray input,
                                       jbyteArray key)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size inLen = env->GetArrayLength(input);
    jbyte *cKey = env->GetByteArrayElements(key, nullptr);
    size keyLen = env->GetArrayLength(key);

    size outLen;
    auto cRes = ngenxx_crypto_aes_encrypt((const byte *)cIn, inLen,
                                          (const byte *)cKey, keyLen,
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
    size inLen = env->GetArrayLength(input);
    jbyte *cKey = env->GetByteArrayElements(key, nullptr);
    size keyLen = env->GetArrayLength(key);

    size outLen;
    auto cRes = ngenxx_crypto_aes_decrypt((const byte *)cIn, inLen,
                                          (const byte *)cKey, keyLen,
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
    size inLen = env->GetArrayLength(input);
    jbyte *cKey = env->GetByteArrayElements(key, nullptr);
    size keyLen = env->GetArrayLength(key);
    jbyte *cIv = env->GetByteArrayElements(init_vector, nullptr);
    size ivLen = env->GetArrayLength(init_vector);
    jbyte *cAad = aad ? env->GetByteArrayElements(aad, nullptr) : nullptr;
    int aadLen = aad ? env->GetArrayLength(aad) : 0;

    size outLen;
    auto cRes = ngenxx_crypto_aes_gcm_encrypt((const byte *)cIn, inLen,
                                              (const byte *)cKey, keyLen,
                                              (const byte *)cIv, ivLen,
                                              (const byte *)cAad, aadLen,
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
    size inLen = env->GetArrayLength(input);
    jbyte *cKey = env->GetByteArrayElements(key, nullptr);
    size keyLen = env->GetArrayLength(key);
    jbyte *cIv = env->GetByteArrayElements(init_vector, nullptr);
    size ivLen = env->GetArrayLength(init_vector);
    jbyte *cAad = aad ? env->GetByteArrayElements(aad, nullptr) : nullptr;
    int aadLen = aad ? env->GetArrayLength(aad) : 0;

    size outLen;
    auto cRes = ngenxx_crypto_aes_gcm_decrypt((const byte *)cIn, inLen,
                                              (const byte *)cKey, keyLen,
                                              (const byte *)cIv, ivLen,
                                              (const byte *)cAad, aadLen,
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
    size inLen = env->GetArrayLength(input);

    size outLen;
    auto cRes = ngenxx_crypto_hash_md5((const byte *)cIn, inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_cryptoHashSha256(JNIEnv *env, jobject thiz,
                                       jbyteArray input)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size inLen = env->GetArrayLength(input);

    size outLen;
    auto cRes = ngenxx_crypto_hash_sha256((const byte *)cIn, inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_cryptoBase64Encode(JNIEnv *env, jobject thiz,
                                         jbyteArray input)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size inLen = env->GetArrayLength(input);

    size outLen;
    auto cRes = ngenxx_crypto_base64_encode((const byte *)cIn, inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_cryptoBase64Decode(JNIEnv *env, jobject thiz,
                                         jbyteArray input)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size inLen = env->GetArrayLength(input);

    size outLen;
    auto cRes = ngenxx_crypto_base64_decode((const byte *)cIn, inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

#pragma mark JsonDecoder

jlong NGenXX_JNI_jsonDecoderInit(JNIEnv *env, jobject thiz,
                                 jstring json)
{
    const char *cJson = env->GetStringUTFChars(json, nullptr);
    auto res = (jlong)ngenxx_json_decoder_init(cJson);
    env->ReleaseStringUTFChars(json, cJson);
    return res;
}

jboolean NGenXX_JNI_jsonDecoderIsArray(JNIEnv *env, jobject thiz,
                                       jlong decoder,
                                       jlong node)
{
    return ngenxx_json_decoder_is_array((void *)decoder, (void *)node);
}

jboolean NGenXX_JNI_jsonDecoderIsObject(JNIEnv *env, jobject thiz,
                                        jlong decoder,
                                        jlong node)
{
    return ngenxx_json_decoder_is_object((void *)decoder, (void *)node);
}

jlong NGenXX_JNI_jsonDecoderReadNode(JNIEnv *env, jobject thiz,
                                     jlong decoder,
                                     jlong node,
                                     jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    auto res = (jlong)ngenxx_json_decoder_read_node((void *)decoder, (void *)node, cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

jstring NGenXX_JNI_jsonDecoderReadString(JNIEnv *env, jobject thiz,
                                         jlong decoder,
                                         jlong node)
{
    const char *res = ngenxx_json_decoder_read_string((void *)decoder, (void *)node);
    jstring jstr = env->NewStringUTF(res ?: "");
    free((void *)res);
    return jstr;
}

jdouble NGenXX_JNI_jsonDecoderReadNumber(JNIEnv *env, jobject thiz,
                                         jlong decoder,
                                         jlong node)
{
    return ngenxx_json_decoder_read_number((void *)decoder, (void *)node);
}

jlong NGenXX_JNI_jsonDecoderReadChild(JNIEnv *env, jobject thiz,
                                      jlong decoder,
                                      jlong node)
{
    return (jlong)ngenxx_json_decoder_read_child((void *)decoder, (void *)node);
}

jlong NGenXX_JNI_jsonDecoderReadNext(JNIEnv *env, jobject thiz,
                                     jlong decoder,
                                     jlong node)
{
    return (jlong)ngenxx_json_decoder_read_next((void *)decoder, (void *)node);
}

void NGenXX_JNI_jsonDecoderRelease(JNIEnv *env, jobject thiz,
                                   jlong decoder)
{
    ngenxx_json_decoder_release((void *)decoder);
}

#pragma mark Zip

jlong NGenXX_JNI_zZipInit(JNIEnv *env, jobject thiz,
                          jint mode, jlong bufferSize)
{
    return (long)ngenxx_z_zip_init(mode, bufferSize);
}

jlong NGenXX_JNI_zZipInput(JNIEnv *env, jobject thiz,
                           jlong zip,
                           jbyteArray inBytes, jint inLen, jboolean inFinish)
{
    jbyte *cIn = env->GetByteArrayElements(inBytes, nullptr);
    // size inLen = env->GetArrayLength(input);

    size ret = ngenxx_z_zip_input((void *)zip, (const byte *)cIn, inLen, inFinish);

    env->ReleaseByteArrayElements(inBytes, cIn, JNI_ABORT);

    return ret;
}

jbyteArray NGenXX_JNI_zZipProcessDo(JNIEnv *env, jobject thiz, jlong zip)
{
    size outLen = 0;
    auto cRes = ngenxx_z_zip_process_do((void *)zip, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    return jba;
}

jboolean NGenXX_JNI_zZipProcessFinished(JNIEnv *env, jobject thiz,
                                        jlong zip)
{
    return ngenxx_z_zip_process_finished((void *)zip);
}

void NGenXX_JNI_zZipRelease(JNIEnv *env, jobject thiz,
                            jlong zip)
{
    ngenxx_z_zip_release((void *)zip);
}

jlong NGenXX_JNI_zUnZipInit(JNIEnv *env, jobject thiz,
                            jlong bufferSize)
{
    return (long)ngenxx_z_unzip_init(bufferSize);
}

jlong NGenXX_JNI_zUnZipInput(JNIEnv *env, jobject thiz,
                             jlong unzip,
                             jbyteArray inBytes, jint inLen, jboolean inFinish)
{
    jbyte *cIn = env->GetByteArrayElements(inBytes, nullptr);
    // size inLen = env->GetArrayLength(input);

    size ret = ngenxx_z_unzip_input((void *)unzip, (const byte *)cIn, inLen, inFinish);

    env->ReleaseByteArrayElements(inBytes, cIn, JNI_ABORT);

    return ret;
}

jbyteArray NGenXX_JNI_zUnZipProcessDo(JNIEnv *env, jobject thiz, jlong unzip)
{
    size outLen = 0;
    auto cRes = ngenxx_z_unzip_process_do((void *)unzip, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    return jba;
}

jboolean NGenXX_JNI_zUnZipProcessFinished(JNIEnv *env, jobject thiz,
                                          jlong unzip)
{
    return ngenxx_z_unzip_process_finished((void *)unzip);
}

void NGenXX_JNI_zUnZipRelease(JNIEnv *env, jobject thiz,
                              jlong zip)
{
    ngenxx_z_unzip_release((void *)zip);
}

jbyteArray NGenXX_JNI_zZipBytes(JNIEnv *env, jobject thiz,
                                jint mode,
                                jlong buffer_size, jbyteArray bytes)
{
    jbyte *cIn = env->GetByteArrayElements(bytes, nullptr);
    size inLen = env->GetArrayLength(bytes);

    size outLen;
    auto cRes = ngenxx_z_bytes_zip(mode, buffer_size, (const byte *)cIn, inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return jba;
}

jbyteArray NGenXX_JNI_zUnZipBytes(JNIEnv *env, jobject thiz,
                                  jlong buffer_size, jbyteArray bytes)
{
    jbyte *cIn = env->GetByteArrayElements(bytes, nullptr);
    size inLen = env->GetArrayLength(bytes);

    size outLen;
    auto cRes = ngenxx_z_bytes_unzip(buffer_size, (const byte *)cIn, inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen, true);

    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return jba;
}

static const char *JClassName = "xyz/rinc/ngenxx/NGenXX$Companion";

static const JNINativeMethod JMethods[] = {
    {"getVersion", "()Ljava/lang/String;", (void *)NGenXX_JNI_getVersion},
    {"init", "(Ljava/lang/String;)Z", (void *)NGenXX_JNI_init},
    {"release", "()V", (void *)NGenXX_JNI_release},

    {"logSetLevel", "(I)V", (void *)NGenXX_JNI_logSetLevel},
    {"logSetCallback", "(Lkotlin/jvm/functions/Function2;)V", (void *)NGenXX_JNI_logSetCallback},
    {"logPrint", "(ILjava/lang/String;)V", (void *)NGenXX_JNI_logPrint},

    {"netHttpRequest", "(Ljava/lang/String;Ljava/lang/String;I[Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;JJ)Ljava/lang/String;", (void *)NGenXX_JNI_netHttpRequest},

    {"lLoadF", "(Ljava/lang/String;)Z", (void *)NGenXX_JNI_lLoadF},
    {"lLoadS", "(Ljava/lang/String;)Z", (void *)NGenXX_JNI_lLoadS},
    {"lCall", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", (void *)NGenXX_JNI_lCall},

    {"jLoadF", "(Ljava/lang/String;)Z", (void *)NGenXX_JNI_jLoadF},
    {"jLoadS", "(Ljava/lang/String;Ljava/lang/String;)Z", (void *)NGenXX_JNI_jLoadS},
    {"jLoadB", "([B)Z", (void *)NGenXX_JNI_jLoadB},
    {"jCall", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", (void *)NGenXX_JNI_jCall},

    {"storeSQLiteOpen", "(Ljava/lang/String;)J", (void *)NGenXX_JNI_storeSQLiteOpen},
    {"storeSQLiteExecute", "(JLjava/lang/String;)Z", (void *)NGenXX_JNI_storeSQLiteExecute},
    {"storeSQLiteQueryDo", "(JLjava/lang/String;)J", (void *)NGenXX_JNI_storeSQLiteQueryDo},
    {"storeSQLiteQueryReadRow", "(J)Z", (void *)NGenXX_JNI_storeSQLiteQueryReadRow},
    {"storeSQLiteQueryReadColumnText", "(JLjava/lang/String;)Ljava/lang/String;", (void *)NGenXX_JNI_storeSQLiteQueryReadColumnText},
    {"storeSQLiteQueryReadColumnInteger", "(JLjava/lang/String;)J", (void *)NGenXX_JNI_storeSQLiteQueryReadColumnInteger},
    {"storeSQLiteQueryReadColumnFloat", "(JLjava/lang/String;)D", (void *)NGenXX_JNI_storeSQLiteQueryReadColumnFloat},
    {"storeSQLiteQueryDrop", "(J)V", (void *)NGenXX_JNI_storeSQLiteQueryDrop},
    {"storeSQLiteClose", "(J)V", (void *)NGenXX_JNI_storeSQLiteClose},

    {"storeKVOpen", "(Ljava/lang/String;)J", (void *)NGenXX_JNI_storeKVOpen},
    {"storeKVReadString", "(JLjava/lang/String;)Ljava/lang/String;", (void *)NGenXX_JNI_storeKVReadString},
    {"storeKVWriteString", "(JLjava/lang/String;Ljava/lang/String;)Z", (void *)NGenXX_JNI_storeKVWriteString},
    {"storeKVReadInteger", "(JLjava/lang/String;)J", (void *)NGenXX_JNI_storeKVReadInteger},
    {"storeKVWriteInteger", "(JLjava/lang/String;J)Z", (void *)NGenXX_JNI_storeKVWriteInteger},
    {"storeKVReadFloat", "(JLjava/lang/String;)D", (void *)NGenXX_JNI_storeKVReadFloat},
    {"storeKVWriteFloat", "(JLjava/lang/String;D)Z", (void *)NGenXX_JNI_storeKVWriteFloat},
    {"storeKVContains", "(JLjava/lang/String;)Z", (void *)NGenXX_JNI_storeKVContains},
    {"storeKVClear", "(J)V", (void *)NGenXX_JNI_storeKVClear},
    {"storeKVClose", "(J)V", (void *)NGenXX_JNI_storeKVClose},

    {"deviceType", "()I", (void *)NGenXX_JNI_deviceType},
    {"deviceName", "()Ljava/lang/String;", (void *)NGenXX_JNI_deviceName},
    {"deviceManufacturer", "()Ljava/lang/String;", (void *)NGenXX_JNI_deviceManufacturer},
    {"deviceOsVersion", "()Ljava/lang/String;", (void *)NGenXX_JNI_deviceOsVersion},
    {"deviceCpuArch", "()I", (void *)NGenXX_JNI_deviceCpuArch},

    {"codingHexBytes2Str", "([B)Ljava/lang/String;", (void *)NGenXX_JNI_codingHexBytes2Str},
    {"codingHexStr2Bytes", "(Ljava/lang/String;)[B", (void *)NGenXX_JNI_codingHexStr2Bytes},

    {"cryptoRandom", "(I)[B", (void *)NGenXX_JNI_cryptoRandom},
    {"cryptoAesEncrypt", "([B[B)[B", (void *)NGenXX_JNI_cryptoAesEncrypt},
    {"cryptoAesDecrypt", "([B[B)[B", (void *)NGenXX_JNI_cryptoAesDecrypt},
    {"cryptoAesGcmEncrypt", "([B[B[B[BI)[B", (void *)NGenXX_JNI_cryptoAesGcmEncrypt},
    {"cryptoAesGcmDecrypt", "([B[B[B[BI)[B", (void *)NGenXX_JNI_cryptoAesGcmDecrypt},
    {"cryptoHashMd5", "([B)[B", (void *)NGenXX_JNI_cryptoHashMd5},
    {"cryptoHashSha256", "([B)[B", (void *)NGenXX_JNI_cryptoHashSha256},
    {"cryptoBase64Encode", "([B)[B", (void *)NGenXX_JNI_cryptoBase64Encode},
    {"cryptoBase64Decode", "([B)[B", (void *)NGenXX_JNI_cryptoBase64Decode},

    {"jsonDecoderInit", "(Ljava/lang/String;)J", (void *)NGenXX_JNI_jsonDecoderInit},
    {"jsonDecoderIsArray", "(JJ)Z", (void *)NGenXX_JNI_jsonDecoderIsArray},
    {"jsonDecoderIsObject", "(JJ)Z", (void *)NGenXX_JNI_jsonDecoderIsObject},
    {"jsonDecoderReadNode", "(JJLjava/lang/String;)J", (void *)NGenXX_JNI_jsonDecoderReadNode},
    {"jsonDecoderReadString", "(JJ)Ljava/lang/String;", (void *)NGenXX_JNI_jsonDecoderReadString},
    {"jsonDecoderReadNumber", "(JJ)D", (void *)NGenXX_JNI_jsonDecoderReadNumber},
    {"jsonDecoderReadChild", "(JJ)J", (void *)NGenXX_JNI_jsonDecoderReadChild},
    {"jsonDecoderReadNext", "(JJ)J", (void *)NGenXX_JNI_jsonDecoderReadNext},
    {"jsonDecoderRelease", "(J)V", (void *)NGenXX_JNI_jsonDecoderRelease},

    {"zZipInit", "(IJ)J", (void *)NGenXX_JNI_zZipInit},
    {"zZipInput", "(J[BIZ)J", (void *)NGenXX_JNI_zZipInput},
    {"zZipProcessDo", "(J)[B", (void *)NGenXX_JNI_zZipProcessDo},
    {"zZipProcessFinished", "(J)Z", (void *)NGenXX_JNI_zZipProcessFinished},
    {"zZipRelease", "(J)V", (void *)NGenXX_JNI_zZipRelease},
    {"zUnZipInit", "(J)J", (void *)NGenXX_JNI_zUnZipInit},
    {"zUnZipInput", "(J[BIZ)J", (void *)NGenXX_JNI_zUnZipInput},
    {"zUnZipProcessDo", "(J)[B", (void *)NGenXX_JNI_zUnZipProcessDo},
    {"zUnZipProcessFinished", "(J)Z", (void *)NGenXX_JNI_zUnZipProcessFinished},
    {"zUnZipRelease", "(J)V", (void *)NGenXX_JNI_zUnZipRelease},
    {"zZipBytes", "(IJ[B)[B", (void *)NGenXX_JNI_zZipBytes},
    {"zUnZipBytes", "(J[B)[B", (void *)NGenXX_JNI_zUnZipBytes},
};

int JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv *env = nullptr;
    int registerResult = vm->GetEnv((void **)&env, JNI_VERSION_1_6);
    if (registerResult != JNI_OK)
        return JNI_ERR;
    jclass jclazz = env->FindClass(JClassName);
    env->RegisterNatives(jclazz, JMethods, sizeof(JMethods) / sizeof(JNINativeMethod));
    return JNI_VERSION_1_6;
}