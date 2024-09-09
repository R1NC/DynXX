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
        JsBridge();

        /**
         *
         */
        bool addModule(const std::string &module, JSModuleInitFunc *callback, const JSCFunctionListEntry* funcList);

        /**
         * @brief Load JS file
         * @param file JS file path
         * @return success or not
         */
        bool loadFile(const std::string &file);

        /**
         *
         */
        std::string callFunc(const std::string &func, const std::string &params);

        /**
         *
         */
        ~JsBridge();
    };
}

#endif

#endif // NGENXX_JS_BRIDGE_HXX_