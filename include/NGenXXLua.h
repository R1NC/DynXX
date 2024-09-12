#ifndef NGENXX_LUA_H_
#define NGENXX_LUA_H_

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief Load Lua file
 * @warning Not accessible in Lua! Not accessible in WebAssembly!(Will alert a prompt window.)
 * @param sdk SDK handle
 * @param file Lua file path
 * @return success or not
 */
bool ngenxx_L_loadF(void *sdk, const char *file);

/**
 * @brief Load Lua script content
 * @warning Not accessible in Lua!
 * @param sdk SDK handle
 * @param script Lua script content
 * @return success or not
 */
bool ngenxx_L_loadS(void *sdk, const char *script);

/**
 * @brief Call Lua function
 * @warning Not accessible in Lua!
 * @param sdk SDK handle
 * @param func Lua function name
 * @param params Lua function params（wrap multiple params with json）
 * @return return value of Lua function
 */
const char *ngenxx_L_call(void *sdk, const char *func, const char *params);

#ifdef  __cplusplus
}
#endif

#endif // NGENXX_LUA_H_