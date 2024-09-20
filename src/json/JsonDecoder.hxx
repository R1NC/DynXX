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

            cJSON *parseObj(void *obj);

        public:
            Decoder(const std::string &json);

            bool isArray(void *obj);

            void *readArray(void *obj);

            void *readArrayNext(void *obj);

            void *readItem(void *obj, const std::string &k);

            std::string readString(void *obj);

            double readNumber(void *obj);

            ~Decoder();
        };
    }
}

#endif

#endif // NGENXX_JSON_DECODER_HXX_