#ifndef NGENXX_LUA_H_
#define NGENXX_LUA_H_

/**
 * @brief Create Lua environment
 * @warning Not accessible in Lua!
 * @return `lua_State*`
 */
void *ngenxx_L_create(void);

/**
 * @brief Release Lua environment
 * @warning Not accessible in Lua!
 * @param lstate `lua_State*`
 */
void ngenxx_L_destroy(void *lstate);

/**
 * @brief Load Lua file
 * @warning Not accessible in Lua! Not accessible in WebAssembly!(Will alert a prompt window.)
 * @param lstate `lua_State*`
 * @param file Lua file path
 * @return success or not
 */
int ngenxx_L_loadF(void *lstate, const char *file);

/**
 * @brief Load Lua script content
 * @warning Not accessible in Lua!
 * @param lstate `lua_State*`
 * @param script Lua script content
 * @return success or not
 */
int ngenxx_L_loadS(void *lstate, const char *script);

/**
 * @brief Call Lua function
 * @warning Not accessible in Lua!
 * @param lstate `lua_State*`
 * @param func Lua function name
 * @param params Lua function params（wrap multiple params with json）
 * @return return value of Lua function
 */
const char *ngenxx_L_call(void *lstate, const char *func, const char *params);

#endif // NGENXX_LUA_H_