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

            cJSON *parseNode(void *const node);

        public:
            Decoder() = delete;

            Decoder(const std::string &json);

            bool isArray(void *const node);

            bool isObject(void *const node);

            void *const readChild(void *const node);

            void *const readNext(void *const node);

            void readChildren(void *const node, std::function<void(int idx, void* child)> callback);

            void *const readNode(void *const node, const std::string &k);

            std::string readString(void *const node);

            double readNumber(void *const node);

            ~Decoder();
        };
    }
}

#endif

#endif // NGENXX_JSON_DECODER_HXX_