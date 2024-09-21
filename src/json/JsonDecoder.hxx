#ifndef NGENXX_JSON_DECODER_HXX_
#define NGENXX_JSON_DECODER_HXX_

#include "../../external/cjson/cJSON.h"

#ifdef __cplusplus

#include <string>

namespace NGenXX
{
    namespace Json
    {
        class Decoder
        {
        private:
            cJSON *cjson;

            cJSON *parseNode(void *node);

        public:
            Decoder() = delete;

            Decoder(const std::string &json);

            bool isArray(void *node);

            bool isObject(void *node);

            void *readChild(void *node);

            void *readNext(void *node);

            void *readNode(void *node, const std::string &k);

            std::string readString(void *node);

            double readNumber(void *node);

            ~Decoder();
        };
    }
}

#endif

#endif // NGENXX_JSON_DECODER_HXX_