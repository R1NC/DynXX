#ifndef DYNXX_SRC_BRIDGE_SCRIPTAPI_HXX_
#define DYNXX_SRC_BRIDGE_SCRIPTAPI_HXX_

#if defined(__cplusplus)

#include <string>

std::string dynxx_get_versionS([[maybe_unused]] std::string_view json);

std::string dynxx_root_pathS([[maybe_unused]] std::string_view json);

int dynxx_device_typeS([[maybe_unused]] std::string_view json);

std::string dynxx_device_nameS([[maybe_unused]] std::string_view json);

std::string dynxx_device_manufacturerS([[maybe_unused]] std::string_view json);

std::string dynxx_device_os_versionS([[maybe_unused]] std::string_view json);

int dynxx_device_cpu_archS([[maybe_unused]] std::string_view json);

void dynxx_log_printS(std::string_view json);

std::string dynxx_net_http_requestS(std::string_view json);

bool dynxx_net_http_downloadS(std::string_view json);

std::string dynxx_sqlite_openS(std::string_view json);

bool dynxx_sqlite_executeS(std::string_view json);

std::string dynxx_sqlite_query_doS(std::string_view json);

bool dynxx_sqlite_query_read_rowS(std::string_view json);

std::string dynxx_sqlite_query_read_column_textS(std::string_view json);

int64_t dynxx_sqlite_query_read_column_integerS(std::string_view json);

double dynxx_sqlite_query_read_column_floatS(std::string_view json);

void dynxx_sqlite_query_dropS(std::string_view json);

void dynxx_sqlite_closeS(std::string_view json);

std::string dynxx_kv_openS(std::string_view json);

std::string dynxx_kv_read_stringS(std::string_view json);

bool dynxx_kv_write_stringS(std::string_view json);

int64_t dynxx_kv_read_integerS(std::string_view json);

bool dynxx_kv_write_integerS(std::string_view json);

double dynxx_kv_read_floatS(std::string_view json);

bool dynxx_kv_write_floatS(std::string_view json);

std::string dynxx_kv_all_keysS(std::string_view json);

bool dynxx_kv_containsS(std::string_view json);

bool dynxx_kv_removeS(std::string_view json);

void dynxx_kv_clearS(std::string_view json);

void dynxx_kv_closeS(std::string_view json);

std::string dynxx_coding_case_upperS(std::string_view json);

std::string dynxx_coding_case_lowerS(std::string_view json);

std::string dynxx_coding_hex_bytes2strS(std::string_view json);

std::string dynxx_coding_hex_str2bytesS(std::string_view json);

std::string dynxx_coding_bytes2strS(std::string_view json);

std::string dynxx_coding_str2bytesS(std::string_view json);

std::string dynxx_crypto_randS(std::string_view json);

std::string dynxx_crypto_aes_encryptS(std::string_view json);

std::string dynxx_crypto_aes_decryptS(std::string_view json);

std::string dynxx_crypto_aes_gcm_encryptS(std::string_view json);

std::string dynxx_crypto_aes_gcm_decryptS(std::string_view json);

std::string dynxx_crypto_rsa_gen_keyS(std::string_view json);

std::string dynxx_crypto_rsa_encryptS(std::string_view json);

std::string dynxx_crypto_rsa_decryptS(std::string_view json);

std::string dynxx_crypto_hash_md5S(std::string_view json);

std::string dynxx_crypto_hash_sha1S(std::string_view json);

std::string dynxx_crypto_hash_sha256S(std::string_view json);

std::string dynxx_crypto_base64_encodeS(std::string_view json);

std::string dynxx_crypto_base64_decodeS(std::string_view json);

std::string dynxx_z_zip_initS(std::string_view json);

size_t dynxx_z_zip_inputS(std::string_view json);

std::string dynxx_z_zip_process_doS(std::string_view json);

bool dynxx_z_zip_process_finishedS(std::string_view json);

void dynxx_z_zip_releaseS(std::string_view json);

std::string dynxx_z_unzip_initS(std::string_view json);

size_t dynxx_z_unzip_inputS(std::string_view json);

std::string dynxx_z_unzip_process_doS(std::string_view json);

bool dynxx_z_unzip_process_finishedS(std::string_view json);

void dynxx_z_unzip_releaseS(std::string_view json);

std::string dynxx_z_bytes_zipS(std::string_view json);

std::string dynxx_z_bytes_unzipS(std::string_view json);

#endif

#endif // DYNXX_SRC_BRIDGE_SCRIPTAPI_HXX_
