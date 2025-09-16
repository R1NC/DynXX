#ifndef DYNXX_INCLUDE_C_KV_H_
#define DYNXX_INCLUDE_C_KV_H_

#include "Types.h"

DYNXX_EXTERN_C_BEGIN

static const size_t DYNXX_STORE_KV_KEY_MAX_LENGTH = 256; /**< Max length of the KV Key */

typedef address DynXXKVConnHandle;

/**
 * @brief open KV Store, support to access from multiple processes
 * @param _id kv id
 * @return A KV connection handle
 */
DynXXKVConnHandle dynxx_kv_open(const char *_id);

/**
 * @brief Read string from KV Store
 * @param conn A KV connection handle
 * @param k key
 * @return String value
 */
const char *dynxx_kv_read_string(const DynXXKVConnHandle conn, const char *k);

/**
 * @brief Write string to KV Store
 * @param conn A KV connection handle
 * @param k key
 * @param v value
 * @return Successfull or not
 */
bool dynxx_kv_write_string(const DynXXKVConnHandle conn, const char *k, const char *v);

/**
 * @brief Read integer from KV Store
 * @param conn A KV connection handle
 * @param k key
 * @return integer value
 */
int64_t dynxx_kv_read_integer(const DynXXKVConnHandle conn, const char *k);

/**
 * @brief Write integer to KV Store
 * @param conn A KV connection handle
 * @param k key
 * @param v value
 * @return Successfull or not
 */
bool dynxx_kv_write_integer(const DynXXKVConnHandle conn, const char *k, int64_t v);

/**
 * @brief Read float from KV Store
 * @param conn A KV connection handle
 * @param k key
 * @return float value
 */
double dynxx_kv_read_float(const DynXXKVConnHandle conn, const char *k);

/**
 * @brief Write float to KV Store
 * @param conn A KV connection handle
 * @param k key
 * @param v value
 * @return Successfull or not
 */
bool dynxx_kv_write_float(const DynXXKVConnHandle conn, const char *k, double v);

/**
 * @brief Fetch all keys from KV Store
 * @param conn A KV connection handle
 * @param len A pointer to read the length of the key array
 * @return A pointer to the key array
 */
char *const *dynxx_kv_all_keys(const DynXXKVConnHandle conn, size_t *len);

/**
 * @brief Search key from KV Store
 * @param conn A KV connection handle
 * @param k key
 * @return Exist or not
 */
bool dynxx_kv_contains(const DynXXKVConnHandle conn, const char *k);

/**
 * @brief Remove key-value from KV Store
 * @param conn A KV connection handle
 * @param k key
 * @return Success or not
 */
bool dynxx_kv_remove(const DynXXKVConnHandle conn, const char *k);

/**
 * @brief Clear a KV Store
 * @param conn A KV connection handle
 */
void dynxx_kv_clear(const DynXXKVConnHandle conn);

/**
 * @brief Close a KV Store connection
 * @param conn A KV connection handle
 */
void dynxx_kv_close(const DynXXKVConnHandle conn);

DYNXX_EXTERN_C_END

#endif // DYNXX_INCLUDE_C_KV_H_
