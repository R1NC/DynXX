#ifndef NGENXX_SRC_SCRIPT_HXX_
#define NGENXX_SRC_SCRIPT_HXX_

#include "../include/NGenXXTypes.h"

#ifdef __cplusplus

#include <string>

const std::string ngenxx_get_versionS(const char *json);
const std::string ngenxx_root_pathS(const char *json);

int ngenxx_device_typeS(const char *json);
const std::string ngenxx_device_nameS(const char *json);
const std::string ngenxx_device_manufacturerS(const char *json);
const std::string ngenxx_device_os_versionS(const char *json);
int ngenxx_device_cpu_archS(const char *json);

void ngenxx_log_printS(const char *json);

const std::string ngenxx_net_http_requestS(const char *json);

const std::string ngenxx_store_sqlite_openS(const char *json);
bool ngenxx_store_sqlite_executeS(const char *json);
const std::string ngenxx_store_sqlite_query_doS(const char *json);
bool ngenxx_store_sqlite_query_read_rowS(const char *json);
const std::string ngenxx_store_sqlite_query_read_column_textS(const char *json);
long long ngenxx_store_sqlite_query_read_column_integerS(const char *json);
double ngenxx_store_sqlite_query_read_column_floatS(const char *json);
void ngenxx_store_sqlite_query_dropS(const char *json);
void ngenxx_store_sqlite_closeS(const char *json);

const std::string ngenxx_store_kv_openS(const char *json);
const std::string ngenxx_store_kv_read_stringS(const char *json);
bool ngenxx_store_kv_write_stringS(const char *json);
long long ngenxx_store_kv_read_integerS(const char *json);
bool ngenxx_store_kv_write_integerS(const char *json);
double ngenxx_store_kv_read_floatS(const char *json);
bool ngenxx_store_kv_write_floatS(const char *json);
const std::string ngenxx_store_kv_all_keysS(const char *json);
bool ngenxx_store_kv_containsS(const char *json);
bool ngenxx_store_kv_removeS(const char *json);
void ngenxx_store_kv_clearS(const char *json);
void ngenxx_store_kv_closeS(const char *json);

const std::string ngenxx_coding_hex_bytes2strS(const char *json);
const std::string ngenxx_coding_hex_str2bytesS(const char *json);
const std::string ngenxx_coding_bytes2strS(const char *json);
const std::string ngenxx_coding_str2bytesS(const char *json);
const std::string ngenxx_coding_case_upperS(const char *json);
const std::string ngenxx_coding_case_lowerS(const char *json);

const std::string ngenxx_crypto_randS(const char *json);
const std::string ngenxx_crypto_aes_encryptS(const char *json);
const std::string ngenxx_crypto_aes_decryptS(const char *json);
const std::string ngenxx_crypto_aes_gcm_encryptS(const char *json);
const std::string ngenxx_crypto_aes_gcm_decryptS(const char *json);
const std::string ngenxx_crypto_hash_md5S(const char *json);
const std::string ngenxx_crypto_hash_sha256S(const char *json);
const std::string ngenxx_crypto_base64_encodeS(const char *json);
const std::string ngenxx_crypto_base64_decodeS(const char *json);

const std::string ngenxx_z_zip_initS(const char *json);
const size_t ngenxx_z_zip_inputS(const char *json);
const std::string ngenxx_z_zip_process_doS(const char *json);
bool ngenxx_z_zip_process_finishedS(const char *json);
void ngenxx_z_zip_releaseS(const char *json);
const std::string ngenxx_z_unzip_initS(const char *json);
const size_t ngenxx_z_unzip_inputS(const char *json);
const std::string ngenxx_z_unzip_process_doS(const char *json);
bool ngenxx_z_unzip_process_finishedS(const char *json);
void ngenxx_z_unzip_releaseS(const char *json);
const std::string ngenxx_z_bytes_zipS(const char *json);
const std::string ngenxx_z_bytes_unzipS(const char *json);

#endif

#endif // NGENXX_SRC_SCRIPT_HXX_