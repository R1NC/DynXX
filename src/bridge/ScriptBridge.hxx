#ifndef DYNXX_SRC_BRIDGE_SCRIPTBRIDGE_HXX_
#define DYNXX_SRC_BRIDGE_SCRIPTBRIDGE_HXX_

#if defined(__cplusplus)

#include <string>

std::string dynxx_get_versionS([[maybe_unused]] const char *json);

std::string dynxx_root_pathS([[maybe_unused]] const char *json);

int dynxx_device_typeS([[maybe_unused]] const char *json);

std::string dynxx_device_nameS([[maybe_unused]] const char *json);

std::string dynxx_device_manufacturerS([[maybe_unused]] const char *json);

std::string dynxx_device_os_versionS([[maybe_unused]] const char *json);

int dynxx_device_cpu_archS([[maybe_unused]] const char *json);

void dynxx_log_printS(const char *json);

std::string dynxx_net_http_requestS(const char *json);

bool dynxx_net_http_downloadS(const char *json);

std::string dynxx_store_sqlite_openS(const char *json);

bool dynxx_store_sqlite_executeS(const char *json);

std::string dynxx_store_sqlite_query_doS(const char *json);

bool dynxx_store_sqlite_query_read_rowS(const char *json);

std::string dynxx_store_sqlite_query_read_column_textS(const char *json);

int64_t dynxx_store_sqlite_query_read_column_integerS(const char *json);

double dynxx_store_sqlite_query_read_column_floatS(const char *json);

void dynxx_store_sqlite_query_dropS(const char *json);

void dynxx_store_sqlite_closeS(const char *json);

std::string dynxx_store_kv_openS(const char *json);

std::string dynxx_store_kv_read_stringS(const char *json);

bool dynxx_store_kv_write_stringS(const char *json);

int64_t dynxx_store_kv_read_integerS(const char *json);

bool dynxx_store_kv_write_integerS(const char *json);

double dynxx_store_kv_read_floatS(const char *json);

bool dynxx_store_kv_write_floatS(const char *json);

std::string dynxx_store_kv_all_keysS(const char *json);

bool dynxx_store_kv_containsS(const char *json);

bool dynxx_store_kv_removeS(const char *json);

void dynxx_store_kv_clearS(const char *json);

void dynxx_store_kv_closeS(const char *json);

std::string dynxx_coding_case_upperS(const char *json);

std::string dynxx_coding_case_lowerS(const char *json);

std::string dynxx_coding_hex_bytes2strS(const char *json);

std::string dynxx_coding_hex_str2bytesS(const char *json);

std::string dynxx_coding_bytes2strS(const char *json);

std::string dynxx_coding_str2bytesS(const char *json);

std::string dynxx_crypto_randS(const char *json);

std::string dynxx_crypto_aes_encryptS(const char *json);

std::string dynxx_crypto_aes_decryptS(const char *json);

std::string dynxx_crypto_aes_gcm_encryptS(const char *json);

std::string dynxx_crypto_aes_gcm_decryptS(const char *json);

std::string dynxx_crypto_rsa_gen_keyS(const char *json);

std::string dynxx_crypto_rsa_encryptS(const char *json);

std::string dynxx_crypto_rsa_decryptS(const char *json);

std::string dynxx_crypto_hash_md5S(const char *json);

std::string dynxx_crypto_hash_sha1S(const char *json);

std::string dynxx_crypto_hash_sha256S(const char *json);

std::string dynxx_crypto_base64_encodeS(const char *json);

std::string dynxx_crypto_base64_decodeS(const char *json);

std::string dynxx_z_zip_initS(const char *json);

size_t dynxx_z_zip_inputS(const char *json);

std::string dynxx_z_zip_process_doS(const char *json);

bool dynxx_z_zip_process_finishedS(const char *json);

void dynxx_z_zip_releaseS(const char *json);

std::string dynxx_z_unzip_initS(const char *json);

size_t dynxx_z_unzip_inputS(const char *json);

std::string dynxx_z_unzip_process_doS(const char *json);

bool dynxx_z_unzip_process_finishedS(const char *json);

void dynxx_z_unzip_releaseS(const char *json);

std::string dynxx_z_bytes_zipS(const char *json);

std::string dynxx_z_bytes_unzipS(const char *json);

#endif

#endif // DYNXX_SRC_BRIDGE_SCRIPTBRIDGE_HXX_
