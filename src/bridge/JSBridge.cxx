#if defined(USE_QJS)
#include "JSBridge.hxx"

#include <cstring>
#include <cstdlib>

#include <memory>
#include <functional>

#include <DynXX/CXX/Macro.hxx>

#include "../core/util/TypeUtil.hxx"
#include "../core/vm/JSVM.hxx"
#include "ScriptBridge.hxx"

namespace {
    std::unique_ptr<DynXX::Core::VM::JSVM> vm = nullptr;
    std::function<const char *(const char *msg)> msgCbk = nullptr;

#define DEF_API(f, T) DEF_JS_FUNC_##T(f##J, f##S)
#define DEF_API_ASYNC(f, T) DEF_JS_FUNC_##T##_ASYNC(vm, f##J, f##S)

#define BIND_API(f)                                                    \
        if (vm) [[likely]] {                                           \
            vm->bindFunc(#f, f##J);                                    \
        }

    bool loadF(const std::string &file, const bool isModule) {
        if (!vm || file.empty()) [[unlikely]] {
            return false;
        }
        return vm->loadFile(file, isModule);
    }

    bool loadS(const std::string &script, const std::string &name, const bool isModule) {
        if (!vm || script.empty() || name.empty()) [[unlikely]] {
            return false;
        }
        return vm->loadScript(script, name, isModule);
    }

    bool loadB(const Bytes &bytes, const bool isModule) {
        if (!vm || bytes.empty()) [[unlikely]] {
            return false;
        }
        return vm->loadBinary(bytes, isModule);
    }

    std::optional<std::string> call(std::string_view func, std::string_view params, const bool await) {
        if (!vm || func.empty()) [[unlikely]] {
            return std::nullopt;
        }
        return vm->callFunc(func, params, await);
    }

    void setMsgCallback(const std::function<const char *(const char *msg)> &callback) {
        msgCbk = callback;
    }

    std::string dynxx_call_platformS(const char *msg) {
        if (msg == nullptr || msgCbk == nullptr) {
            return {};
        }
        const auto res = msgCbk(msg);
        if (res == nullptr) [[unlikely]] {
            return {};
        }
        return {res};
    }
}

#pragma mark C++ API

bool dynxxJsLoadF(const std::string &file, bool isModule) {
    return loadF(file, isModule);
}

bool dynxxJsLoadS(const std::string &script, const std::string &name, bool isModule) {
    return loadS(script, name, isModule);
}

bool dynxxJsLoadB(const Bytes &bytes, bool isModule) {
    return loadB(bytes, isModule);
}

std::optional<std::string> dynxxJsCall(const std::string &func, const std::string &params, bool await) {
    return call(func, params, await);
}

void dynxxJsSetMsgCallback(const std::function<const char *(const char *msg)> &callback) {
    setMsgCallback(callback);
}

#pragma mark C API

EXPORT_AUTO
bool dynxx_js_loadF(const char *file, bool is_module) {
    return dynxxJsLoadF(makeStr(file), is_module);
}

EXPORT_AUTO
bool dynxx_js_loadS(const char *script, const char *name, bool is_module) {
    return dynxxJsLoadS(makeStr(script), makeStr(name), is_module);
}

EXPORT_AUTO
bool dynxx_js_loadB(const byte *bytes, size_t len, bool is_module) {
    return dynxxJsLoadB(makeBytes(bytes, len), is_module);
}

EXPORT_AUTO
const char *dynxx_js_call(const char *func, const char *params, bool await) {
    if (func == nullptr) [[unlikely]] {
        return "";
    }
    const auto s = dynxxJsCall(func, params ? params : "", await).value_or("");
    return DynXX::Core::Util::Type::copyStr(s);
}

EXPORT_AUTO
void dynxx_js_set_msg_callback(const char *(*const callback)(const char *msg)) {
    dynxxJsSetMsgCallback(callback);
}

#pragma mark JS API - Declaration

DEF_API(dynxx_call_platform, STRING)

DEF_API(dynxx_get_version, STRING)
DEF_API(dynxx_root_path, STRING)

DEF_API(dynxx_device_type, INT32)
DEF_API(dynxx_device_name, STRING)
DEF_API(dynxx_device_manufacturer, STRING)
DEF_API(dynxx_device_os_version, STRING)
DEF_API(dynxx_device_cpu_arch, INT32)

DEF_API(dynxx_log_print, VOID)

DEF_API_ASYNC(dynxx_net_http_request, STRING)
DEF_API_ASYNC(dynxx_net_http_download, BOOL)

DEF_API(dynxx_store_sqlite_open, STRING)
DEF_API_ASYNC(dynxx_store_sqlite_execute, BOOL)
DEF_API_ASYNC(dynxx_store_sqlite_query_do, STRING)
DEF_API(dynxx_store_sqlite_query_read_row, BOOL)
DEF_API(dynxx_store_sqlite_query_read_column_text, STRING)
DEF_API(dynxx_store_sqlite_query_read_column_integer, INT64)
DEF_API(dynxx_store_sqlite_query_read_column_float, FLOAT)
DEF_API(dynxx_store_sqlite_query_drop, VOID)
DEF_API(dynxx_store_sqlite_close, VOID)

DEF_API(dynxx_store_kv_open, STRING)
DEF_API(dynxx_store_kv_read_string, STRING)
DEF_API(dynxx_store_kv_write_string, BOOL)
DEF_API(dynxx_store_kv_read_integer, INT64)
DEF_API(dynxx_store_kv_write_integer, BOOL)
DEF_API(dynxx_store_kv_read_float, FLOAT)
DEF_API(dynxx_store_kv_write_float, BOOL)
DEF_API(dynxx_store_kv_all_keys, STRING)
DEF_API(dynxx_store_kv_contains, BOOL)
DEF_API(dynxx_store_kv_remove, BOOL)
DEF_API(dynxx_store_kv_clear, VOID)
DEF_API(dynxx_store_kv_close, VOID)

DEF_API(dynxx_coding_hex_bytes2str, STRING)
DEF_API(dynxx_coding_hex_str2bytes, STRING)
DEF_API(dynxx_coding_bytes2str, STRING)
DEF_API(dynxx_coding_str2bytes, STRING)
DEF_API(dynxx_coding_case_upper, STRING)
DEF_API(dynxx_coding_case_lower, STRING)

DEF_API(dynxx_crypto_rand, STRING)
DEF_API(dynxx_crypto_aes_encrypt, STRING)
DEF_API(dynxx_crypto_aes_decrypt, STRING)
DEF_API(dynxx_crypto_aes_gcm_encrypt, STRING)
DEF_API(dynxx_crypto_aes_gcm_decrypt, STRING)
DEF_API(dynxx_crypto_rsa_gen_key, STRING)
DEF_API(dynxx_crypto_rsa_encrypt, STRING)
DEF_API(dynxx_crypto_rsa_decrypt, STRING)
DEF_API(dynxx_crypto_hash_md5, STRING)
DEF_API(dynxx_crypto_hash_sha1, STRING)
DEF_API(dynxx_crypto_hash_sha256, STRING)
DEF_API(dynxx_crypto_base64_encode, STRING)
DEF_API(dynxx_crypto_base64_decode, STRING)

DEF_API(dynxx_z_zip_init, STRING)
DEF_API(dynxx_z_zip_input, INT64)
DEF_API(dynxx_z_zip_process_do, STRING)
DEF_API(dynxx_z_zip_process_finished, BOOL)
DEF_API(dynxx_z_zip_release, VOID)
DEF_API(dynxx_z_unzip_init, STRING)
DEF_API(dynxx_z_unzip_input, INT64)
DEF_API(dynxx_z_unzip_process_do, STRING)
DEF_API(dynxx_z_unzip_process_finished, BOOL)
DEF_API(dynxx_z_unzip_release, VOID)
DEF_API_ASYNC(dynxx_z_bytes_zip, STRING)
DEF_API_ASYNC(dynxx_z_bytes_unzip, STRING)

#pragma mark JS API - Binding

static void registerFuncs() {
    BIND_API(dynxx_call_platform);

    BIND_API(dynxx_get_version);
    BIND_API(dynxx_root_path);

    BIND_API(dynxx_log_print);

    BIND_API(dynxx_device_type);
    BIND_API(dynxx_device_name);
    BIND_API(dynxx_device_manufacturer);
    BIND_API(dynxx_device_os_version);
    BIND_API(dynxx_device_cpu_arch);

    BIND_API(dynxx_net_http_request);
    BIND_API(dynxx_net_http_download);

    BIND_API(dynxx_store_sqlite_open);
    BIND_API(dynxx_store_sqlite_execute);
    BIND_API(dynxx_store_sqlite_query_do);
    BIND_API(dynxx_store_sqlite_query_read_row);
    BIND_API(dynxx_store_sqlite_query_read_column_text);
    BIND_API(dynxx_store_sqlite_query_read_column_integer);
    BIND_API(dynxx_store_sqlite_query_read_column_float);
    BIND_API(dynxx_store_sqlite_query_drop);
    BIND_API(dynxx_store_sqlite_close);

    BIND_API(dynxx_store_kv_open);
    BIND_API(dynxx_store_kv_read_string);
    BIND_API(dynxx_store_kv_write_string);
    BIND_API(dynxx_store_kv_read_integer);
    BIND_API(dynxx_store_kv_write_integer);
    BIND_API(dynxx_store_kv_read_float);
    BIND_API(dynxx_store_kv_write_float);
    BIND_API(dynxx_store_kv_all_keys);
    BIND_API(dynxx_store_kv_contains);
    BIND_API(dynxx_store_kv_remove);
    BIND_API(dynxx_store_kv_clear);
    BIND_API(dynxx_store_kv_close);

    BIND_API(dynxx_coding_hex_bytes2str);
    BIND_API(dynxx_coding_hex_str2bytes);
    BIND_API(dynxx_coding_bytes2str);
    BIND_API(dynxx_coding_str2bytes);
    BIND_API(dynxx_coding_case_upper);
    BIND_API(dynxx_coding_case_lower);

    BIND_API(dynxx_crypto_rand);
    BIND_API(dynxx_crypto_aes_encrypt);
    BIND_API(dynxx_crypto_aes_decrypt);
    BIND_API(dynxx_crypto_aes_gcm_encrypt);
    BIND_API(dynxx_crypto_aes_gcm_decrypt);
    BIND_API(dynxx_crypto_rsa_gen_key);
    BIND_API(dynxx_crypto_rsa_encrypt);
    BIND_API(dynxx_crypto_rsa_decrypt);
    BIND_API(dynxx_crypto_hash_md5);
    BIND_API(dynxx_crypto_hash_sha1);
    BIND_API(dynxx_crypto_hash_sha256);
    BIND_API(dynxx_crypto_base64_encode);
    BIND_API(dynxx_crypto_base64_decode);

    BIND_API(dynxx_z_zip_init);
    BIND_API(dynxx_z_zip_input);
    BIND_API(dynxx_z_zip_process_do);
    BIND_API(dynxx_z_zip_process_finished);
    BIND_API(dynxx_z_zip_release);
    BIND_API(dynxx_z_unzip_init);
    BIND_API(dynxx_z_unzip_input);
    BIND_API(dynxx_z_unzip_process_do);
    BIND_API(dynxx_z_unzip_process_finished);
    BIND_API(dynxx_z_unzip_release);
    BIND_API(dynxx_z_bytes_zip)
    BIND_API(dynxx_z_bytes_unzip)
}

#pragma mark Inner API

void dynxx_js_init() {
    if (vm) [[unlikely]] {
        return;
    }
    vm = std::make_unique<DynXX::Core::VM::JSVM>();
    registerFuncs();
}

void dynxx_js_release() {
    if (!vm) [[unlikely]] {
        return;
    }
    vm.reset();
    msgCbk = nullptr;
}

#endif
