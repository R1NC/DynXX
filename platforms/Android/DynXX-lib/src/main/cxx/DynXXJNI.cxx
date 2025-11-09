#include <cstring>

#include "JNIUtil.hxx"
#include <DynXX/C/DynXX.h>
#include <DynXX/CXX/DynXX.hxx>

namespace {
    JavaVM *sVM;
    jclass sJNIClass;
    jobject sLogCallback;
    jmethodID sLogCallbackMethodId;
    jobject sJsMsgCallback;
    jmethodID sJsMsgCallbackMethodId;

    constexpr auto JNI_VER = JNI_VERSION_1_6;
    constexpr auto JNIClassName = "xyz/rinc/dynxx/DynXX$Companion";

#define DECLARE_JNI_FUNC(func, signature) {#func, signature, reinterpret_cast<void *>(func)}

// Engine Callback

    void log_callback(int level, const char *content) {
        if (sVM == nullptr || sLogCallback == nullptr || sLogCallbackMethodId == nullptr ||
            content == nullptr) {
            return;
        }
        const auto env = attachEnv(sVM);
        if (env == nullptr) {
            return;
        }
        auto jContent = boxJString(env, content);
        //env->CallVoidMethod(sLogCallback, sLogCallbackMethodId, level, jContent);
        auto jLevel = boxJInt(env, static_cast<jint>(level));
        auto jRet = env->CallObjectMethod(sLogCallback, sLogCallbackMethodId, jLevel, jContent);
        if (jRet != nullptr) {
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
        const auto env = attachEnv(sVM);
        if (env == nullptr) {
            return nullptr;
        }
        auto jMsg = boxJString(env, msg);
        auto jRes = env->CallObjectMethod(sJsMsgCallback, sJsMsgCallbackMethodId, jMsg);
        auto cRes = readJString(env, jRes);
        auto newRes = dupStr(cRes);
        if (cRes != nullptr) {
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
        auto cRoot = JStringArg(env, root);
        return static_cast<jboolean>(dynxx_init(cRoot.data));
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
        if (callback != nullptr) {
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
        const auto cContent = JStringArg(env, content);
        dynxx_log_print(static_cast<DynXXLogLevel>(level), cContent.data);
    }

// Net

    jstring netHttpRequest(JNIEnv *env, [[maybe_unused]] jobject thiz,
                           jstring url, jstring params, jint method,
                           jobjectArray headerV, jobjectArray formFieldNameV, jobjectArray formFieldMimeV, jobjectArray formFieldDataV,
                           jstring filePath, jlong fileLength,
                           jlong timeout) {
        const auto cUrl = JStringArg(env, url);
        const auto cParams = JStringArg(env, params);

        const auto cHeaders = JStringArrayArg(env, headerV);
        const auto cFormFieldNameV = JStringArrayArg(env, formFieldNameV);
        const auto cFormFieldMimeV = JStringArrayArg(env, formFieldMimeV);
        const auto cFormFieldDataV = JStringArrayArg(env, formFieldDataV);

        const auto cFilePath = filePath != nullptr ? JStringArg(env, filePath).data : "";
        auto cFILE = cFilePath != nullptr ? std::fopen(cFilePath, "r") : nullptr;

        auto cRsp = dynxx_net_http_request(cUrl.data, cParams.data, static_cast<DynXXHttpMethod>(method),
                                            cHeaders.data, cHeaders.size,
                                            cFormFieldNameV.data, cFormFieldMimeV.data, cFormFieldDataV.data, cFormFieldNameV.size,
                                            cFILE, fileLength,
                                            static_cast<const size_t>(timeout));
        auto jStr = boxJString(env, cRsp);

        if (cFILE != nullptr) {
            if (const auto ret = std::fclose(cFILE); ret != 0) {
                dynxxLogPrintF(DynXXLogLevelX::Error, "Failed to close file: {}", ret);
            }
        }

        return jStr;
    }

// Lua

    jboolean lLoadF(JNIEnv *env, [[maybe_unused]] jobject thiz,
                    jstring file) {
        const auto cFile = JStringArg(env, file);
        return static_cast<jboolean>(dynxx_lua_loadF(cFile.data));
    }

    jboolean lLoadS(JNIEnv *env, [[maybe_unused]] jobject thiz,
                    jstring script) {
        const auto cScript = JStringArg(env, script);
        return static_cast<jboolean>(dynxx_lua_loadS(cScript.data));
    }

    jstring lCall(JNIEnv *env, [[maybe_unused]] jobject thiz,
                  jstring func, jstring params) {
        const auto cFunc = JStringArg(env, func);
        const auto cParams = JStringArg(env, params);
        auto cRes = dynxx_lua_call(cFunc.data, cParams.data);
        return boxJString(env, cRes);
    }

// JS

    jboolean jLoadF(JNIEnv *env, [[maybe_unused]] jobject thiz,
                    jstring file, jboolean is_module) {
        const auto cFile = JStringArg(env, file);
        return static_cast<jboolean>(dynxx_js_loadF(cFile.data, is_module == JNI_TRUE));
    }

    jboolean jLoadS(JNIEnv *env, [[maybe_unused]] jobject thiz,
                    jstring script, jstring name, jboolean is_module) {
        const auto cScript = JStringArg(env, script);
        const auto cName = JStringArg(env, name);
        return static_cast<jboolean>(dynxx_js_loadS(cScript.data, cName.data, is_module == JNI_TRUE));
    }

    jboolean jLoadB(JNIEnv *env, [[maybe_unused]] jobject thiz,
                    jbyteArray bytes, jboolean is_module) {
        const auto cIn = JByteArrayArg(env, bytes);
        return static_cast<jboolean>(dynxx_js_loadB(cIn.data, cIn.size, is_module == JNI_TRUE));
    }

    jstring jCall(JNIEnv *env, [[maybe_unused]] jobject thiz,
                  jstring func, jstring params, jboolean await) {
        const auto cFunc = JStringArg(env, func);
        const auto cParams = JStringArg(env, params);
        auto cRes = dynxx_js_call(cFunc.data, cParams.data, await == JNI_TRUE);
        return boxJString(env, cRes);
    }

    void jSetMsgCallback(JNIEnv *env, [[maybe_unused]] jobject thiz,
                         jobject callback) {
        if (callback != nullptr) {
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
        const auto cId = JStringArg(env, id);
        return dynxx_sqlite_open(cId.data);
    }

    jboolean sqliteExecute(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jlong conn, jstring sql) {
        const auto cSql = JStringArg(env, sql);
        return static_cast<jboolean>(dynxx_sqlite_execute(conn, cSql.data));
    }

    jlong sqliteQueryDo(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jlong conn, jstring sql) {
        const auto cSql = JStringArg(env, sql);
        return dynxx_sqlite_query_do(conn, cSql.data);
    }

    jboolean sqliteQueryReadRow([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                                     jlong query_result) {
        return static_cast<jboolean>(dynxx_sqlite_query_read_row(query_result));
    }

    jstring sqliteQueryReadColumnText(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                           jlong query_result, jstring column) {
        const auto cColumn = JStringArg(env, column);
        auto cRes = dynxx_sqlite_query_read_column_text(query_result, cColumn.data);
        return boxJString(env, cRes);
    }

    jlong sqliteQueryReadColumnInteger(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                            jlong query_result, jstring column) {
        const auto cColumn = JStringArg(env, column);
        return dynxx_sqlite_query_read_column_integer(query_result, cColumn.data);
    }

    jdouble sqliteQueryReadColumnFloat(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                       jlong query_result, jstring column) {
        const auto cColumn = JStringArg(env, column);
        return dynxx_sqlite_query_read_column_float(query_result, cColumn.data);
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
        const auto cId = JStringArg(env, id);
        return dynxx_kv_open(cId.data);
    }

    jstring kvReadString(JNIEnv *env, [[maybe_unused]] jobject thiz,
                              jlong conn, jstring k) {
        const auto cK = JStringArg(env, k);
        auto cRes = dynxx_kv_read_string(conn, cK.data);
        return boxJString(env, cRes);
    }

    jboolean kvWriteString(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jlong conn, jstring k, jstring v) {
        const auto cK = JStringArg(env, k);
        const auto cV = JStringArg(env, v);
        return static_cast<jboolean>(dynxx_kv_write_string(conn, cK.data, cV.data));
    }

    jlong kvReadInteger(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jlong conn, jstring k) {
        const auto cK = JStringArg(env, k);
        return dynxx_kv_read_integer(conn, cK.data);
    }

    jboolean kvWriteInteger(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                 jlong conn, jstring k, jlong v) {
        const auto cK = JStringArg(env, k);
        return static_cast<jboolean>(dynxx_kv_write_integer(conn, cK.data, v));
    }

    jdouble kvReadFloat(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jlong conn, jstring k) {
        const auto cK = JStringArg(env, k);
        return dynxx_kv_read_float(conn, cK.data);
    }

    jboolean kvWriteFloat(JNIEnv *env, [[maybe_unused]] jobject thiz,
                               jlong conn, jstring k, jdouble v) {
        const auto cK = JStringArg(env, k);
        return static_cast<jboolean>(dynxx_kv_write_float(conn, cK.data, v));
    }

    jobjectArray kvAllKeys(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jlong conn) {
        size_t len{0};
        auto cRes = dynxx_kv_all_keys(conn, &len);
        return moveToJStringArray(env, cRes, len);
    }

    jboolean kvContains(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jlong conn, jstring k) {
        const auto cK = JStringArg(env, k);
        return static_cast<jboolean>(dynxx_kv_contains(conn, cK.data));
    }

    jboolean kvRemove(JNIEnv *env, [[maybe_unused]] jobject thiz,
                           jlong conn, jstring k) {
        const auto cK = JStringArg(env, k);
        return static_cast<jboolean>(dynxx_kv_remove(conn, cK.data));
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
        const auto cIn = JByteArrayArg(env, bytes);
        auto cRes = dynxx_coding_hex_bytes2str(cIn.data, cIn.size);
        return boxJString(env, cRes);
    }

    jbyteArray codingHexStr2Bytes(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                  jstring str) {
        const auto cStr = JStringArg(env, str);
        size_t outLen{0};
        const auto cRes = dynxx_coding_hex_str2bytes(cStr.data, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

// Crypto

    jbyteArray cryptoRandom(JNIEnv *env, [[maybe_unused]] jobject thiz,
                            jint len) {
        const auto cRes = dynxx_crypto_rand(len);
        return moveToJByteArray(env, cRes, len);
    }

    jbyteArray cryptoAesEncrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input, jbyteArray key) {
        const auto cIn = JByteArrayArg(env, input);
        const auto cKey = JByteArrayArg(env, key);

        size_t outLen{0};
        const auto cRes = dynxx_crypto_aes_encrypt(cIn.data, cIn.size,
                                                    cKey.data, cKey.size,
                                                    &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jbyteArray cryptoAesDecrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input, jbyteArray key) {
        const auto cIn = JByteArrayArg(env, input);
        const auto cKey = JByteArrayArg(env, key);

        size_t outLen{0};
        const auto cRes = dynxx_crypto_aes_decrypt(cIn.data, cIn.size,
                                                    cKey.data, cKey.size,
                                                    &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jbyteArray cryptoAesGcmEncrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                   jbyteArray input, jbyteArray key, jbyteArray init_vector, jbyteArray aad, jint tag_bits) {
        const auto cIn = JByteArrayArg(env, input);
        const auto cKey = JByteArrayArg(env, key);
        const auto cIv = JByteArrayArg(env, init_vector);
        const auto cAad = JByteArrayArg(env, aad);

        size_t outLen{0};
        const auto cRes = dynxx_crypto_aes_gcm_encrypt(cIn.data, cIn.size,
                                                        cKey.data, cKey.size,
                                                        cIv.data, cIv.size,
                                                        cAad.data, cAad.size,
                                                        tag_bits, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jbyteArray cryptoAesGcmDecrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                   jbyteArray input, jbyteArray key, jbyteArray init_vector, jbyteArray aad, jint tag_bits) {
        const auto cIn = JByteArrayArg(env, input);
        const auto cKey = JByteArrayArg(env, key);
        const auto cIv = JByteArrayArg(env, init_vector);
        const auto cAad = JByteArrayArg(env, aad);

        size_t outLen{0};
        const auto cRes = dynxx_crypto_aes_gcm_decrypt(cIn.data, cIn.size,
                                                        cKey.data, cKey.size,
                                                        cIv.data, cIv.size,
                                                        cAad.data, cAad.size,
                                                        tag_bits, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jstring cryptoRsaGenKey(JNIEnv *env, [[maybe_unused]] jobject thiz,
                            jstring base64, jboolean is_public) {
        const auto cBase64 = JStringArg(env, base64);
        auto cRes = dynxx_crypto_rsa_gen_key(cBase64.data, is_public == JNI_TRUE);
        return boxJString(env, cRes);
    }

    jbyteArray cryptoRsaEncrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input, jbyteArray key, jint padding) {
        const auto cIn = JByteArrayArg(env, input);
        const auto cKey = JByteArrayArg(env, key);

        size_t outLen{0};
        const auto cRes = dynxx_crypto_rsa_encrypt(cIn.data, cIn.size,
                                                    cKey.data, cKey.size,
                                                    static_cast<DynXXCryptoRSAPadding>(padding), &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jbyteArray cryptoRsaDecrypt(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input, jbyteArray key, jint padding) {
        const auto cIn = JByteArrayArg(env, input);
        const auto cKey = JByteArrayArg(env, key);

        size_t outLen{0};
        const auto cRes = dynxx_crypto_rsa_decrypt(cIn.data, cIn.size,
                                                    cKey.data, cKey.size,
                                                    static_cast<DynXXCryptoRSAPadding>(padding), &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jbyteArray cryptoHashMd5(JNIEnv *env, [[maybe_unused]] jobject thiz,
                             jbyteArray input) {
        const auto cIn = JByteArrayArg(env, input);
        size_t outLen{0};
        const auto cRes = dynxx_crypto_hash_md5(cIn.data, cIn.size, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jbyteArray cryptoHashSha1(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input) {
        const auto cIn = JByteArrayArg(env, input);
        size_t outLen{0};
        const auto cRes = dynxx_crypto_hash_sha1(cIn.data, cIn.size, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jbyteArray cryptoHashSha256(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                jbyteArray input) {
        const auto cIn = JByteArrayArg(env, input);
        size_t outLen{0};
        const auto cRes = dynxx_crypto_hash_sha256(cIn.data, cIn.size, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jbyteArray cryptoBase64Encode(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                  jbyteArray input, jboolean noNewLines) {
        const auto cIn = JByteArrayArg(env, input);
        size_t outLen{0};
        const auto cRes = dynxx_crypto_base64_encode(cIn.data, cIn.size, noNewLines == JNI_TRUE, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jbyteArray cryptoBase64Decode(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                  jbyteArray input, jboolean noNewLines) {
        const auto cIn = JByteArrayArg(env, input);
        size_t outLen{0};
        const auto cRes = dynxx_crypto_base64_decode(cIn.data, cIn.size, noNewLines == JNI_TRUE, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

// JsonDecoder

    jint jsonReadType([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                      jlong node) {
        return dynxx_json_node_read_type(node);
    }

    jlong jsonDecoderInit(JNIEnv *env, [[maybe_unused]] jobject thiz,
                          jstring json) {
        const auto cJson = JStringArg(env, json);
        return dynxx_json_decoder_init(cJson.data);
    }

    jlong jsonDecoderReadNode(JNIEnv *env, [[maybe_unused]] jobject thiz,
                              jlong decoder, jlong node, jstring k) {
        const auto cK = JStringArg(env, k);
        return dynxx_json_decoder_read_node(decoder, node, cK.data);
    }

    jstring jsonDecoderReadString(JNIEnv *env, [[maybe_unused]] jobject thiz,
                                  jlong decoder, jlong node) {
        auto cRes = dynxx_json_decoder_read_string(decoder, node);
        return boxJString(env, cRes);
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
        const auto cIn = JByteArrayArg(env, inBytes);
        auto ret = dynxx_z_zip_input(zip, cIn.data, cIn.size, inFinish == JNI_TRUE);
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
        return static_cast<jboolean>(dynxx_z_zip_process_finished(zip));
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
        const auto cIn = JByteArrayArg(env, inBytes);
        auto ret = dynxx_z_unzip_input(unzip, cIn.data, cIn.size, inFinish == JNI_TRUE);
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
        return static_cast<jboolean>(dynxx_z_unzip_process_finished(unzip));
    }

    void zUnZipRelease([[maybe_unused]] JNIEnv *env, [[maybe_unused]] jobject thiz,
                       jlong unzip) {
        dynxx_z_unzip_release(unzip);
    }

    jbyteArray zZipBytes(JNIEnv *env, [[maybe_unused]] jobject thiz,
                         jint mode, jlong buffer_size, jint format, jbyteArray bytes) {
        const auto cIn = JByteArrayArg(env, bytes);
        size_t outLen{0};
        const auto cRes = dynxx_z_bytes_zip(static_cast<DynXXZipCompressMode>(mode), buffer_size, static_cast<DynXXZFormat>(format),
                                             cIn.data, cIn.size, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    jbyteArray zUnZipBytes(JNIEnv *env, [[maybe_unused]] jobject thiz,
                           jlong buffer_size, jint format, jbyteArray bytes) {
        const auto cIn = JByteArrayArg(env, bytes);
        size_t outLen{0};
        const auto cRes = dynxx_z_bytes_unzip(buffer_size, static_cast<DynXXZFormat>(format),
                                               cIn.data, cIn.size, &outLen);
        return moveToJByteArray(env, cRes, outLen);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
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
    const auto env = getEnv(vm, JNI_VER);
    if (env == nullptr) {
        return JNI_ERR;
    }

    sJNIClass = findClassInCache(env, JNIClassName);
    if (sJNIClass == nullptr) {
        return JNI_ERR;
    }

    if (const auto ret = env->RegisterNatives(sJNIClass, JCFuncList, sizeof(JCFuncList) / sizeof(JNINativeMethod)); ret != JNI_OK) {
        return JNI_ERR;
    }

    return JNI_VER;
}

void JNI_OnUnload(JavaVM *vm, [[maybe_unused]] void *reserved) {
    if (const auto env = getEnv(vm, JNI_VER); env != nullptr) {
        env->UnregisterNatives(sJNIClass);
        releaseCachedClass(env);
        env->DeleteGlobalRef(sLogCallback);
        env->DeleteGlobalRef(sJsMsgCallback);
    }

    dynxx_log_set_callback(nullptr);
    dynxx_js_set_msg_callback(nullptr);

    sVM = nullptr;
    sLogCallback = nullptr;
    sJsMsgCallback = nullptr;
}
