#ifndef NGENXX_S_HXX_
#define NGENXX_S_HXX_

#ifdef __cplusplus

const char *ngenxx_get_versionS(const char *json);

int ngenxx_device_typeS(const char *json);
const char *ngenxx_device_nameS(const char *json);
const char *ngenxx_device_manufacturerS(const char *json);
const char *ngenxx_device_os_versionS(const char *json);
int ngenxx_device_cpu_archS(const char *json);

void ngenxx_log_printS(const char *json);

const char *ngenxx_net_http_requestS(const char *json);

void *ngenxx_store_sqlite_openS(const char *json);
bool ngenxx_store_sqlite_executeS(const char *json);
void *ngenxx_store_sqlite_query_doS(const char *json);
bool ngenxx_store_sqlite_query_read_rowS(const char *json);
const char *ngenxx_store_sqlite_query_read_column_textS(const char *json);
long long ngenxx_store_sqlite_query_read_column_integerS(const char *json);
double ngenxx_store_sqlite_query_read_column_floatS(const char *json);
void ngenxx_store_sqlite_query_dropS(const char *json);
void ngenxx_store_sqlite_closeS(const char *json);

void *ngenxx_store_kv_openS(const char *json);
const char *ngenxx_store_kv_read_stringS(const char *json);
bool ngenxx_store_kv_write_stringS(const char *json);
long long ngenxx_store_kv_read_integerS(const char *json);
bool ngenxx_store_kv_write_integerS(const char *json);
double ngenxx_store_kv_read_floatS(const char *json);
double ngenxx_store_kv_write_floatS(const char *json);
bool ngenxx_store_kv_containsS(const char *json);
void ngenxx_store_kv_clearS(const char *json);
void ngenxx_store_kv_closeS(const char *json);

const char *ngenxx_coding_hex_bytes2strS(const char *json);
const char *ngenxx_coding_hex_str2bytesS(const char *json);

const char *ngenxx_crypto_randS(const char *json);
const char *ngenxx_crypto_aes_encryptS(const char *json);
const char *ngenxx_crypto_aes_decryptS(const char *json);
const char *ngenxx_crypto_hash_md5S(const char *json);
const char *ngenxx_crypto_hash_sha256S(const char *json);
const char *ngenxx_crypto_base64_encodeS(const char *json);
const char *ngenxx_crypto_base64_decodeS(const char *json);

#endif

#endif // NGENXX_S_HXX_