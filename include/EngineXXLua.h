#ifndef ENGINEXX_LUA_H_
#define ENGINEXX_LUA_H_

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

#endif // ENGINEXX_LUA_H_