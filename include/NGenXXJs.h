#ifndef NGENXX_INCLUDE_JS_H_
#define NGENXX_INCLUDE_JS_H_

#include "NGenXXTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Load JS file
     * @warning Not accessible in JS/Lua!
     * @param file JS file path
     * @return success or not
     */
    bool ngenxx_js_loadF(const char *file);

    /**
     * @brief Load JS Script
     * @warning Not accessible in JS/Lua!
     * @param script JS script
     * @param name JS file name
     * @return success or not
     */
    bool ngenxx_js_loadS(const char *script, const char *name);

    /**
     * @brief Load JS ByteCode
     * @warning Not accessible in JS/Lua!
     * @param bytes Byte array data
     * @param len Byte array length
     * @return success or not
     */
    bool ngenxx_js_loadB(const byte *bytes, const size_t len);

    /**
     * @brief Call JS function
     * @warning Not accessible in JS/Lua!
     * @param func JS function name
     * @param params JS function params（wrap multiple params with json）
     * @return return value of JS function
     */
    const char * ngenxx_js_call(const char *func, const char *params);

    /**
     * @brief Set JS msg callback
     * @param callback JS msg callback
     */
    void ngenxx_js_set_msg_callback(const char *(*callback)(const char *msg));

#ifdef __cplusplus
}
#endif

#endif // NGENXX_INCLUDE_JS_H_