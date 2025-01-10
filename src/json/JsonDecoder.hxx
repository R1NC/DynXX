#ifndef NGENXX_SRC_JSON_DECODER_HXX_
#define NGENXX_SRC_JSON_DECODER_HXX_

#include <cJSON.h>

#if defined(__cplusplus)

#include <string>
#include <functional>

namespace NGenXX
{
    namespace Json
    {
        class Decoder
        {
        public:
            Decoder() = delete;
            explicit Decoder(const std::string &json);
            Decoder(const Decoder &) = delete;
            Decoder &operator=(const Decoder &) = delete;

            bool isArray(const void *const node);

            bool isObject(const void *const node);

            void *readChild(const void *const node);

            void *readNext(const void *const node);

            void readChildren(const void *const node, const std::function<void(const size_t idx, const void *const child)> &callback);

            void *readNode(const void *const node, const std::string &k);

            std::string readString(const void *const node);

            double readNumber(const void *const node);

            ~Decoder();

        private:
            cJSON *cjson{NULL};

            const cJSON *parseNode(const void *const node);
        };
    }
}

#endif

#endif // NGENXX_SRC_JSON_DECODER_HXX_