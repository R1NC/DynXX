#pragma once

#include "Types.h"

DYNXX_EXTERN_C_BEGIN

/**
 * @brief Load Lua file
 * @warning Not accessible in JS/Lua!
 * @warning Not accessible in WebAssembly!(Will alert a prompt window.)
 * @param file Lua file path
 * @return success or not
 */
bool dynxx_lua_loadF(const char *file);

/**
 * @brief Load Lua script content
 * @warning Not accessible in JS/Lua!
 * @param script Lua script content
 * @return success or not
 */
bool dynxx_lua_loadS(const char *script);

/**
 * @brief Call Lua function
 * @warning Not accessible in JS/Lua!
 * @param func Lua function name
 * @param params Lua function params（wrap multiple params with json）
 * @return return value of Lua function
 */
const char *dynxx_lua_call(const char *func, const char *params);

DYNXX_EXTERN_C_END
