#ifndef NGENXX_JS_H_
#define NGENXX_JS_H_

/**
 * @brief Load JS file
 * @warning Not accessible in JS!
 * @param file JS file path
 * @return success or not
 */
bool ngenxx_J_loadF(const char *file);

/**
 * @brief Call JS function
 * @warning Not accessible in JS!
 * @param func JS function name
 * @param params JS function params（wrap multiple params with json）
 * @return return value of JS function
 */
const char *ngenxx_J_call(const char *func, const char *params);

#endif // NGENXX_JS_H_