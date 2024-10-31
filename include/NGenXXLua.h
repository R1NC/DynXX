#ifndef NGENXX_INCLUDE_LUA_H_
#define NGENXX_INCLUDE_LUA_H_

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief Load Lua file
 * @warning Not accessible in JS/Lua! 
 * @warning Not accessible in WebAssembly!(Will alert a prompt window.)
 * @param file Lua file path
 * @return success or not
 */
bool ngenxx_lua_loadF(const char *file);

/**
 * @brief Load Lua script content
 * @warning Not accessible in JS/Lua!
 * @param script Lua script content
 * @return success or not
 */
bool ngenxx_lua_loadS(const char *script);

/**
 * @brief Call Lua function
 * @warning Not accessible in JS/Lua!
 * @param func Lua function name
 * @param params Lua function params（wrap multiple params with json）
 * @return return value of Lua function
 */
const char *ngenxx_lua_call(const char *func, const char *params);

#ifdef  __cplusplus
}
#endif

#endif // NGENXX_INCLUDE_LUA_H_