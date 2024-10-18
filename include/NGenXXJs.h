#ifndef NGENXX_JS_H_
#define NGENXX_JS_H_

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
    bool ngenxx_J_loadF(const char *file);

    /**
     * @brief Load JS Script
     * @warning Not accessible in JS/Lua!
     * @param script JS script
     * @param name JS file name
     * @return success or not
     */
    bool ngenxx_J_loadS(const char *script, const char *name);

    /**
     * @brief Load JS ByteCode
     * @warning Not accessible in JS/Lua!
     * @param bytes Byte array data
     * @param len Byte array length
     * @return success or not
     */
    bool ngenxx_J_loadB(const byte *bytes, const size len);

    /**
     * @brief Call JS function
     * @warning Not accessible in JS/Lua!
     * @param func JS function name
     * @param params JS function params（wrap multiple params with json）
     * @return return value of JS function
     */
    const char *ngenxx_J_call(const char *func, const char *params);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_JS_H_