#ifndef ENGINEXX_H_
#define ENGINEXX_H_

/**
 * @brief Read version
 * @return version name
 */
const char *enginexx_get_version(void);

/**
 * @brief http request
 * @param url URL
 * @param params params(transfer multiple params like `v1=a&v2=b`)
 * @return response
 */
const char *enginexx_http_req(const char *url, const char *params);

/**
 * @brief Create Lua environment
 * @return `lua_State*`
 */
void *enginexx_L_create(void);

/**
 * @brief Release Lua environment
 * @param lstate `lua_State*`
 */
void enginexx_L_destroy(void *lstate);

/**
 * @brief Load Lua file
 * @param lstate `lua_State*`
 * @param file Lua file path
 * @return success or not
 */
int enginexx_L_loadF(void *lstate, const char *file);

/**
 * @brief Load Lua script content
 * @param lstate `lua_State*`
 * @param script Lua script content
 * @return success or not
 */
int enginexx_L_loadS(void *lstate, const char *script);

/**
 * @brief Call Lua function
 * @param lstate `lua_State*`
 * @param func Lua function name
 * @param params Lua function params（wrap multiple params with json）
 * @return return value of Lua function
 */
const char *enginexx_L_call(void *lstate, const char *func, const char *params);

#endif // ENGINEXX_H_