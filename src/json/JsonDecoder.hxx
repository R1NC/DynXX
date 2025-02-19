#ifndef NGENXX_SRC_JSON_DECODER_HXX_
#define NGENXX_SRC_JSON_DECODER_HXX_

#include <cJSON.h>

#if defined(__cplusplus)

#include <string>
#include <functional>
#include <unordered_map>

#include <NGenXXTypes.hxx>

namespace NGenXX
{
    namespace Json
    {
        std::unordered_map<std::string, std::string> dictFromJson(const std::string &json);
        std::string dictToJson(const std::unordered_map<std::string, std::string> &dict);
        std::unordered_map<std::string, Any> dictAnyFromJson(const std::string &json);
        std::string dictAnyToJson(const std::unordered_map<std::string, Any> &dict);
        
        class Decoder
        {
        public:
            Decoder() = delete;
            explicit Decoder(const std::string &json);
            Decoder(const Decoder &) = delete;
            Decoder &operator=(const Decoder &) = delete;
            Decoder(Decoder &&) noexcept;
            Decoder &operator=(Decoder &&) noexcept;

            bool isArray(const void *const node);

            bool isObject(const void *const node);

            void *readChild(const void *const node);

            void *readNext(const void *const node);

            void readChildren(const void *const node, const std::function<void(const size_t idx, const void *const child)> &callback);

            void *readNode(const void *const node, const std::string &k);
            void *operator[](const std::string &k)
            {
                return this->readNode(NULL, k);
            }

            const std::string readString(const void *const node);

            double readNumber(const void *const node);

            ~Decoder();

        private:
            cJSON *cjson{NULL};

            void moveImp(Decoder&& other) noexcept;
            void cleanup();

            const cJSON *parseNode(const void *const node);
        };
    }
}

#endif

#endif // NGENXX_SRC_JSON_DECODER_HXX_