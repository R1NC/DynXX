#ifndef NGENXX_JS_BRIDGE_HXX_
#define NGENXX_JS_BRIDGE_HXX_

#ifdef __cplusplus

#include <string>

#include "../../external/quickjs/quickjs-libc.h"

namespace NGenXX
{
    class JsBridge
    {
    private:
        JSRuntime *runtime;
        JSContext *context;

    public:
        /**
         * Create JS VM
         */
        JsBridge();

        /**
         * @brief Export C func for JS
         * @param funcJ func name
         * @param funcC func implemention
         * @return success or not
         */
        bool bindFunc(const std::string &funcJ, JSCFunction *funcC);

        /**
         * @brief Load JS file
         * @param file JS file path
         * @return success or not
         */
        bool loadFile(const std::string &file);

        /**
         * @brief call JS func
         * @param func func name
         * @param params parameters(json)
         */
        std::string callFunc(const std::string &func, const std::string &params);

        /**
         * Relase JS VM
         */
        ~JsBridge();
    };
}

#endif

#endif // NGENXX_JS_BRIDGE_HXX_