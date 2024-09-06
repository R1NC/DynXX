#ifndef NGENXX_STORE_KV_H_
#define NGENXX_STORE_KV_H_

/**
 * @brief open KV Store, support to access from multiple processes
 * @param uid universal id
 * @return A KV store handle
 */
void *ngenxx_store_kv_open(const char *uid);

/**
 * @brief Read string from KV Store
 * @param kv A KV store handle
 * @param k key
 * @return String value
 */
const char *ngenxx_store_kv_read_string(void *kv, const char *k);

/**
 * @brief Write string to KV Store
 * @param kv A KV store handle
 * @param k key
 * @param v value
 * @return Successfull or not
 */
bool ngenxx_store_kv_write_string(void *kv, const char *k, const char *v);

/**
 * @brief Read integer from KV Store
 * @param kv A KV store handle
 * @param k key
 * @return int value
 */
long long ngenxx_store_kv_read_integer(void *kv, const char *k);

/**
 * @brief Write integer to KV Store
 * @param kv A KV store handle
 * @param k key
 * @param v value
 * @return Successfull or not
 */
bool ngenxx_store_kv_write_integer(void *kv, const char *k, long long v);

/**
 * @brief Read float from KV Store
 * @param kv A KV store handle
 * @param k key
 * @return float value
 */
double ngenxx_store_kv_read_float(void *kv, const char *k);

/**
 * @brief Write float to KV Store
 * @param kv A KV store handle
 * @param k key
 * @param v value
 * @return Successfull or not
 */
bool ngenxx_store_kv_write_float(void *kv, const char *k, double v);

/**
 * @brief Search key from KV Store
 * @param kv A KV store handle
 * @param k key
 * @return Exist or not
 */
bool ngenxx_store_kv_contains(void *kv, const char *k);

/**
 * @brief Clear a KV Store
 * @param kv A KV store handle
 */
void ngenxx_store_kv_clear(void *kv);

/**
 * @brief Search key from KV Store
 * @param kv A KV store handle
 */
void ngenxx_store_kv_close(void *kv);

#endif // NGENXX_STORE_KV_H_