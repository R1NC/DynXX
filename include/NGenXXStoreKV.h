#ifndef NGENXX_STORE_KV_H_
#define NGENXX_STORE_KV_H_

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief open KV Store, support to access from multiple processes
 * @param _id kv id
 * @return A KV connection handle
 */
void *ngenxx_store_kv_open(const char *_id);

/**
 * @brief Read string from KV Store
 * @param conn A KV connection handle
 * @param k key
 * @return String value
 */
const char *ngenxx_store_kv_read_string(void *conn, const char *k);

/**
 * @brief Write string to KV Store
 * @param conn A KV connection handle
 * @param k key
 * @param v value
 * @return Successfull or not
 */
bool ngenxx_store_kv_write_string(void *conn, const char *k, const char *v);

/**
 * @brief Read integer from KV Store
 * @param conn A KV connection handle
 * @param k key
 * @return integer value
 */
long long ngenxx_store_kv_read_integer(void *conn, const char *k);

/**
 * @brief Write integer to KV Store
 * @param conn A KV connection handle
 * @param k key
 * @param v value
 * @return Successfull or not
 */
bool ngenxx_store_kv_write_integer(void *conn, const char *k, long long v);

/**
 * @brief Read float from KV Store
 * @param conn A KV connection handle
 * @param k key
 * @return float value
 */
double ngenxx_store_kv_read_float(void *conn, const char *k);

/**
 * @brief Write float to KV Store
 * @param conn A KV connection handle
 * @param k key
 * @param v value
 * @return Successfull or not
 */
bool ngenxx_store_kv_write_float(void *conn, const char *k, double v);

/**
 * @brief Search key from KV Store
 * @param conn A KV connection handle
 * @param k key
 * @return Exist or not
 */
bool ngenxx_store_kv_contains(void *conn, const char *k);

/**
 * @brief Remove key-value from KV Store
 * @param conn A KV connection handle
 * @param k key
 */
void ngenxx_store_kv_remove(void *conn, const char *k);

/**
 * @brief Clear a KV Store
 * @param conn A KV connection handle
 */
void ngenxx_store_kv_clear(void *conn);

/**
 * @brief Close a KV Store connection
 * @param conn A KV connection handle
 */
void ngenxx_store_kv_close(void *conn);

#ifdef  __cplusplus
}
#endif

#endif // NGENXX_STORE_KV_H_