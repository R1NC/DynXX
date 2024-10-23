#ifndef NGENXX_JSON_DECODER_HXX_
#define NGENXX_JSON_DECODER_HXX_

#include "../../external/cjson/cJSON.h"

#ifdef __cplusplus

#include <string>
#include <functional>

namespace NGenXX
{
    namespace Json
    {
        class Decoder
        {
        private:
            cJSON *cjson;

            cJSON *parseNode(const void *node);

        public:
            Decoder() = delete;

            Decoder(const std::string &json);

            bool isArray(const void *node);

            bool isObject(const void *node);

            void *readChild(const void *node);

            void *readNext(const void *node);

            void readChildren(const void *node, std::function<void(int idx, void* child)> callback);

            void *readNode(const void *node, const std::string &k);

            std::string readString(const void *node);

            double readNumber(const void *node);

            ~Decoder();
        };
    }
}

#endif

#endif // NGENXX_JSON_DECODER_HXX_