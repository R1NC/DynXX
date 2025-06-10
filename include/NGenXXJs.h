#ifndef NGENXX_INCLUDE_JS_H_
#define NGENXX_INCLUDE_JS_H_

#include <stdbool.h>
#include <stddef.h>

#include "NGenXXMacro.h"
#include "NGenXXTypes.h"

EXTERN_C_BEGIN

    /**
     * @brief Load JS file
     * @warning Not accessible in JS/Lua!
     * @param file JS file path
     * @param is_module Whether the js import modules or not 
     * @return success or not
     */
    bool ngenxx_js_loadF(const char *file, bool is_module);

    /**
     * @brief Load JS Script
     * @warning Not accessible in JS/Lua!
     * @param script JS script
     * @param name JS file name
     * @param is_module Whether the js import modules or not 
     * @return success or not
     */
    bool ngenxx_js_loadS(const char *script, const char *name, bool is_module);

    /**
     * @brief Load JS ByteCode
     * @warning Not accessible in JS/Lua!
     * @param bytes Byte array data
     * @param len Byte array length
     * @param is_module Whether the js import modules or not 
     * @return success or not
     */
    bool ngenxx_js_loadB(const byte *bytes, size_t len, bool is_module);

    /**
     * @brief Call JS function
     * @warning Not accessible in JS/Lua!
     * @param func JS function name
     * @param params JS function params（wrap multiple params with json）
     * @param await Whether wait for the promise result or not
     * @return return value of JS function
     */
    const char *ngenxx_js_call(const char *func, const char *params, bool await);

    /**
     * @brief Set JS msg callback
     * @param callback JS msg callback
     */
    void ngenxx_js_set_msg_callback(const char *(*const callback)(const char *msg));

EXTERN_C_END

#endif // NGENXX_INCLUDE_JS_H_