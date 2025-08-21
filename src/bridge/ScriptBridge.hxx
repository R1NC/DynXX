#ifndef DYNXX_SRC_BRIDGE_SCRIPTBRIDGE_HXX_
#define DYNXX_SRC_BRIDGE_SCRIPTBRIDGE_HXX_

#if defined(__cplusplus)

#include <string>

std::string dynxx_get_versionS([[maybe_unused]] const std::string_view json);

std::string dynxx_root_pathS([[maybe_unused]] const std::string_view json);

int dynxx_device_typeS([[maybe_unused]] const std::string_view json);

std::string dynxx_device_nameS([[maybe_unused]] const std::string_view json);

std::string dynxx_device_manufacturerS([[maybe_unused]] const std::string_view json);

std::string dynxx_device_os_versionS([[maybe_unused]] const std::string_view json);

int dynxx_device_cpu_archS([[maybe_unused]] const std::string_view json);

void dynxx_log_printS(const std::string_view json);

std::string dynxx_net_http_requestS(const std::string_view json);

bool dynxx_net_http_downloadS(const std::string_view json);

std::string dynxx_store_sqlite_openS(const std::string_view json);

bool dynxx_store_sqlite_executeS(const std::string_view json);

std::string dynxx_store_sqlite_query_doS(const std::string_view json);

bool dynxx_store_sqlite_query_read_rowS(const std::string_view json);

std::string dynxx_store_sqlite_query_read_column_textS(const std::string_view json);

int64_t dynxx_store_sqlite_query_read_column_integerS(const std::string_view json);

double dynxx_store_sqlite_query_read_column_floatS(const std::string_view json);

void dynxx_store_sqlite_query_dropS(const std::string_view json);

void dynxx_store_sqlite_closeS(const std::string_view json);

std::string dynxx_store_kv_openS(const std::string_view json);

std::string dynxx_store_kv_read_stringS(const std::string_view json);

bool dynxx_store_kv_write_stringS(const std::string_view json);

int64_t dynxx_store_kv_read_integerS(const std::string_view json);

bool dynxx_store_kv_write_integerS(const std::string_view json);

double dynxx_store_kv_read_floatS(const std::string_view json);

bool dynxx_store_kv_write_floatS(const std::string_view json);

std::string dynxx_store_kv_all_keysS(const std::string_view json);

bool dynxx_store_kv_containsS(const std::string_view json);

bool dynxx_store_kv_removeS(const std::string_view json);

void dynxx_store_kv_clearS(const std::string_view json);

void dynxx_store_kv_closeS(const std::string_view json);

std::string dynxx_coding_case_upperS(const std::string_view json);

std::string dynxx_coding_case_lowerS(const std::string_view json);

std::string dynxx_coding_hex_bytes2strS(const std::string_view json);

std::string dynxx_coding_hex_str2bytesS(const std::string_view json);

std::string dynxx_coding_bytes2strS(const std::string_view json);

std::string dynxx_coding_str2bytesS(const std::string_view json);

std::string dynxx_crypto_randS(const std::string_view json);

std::string dynxx_crypto_aes_encryptS(const std::string_view json);

std::string dynxx_crypto_aes_decryptS(const std::string_view json);

std::string dynxx_crypto_aes_gcm_encryptS(const std::string_view json);

std::string dynxx_crypto_aes_gcm_decryptS(const std::string_view json);

std::string dynxx_crypto_rsa_gen_keyS(const std::string_view json);

std::string dynxx_crypto_rsa_encryptS(const std::string_view json);

std::string dynxx_crypto_rsa_decryptS(const std::string_view json);

std::string dynxx_crypto_hash_md5S(const std::string_view json);

std::string dynxx_crypto_hash_sha1S(const std::string_view json);

std::string dynxx_crypto_hash_sha256S(const std::string_view json);

std::string dynxx_crypto_base64_encodeS(const std::string_view json);

std::string dynxx_crypto_base64_decodeS(const std::string_view json);

std::string dynxx_z_zip_initS(const std::string_view json);

size_t dynxx_z_zip_inputS(const std::string_view json);

std::string dynxx_z_zip_process_doS(const std::string_view json);

bool dynxx_z_zip_process_finishedS(const std::string_view json);

void dynxx_z_zip_releaseS(const std::string_view json);

std::string dynxx_z_unzip_initS(const std::string_view json);

size_t dynxx_z_unzip_inputS(const std::string_view json);

std::string dynxx_z_unzip_process_doS(const std::string_view json);

bool dynxx_z_unzip_process_finishedS(const std::string_view json);

void dynxx_z_unzip_releaseS(const std::string_view json);

std::string dynxx_z_bytes_zipS(const std::string_view json);

std::string dynxx_z_bytes_unzipS(const std::string_view json);

#endif

#endif // DYNXX_SRC_BRIDGE_SCRIPTBRIDGE_HXX_
