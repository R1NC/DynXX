#include <jni.h>
#include <cstdlib>
#include <string>
#include "../../../../../../build.Android/output/include/NGenXX.h"
#include "JNIUtil.hxx"

extern "C" JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_getVersion(JNIEnv *env,
                                                      jobject thiz)
{
    const char *cV = ngenxx_get_version();
    jstring jstr = env->NewStringUTF(cV ?: "");
    free((void *)cV);
    return jstr;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_init(JNIEnv *env, jobject thiz, jstring root)
{
    const char *cRoot = env->GetStringUTFChars(root, nullptr);
    jboolean res = ngenxx_init(cRoot);
    env->ReleaseStringUTFChars(root, cRoot);
    return res;
}

extern "C" JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_release(JNIEnv *env, jobject thiz)
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

extern "C" JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_logSetLevel(JNIEnv *env, jobject thiz,
                                                       jint level)
{
    ngenxx_log_set_level(level);
}

extern "C" JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_logSetCallback(JNIEnv *env, jobject thiz,
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

extern "C" JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_logPrint(JNIEnv *env, jobject thiz,
                                                    jint level, jstring content)
{
    const char *cContent = env->GetStringUTFChars(content, nullptr);
    ngenxx_log_print(level, cContent);
    env->ReleaseStringUTFChars(content, cContent);
}

#pragma mark Net

extern "C" JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_netHttpRequest(JNIEnv *env, jobject thiz,
                                                          jstring url, jstring params,
                                                          jint method,
                                                          jobjectArray headers,
                                                          jlong timeout)
{
    const char *cUrl = env->GetStringUTFChars(url, nullptr);
    const char *cParams = env->GetStringUTFChars(params, nullptr);

    const unsigned int headersCount = env->GetArrayLength(headers);
    char **cHeadersV = (char **)malloc(headersCount * sizeof(char *));
    auto *jstrV = (jstring *)malloc(sizeof(jstring));
    for (int i = 0; i < headersCount; i++)
    {
        jstrV[i] = (jstring)(env->GetObjectArrayElement(headers, i));
        cHeadersV[i] = (char *)env->GetStringUTFChars(jstrV[i], nullptr);
    }

    const char *cRsp = ngenxx_net_http_request(cUrl, cParams, (const int)method, (const char **)cHeadersV, headersCount, (const unsigned long)timeout);
    jstring jstr = env->NewStringUTF(cRsp ?: "");
    free((void *)cRsp);

    for (int i = 0; i < headersCount; i++)
    {
        env->ReleaseStringUTFChars(jstrV[i], cHeadersV[i]);
    }
    free((void *)jstrV);
    free((void *)cHeadersV);

    env->ReleaseStringUTFChars(params, cParams);
    env->ReleaseStringUTFChars(url, cUrl);

    return jstr;
}

#pragma mark Lua

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_lLoadF(JNIEnv *env, jobject thiz,
                                                  jstring file)
{
    const char *cFile = env->GetStringUTFChars(file, nullptr);
    jboolean res = ngenxx_L_loadF(cFile);
    env->ReleaseStringUTFChars(file, cFile);
    return res;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_lLoadS(JNIEnv *env, jobject thiz,
                                                  jstring script)
{
    const char *cScript = env->GetStringUTFChars(script, nullptr);
    jboolean res = ngenxx_L_loadS(cScript);
    env->ReleaseStringUTFChars(script, cScript);
    return res;
}

extern "C" JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_lCall(JNIEnv *env, jobject thiz,
                                                 jstring func,
                                                 jstring params)
{
    const char *cFunc = env->GetStringUTFChars(func, nullptr);
    const char *cParams = params ? env->GetStringUTFChars(params, JNI_FALSE) : nullptr;
    const char *cRes = ngenxx_L_call(cFunc, cParams);
    jstring jstr = env->NewStringUTF(cRes ?: "");
    free((void *)cRes);
    env->ReleaseStringUTFChars(params, cParams);
    env->ReleaseStringUTFChars(func, cFunc);
    return jstr;
}

#pragma mark Store.SQLite

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeSQLiteOpen(JNIEnv *env, jobject thiz,
                                                           jstring id)
{
    const char *cId = env->GetStringUTFChars(id, nullptr);
    auto res = (jlong)ngenxx_store_sqlite_open(cId);
    env->ReleaseStringUTFChars(id, cId);
    return res;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeSQLiteExecute(JNIEnv *env, jobject thiz,
                                                              jlong conn,
                                                              jstring sql)
{
    const char *cSql = env->GetStringUTFChars(sql, nullptr);
    jboolean res = ngenxx_store_sqlite_execute((void *)conn, cSql);
    env->ReleaseStringUTFChars(sql, cSql);
    return res;
}

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeSQLiteQueryDo(JNIEnv *env, jobject thiz,
                                                              jlong conn,
                                                              jstring sql)
{
    const char *cSql = env->GetStringUTFChars(sql, nullptr);
    auto res = (jlong)ngenxx_store_sqlite_query_do((void *)conn, cSql);
    env->ReleaseStringUTFChars(sql, cSql);
    return res;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeSQLiteQueryReadRow(JNIEnv *env, jobject thiz,
                                                                   jlong query_result)
{
    jboolean res = ngenxx_store_sqlite_query_read_row((void *)query_result);
    return res;
}

extern "C" JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeSQLiteQueryReadColumnText(JNIEnv *env, jobject thiz,
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

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeSQLiteQueryReadColumnInteger(JNIEnv *env, jobject thiz,
                                                                             jlong query_result, jstring column)
{
    const char *cColumn = env->GetStringUTFChars(column, nullptr);
    jlong res = ngenxx_store_sqlite_query_read_column_integer((void *)query_result, cColumn);
    env->ReleaseStringUTFChars(column, cColumn);
    return res;
}

extern "C" JNIEXPORT jdouble JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeSQLiteQueryReadColumnFloat(JNIEnv *env, jobject thiz,
                                                                           jlong query_result, jstring column)
{
    const char *cColumn = env->GetStringUTFChars(column, nullptr);
    jdouble res = ngenxx_store_sqlite_query_read_column_float((void *)query_result, cColumn);
    env->ReleaseStringUTFChars(column, cColumn);
    return res;
}

extern "C" JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeSQLiteQueryDrop(JNIEnv *env, jobject thiz,
                                                                jlong query_result)
{
    ngenxx_store_sqlite_query_drop((void *)query_result);
}

extern "C" JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeSQLiteClose(JNIEnv *env, jobject thiz,
                                                            jlong conn)
{
    ngenxx_store_sqlite_close((void *)conn);
}

#pragma mark Srore.KV

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeKVOpen(JNIEnv *env, jobject thiz,
                                                       jstring id)
{
    const char *cId = env->GetStringUTFChars(id, nullptr);
    auto res = (jlong)ngenxx_store_kv_open(cId);
    env->ReleaseStringUTFChars(id, cId);
    return res;
}

extern "C" JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeKVReadString(JNIEnv *env, jobject thiz,
                                                             jlong conn, jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    const char *cRes = ngenxx_store_kv_read_string((void *)conn, cK);
    jstring jstr = env->NewStringUTF(cRes ?: "");
    free((void *)cRes);
    env->ReleaseStringUTFChars(k, cK);
    return jstr;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeKVWriteString(JNIEnv *env, jobject thiz,
                                                              jlong conn, jstring k,
                                                              jstring v)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    const char *cV = env->GetStringUTFChars(v, nullptr);
    jboolean res = ngenxx_store_kv_write_string((void *)conn, cK, cV);
    env->ReleaseStringUTFChars(v, cV);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeKVReadInteger(JNIEnv *env, jobject thiz,
                                                              jlong conn, jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jlong res = ngenxx_store_kv_read_integer((void *)conn, cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeKVWriteInteger(JNIEnv *env, jobject thiz,
                                                               jlong conn, jstring k,
                                                               jlong v)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jboolean res = ngenxx_store_kv_write_integer((void *)conn, cK, v);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

extern "C" JNIEXPORT jdouble JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeKVReadFloat(JNIEnv *env, jobject thiz,
                                                            jlong conn, jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jdouble res = ngenxx_store_kv_read_float((void *)conn, cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeKVWriteFloat(JNIEnv *env, jobject thiz,
                                                             jlong conn, jstring k, jdouble v)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jboolean res = ngenxx_store_kv_write_float((void *)conn, cK, v);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeKVContains(JNIEnv *env, jobject thiz,
                                                           jlong conn, jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    jboolean res = ngenxx_store_kv_contains((void *)conn, cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

extern "C" JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeKVClear(JNIEnv *env, jobject thiz,
                                                        jlong conn)
{
    ngenxx_store_kv_clear((void *)conn);
}

extern "C" JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_storeKVClose(JNIEnv *env, jobject thiz,
                                                        jlong conn)
{
    ngenxx_store_kv_close((void *)conn);
}

#pragma mark Device

extern "C" JNIEXPORT jint JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_deviceType(JNIEnv *env, jobject thiz)
{
    return ngenxx_device_type();
}

extern "C" JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_deviceName(JNIEnv *env, jobject thiz)
{
    const char *cDN = ngenxx_device_name();
    jstring jstr = env->NewStringUTF(cDN ?: "");
    free((void *)cDN);
    return jstr;
}

extern "C" JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_deviceManufacturer(JNIEnv *env, jobject thiz)
{
    const char *cDM = ngenxx_device_manufacturer();
    jstring jstr = env->NewStringUTF(cDM ?: "");
    free((void *)cDM);
    return jstr;
}

extern "C" JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_deviceOsVersion(JNIEnv *env, jobject thiz)
{
    const char *cDOV = ngenxx_device_os_version();
    jstring jstr = env->NewStringUTF(cDOV ?: "");
    free((void *)cDOV);
    return jstr;
}

extern "C" JNIEXPORT jint JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_deviceCpuArch(JNIEnv *env, jobject thiz)
{
    return ngenxx_device_cpu_arch();
}

#pragma mark Coding

extern "C" JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_codingHexBytes2Str(JNIEnv *env, jobject thiz,
                                                                   jbyteArray bytes) {
    jbyte *cIn = env->GetByteArrayElements(bytes, nullptr);
    size inLen = env->GetArrayLength(bytes);

    auto cRes = ngenxx_coding_hex_bytes2str((const byte *)cIn, inLen);
    jstring jstr = env->NewStringUTF(cRes ?: "");

    free((void *)cRes);
    env->ReleaseByteArrayElements(bytes, cIn, JNI_ABORT);
    return jstr;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_codingHexStr2Bytes(JNIEnv *env,
                                                                                   jobject thiz,
                                                                                   jstring str) {
    const char *cStr = env->GetStringUTFChars(str, nullptr);

    size outLen;
    auto cRes = ngenxx_coding_hex_str2bytes(cStr, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen);

    env->ReleaseStringUTFChars(str, cStr);
    return jba;
}

#pragma mark Crypto

extern "C" JNIEXPORT jbyteArray JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_cryptoAesEncrypt(JNIEnv *env, jobject thiz,
                                                            jbyteArray input,
                                                            jbyteArray key)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size inLen = env->GetArrayLength(input);
    jbyte *cKey = env->GetByteArrayElements(key, nullptr);
    size keyLen = env->GetArrayLength(key);

    size outLen;
    auto cRes = ngenxx_crypto_aes_encrypt((const byte *)cIn, inLen, (const byte *)cKey, keyLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
    return jba;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_cryptoAesDecrypt(JNIEnv *env, jobject thiz,
                                                            jbyteArray input,
                                                            jbyteArray key)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size inLen = env->GetArrayLength(input);
    jbyte *cKey = env->GetByteArrayElements(key, nullptr);
    size keyLen = env->GetArrayLength(key);

    size outLen;
    auto cRes = ngenxx_crypto_aes_decrypt((const byte *)cIn, inLen, (const byte *)cKey, keyLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    env->ReleaseByteArrayElements(key, cKey, JNI_ABORT);
    return jba;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_cryptoHashMd5(JNIEnv *env, jobject thiz,
                                                         jbyteArray input)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size inLen = env->GetArrayLength(input);

    size outLen;
    auto cRes = ngenxx_crypto_hash_md5((const byte *)cIn, inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_cryptoHashSha256(JNIEnv *env, jobject thiz,
                                                            jbyteArray input)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size inLen = env->GetArrayLength(input);

    size outLen;
    auto cRes = ngenxx_crypto_hash_sha256((const byte *)cIn, inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_cryptoBase64Encode(JNIEnv *env, jobject thiz,
                                                              jbyteArray input)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size inLen = env->GetArrayLength(input);

    size outLen;
    auto cRes = ngenxx_crypto_base64_encode((const byte *)cIn, inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_cryptoBase64Decode(JNIEnv *env, jobject thiz,
                                                              jbyteArray input)
{
    jbyte *cIn = env->GetByteArrayElements(input, nullptr);
    size inLen = env->GetArrayLength(input);

    size outLen;
    auto cRes = ngenxx_crypto_base64_decode((const byte *)cIn, inLen, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen);

    env->ReleaseByteArrayElements(input, cIn, JNI_ABORT);
    return jba;
}

#pragma mark JsonDecoder

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_jsonDecoderInit(JNIEnv *env, jobject thiz,
                                                           jstring json)
{
    const char *cJson = env->GetStringUTFChars(json, nullptr);
    auto res = (jlong)ngenxx_json_decoder_init(cJson);
    env->ReleaseStringUTFChars(json, cJson);
    return res;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_jsonDecoderIsArray(JNIEnv *env, jobject thiz,
                                                              jlong decoder, jlong node)
{
    return ngenxx_json_decoder_is_array((void *)decoder, (void *)node);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_jsonDecoderIsObject(JNIEnv *env, jobject thiz,
                                                               jlong decoder, jlong node)
{
    return ngenxx_json_decoder_is_object((void *)decoder, (void *)node);
}

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_jsonDecoderReadNode(JNIEnv *env, jobject thiz,
                                                               jlong decoder, jlong node,
                                                               jstring k)
{
    const char *cK = env->GetStringUTFChars(k, nullptr);
    auto res = (jlong)ngenxx_json_decoder_read_node((void *)decoder, (void *)node, cK);
    env->ReleaseStringUTFChars(k, cK);
    return res;
}

extern "C" JNIEXPORT jstring JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_jsonDecoderReadString(JNIEnv *env, jobject thiz,
                                                                 jlong decoder, jlong node)
{
    const char *res = ngenxx_json_decoder_read_string((void *)decoder, (void *)node);
    jstring jstr = env->NewStringUTF(res ?: "");
    free((void *)res);
    return jstr;
}

extern "C" JNIEXPORT jdouble JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_jsonDecoderReadNumber(JNIEnv *env, jobject thiz,
                                                                 jlong decoder, jlong node)
{
    return ngenxx_json_decoder_read_number((void *)decoder, (void *)node);
}

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_jsonDecoderReadChild(JNIEnv *env, jobject thiz,
                                                                jlong decoder, jlong node)
{
    return (jlong)ngenxx_json_decoder_read_child((void *)decoder, (void *)node);
}

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_jsonDecoderReadNext(JNIEnv *env, jobject thiz,
                                                               jlong decoder, jlong node)
{
    return (jlong)ngenxx_json_decoder_read_next((void *)decoder, (void *)node);
}

extern "C" JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_jsonDecoderRelease(JNIEnv *env, jobject thiz,
                                                              jlong decoder)
{
    ngenxx_json_decoder_release((void *)decoder);
}

#pragma mark Zip

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_zZipInit(JNIEnv *env, jobject thiz,
                                                    jint mode, jlong bufferSize)
{
    return (long)ngenxx_z_zip_init(mode, bufferSize);
}

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_zZipInput(JNIEnv *env, jobject thiz, jlong zip,
                                                     jbyteArray inBytes, jint inLen, jboolean inFinish)
{
    jbyte *cIn = env->GetByteArrayElements(inBytes, nullptr);
    //size inLen = env->GetArrayLength(input);

    size ret = ngenxx_z_zip_input((void *)zip, (const byte *)cIn, inLen, inFinish);

    env->ReleaseByteArrayElements(inBytes, cIn, JNI_ABORT);

    return ret;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_zZipProcessDo(JNIEnv *env, jobject thiz, jlong zip)
{
    size outLen = 0;
    auto cRes = ngenxx_z_zip_process_do((void *)zip, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen);

    return jba;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_zZipProcessFinished(JNIEnv *env, jobject thiz,
                                                               jlong zip)
{
    return ngenxx_z_zip_process_finished((void *)zip);
}

extern "C" JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_zZipRelease(JNIEnv *env, jobject thiz,
                                                       jlong zip)
{
    ngenxx_z_zip_release((void *)zip);
}

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_zUnZipInit(JNIEnv *env, jobject thiz,
                                                      jlong bufferSize)
{
    return (long)ngenxx_z_unzip_init(bufferSize);
}

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_zUnZipInput(JNIEnv *env, jobject thiz, jlong unzip,
                                                       jbyteArray inBytes, jint inLen, jboolean inFinish)
{
    jbyte *cIn = env->GetByteArrayElements(inBytes, nullptr);
    //size inLen = env->GetArrayLength(input);

    size ret = ngenxx_z_unzip_input((void *)unzip, (const byte *)cIn, inLen, inFinish);

    env->ReleaseByteArrayElements(inBytes, cIn, JNI_ABORT);

    return ret;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_zUnZipProcessDo(JNIEnv *env, jobject thiz, jlong unzip)
{
    size outLen = 0;
    auto cRes = ngenxx_z_unzip_process_do((void *)unzip, &outLen);
    jbyteArray jba = moveToJByteArray(env, cRes, outLen);

    return jba;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_zUnZipProcessFinished(JNIEnv *env, jobject thiz,
                                                                 jlong unzip)
{
    return ngenxx_z_unzip_process_finished((void *)unzip);
}

extern "C" JNIEXPORT void JNICALL
Java_xyz_rinc_ngenxx_NGenXX_00024Companion_zUnZipRelease(JNIEnv *env, jobject thiz,
                                                         jlong zip)
{
    ngenxx_z_unzip_release((void *)zip);
}