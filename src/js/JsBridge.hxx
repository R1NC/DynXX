#ifndef NGENXX_SRC_JS_BRIDGE_HXX_
#define NGENXX_SRC_JS_BRIDGE_HXX_

#ifdef __cplusplus

#include <thread>
#include <functional>

#include "../../external/quickjs/quickjs-libc.h"
#include "../../include/NGenXXTypes.hxx"

namespace NGenXX
{
    class JsBridge
    {
    private:
        JSRuntime *runtime;
        JSContext *context;
        std::vector<JSValue> jValues;
        std::thread loopThreadP;
        std::thread loopThreadT;
        std::vector<std::thread> promiseThreadV;
        

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
        bool loadFile(const std::string &file, const bool isModule);

        /**
         * @brief Load JS script
         * @param script JS script
         * @param name JS file name
         * @return success or not
         */
        bool loadScript(const std::string &script, const std::string &name, const bool isModule);

        /**
         * @brief Load JS ByteCode
         * @param bytes JS ByteCode
         */
        bool loadBinary(Bytes bytes, const bool isModule);

        /**
         * @brief call JS func
         * @param func func name
         * @param params parameters(json)
         */
        std::string callFunc(const std::string &func, const std::string &params);

        /**
         * @brief New JS `Promise`
         * @param f Callback to do work in background
         * @return JSValue of the `Promise`
         */        
        JSValue newPromiseVoid(std::function<void()> f);
        JSValue newPromiseBool(std::function<const bool()> f);
        JSValue newPromiseInt32(std::function<const int()> f);
        JSValue newPromiseInt64(std::function<const long long()> f);
        JSValue newPromiseFloat(std::function<const double()> f);
        JSValue newPromiseString(std::function<const std::string()> f);

        /**
         * Relase JS VM
         */
        ~JsBridge();
    };
}

#endif

#endif // NGENXX_SRC_JS_BRIDGE_HXX_