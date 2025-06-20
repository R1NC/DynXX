#ifndef NGENXX_SRC_CORE_JSON_CODEC_HXX_
#define NGENXX_SRC_CORE_JSON_CODEC_HXX_

#include <cJSON.h>

#if defined(__cplusplus)

#include <functional>

#include <NGenXXTypes.hxx>
#include <NGenXXJson.hxx>

namespace NGenXX::Core::Json {
    NGenXXJsonNodeTypeX cJSONReadType(void *const cjson);

    std::optional<std::string> cJSONToStr(void *const cjson);

    std::optional<std::string> jsonFromDictAny(const DictAny &dict);

    std::optional<DictAny> jsonToDictAny(const std::string &json);

    class Decoder {
    public:
        Decoder() = delete;

        explicit Decoder(const std::string &json);

        Decoder(const Decoder &) = delete;

        Decoder &operator=(const Decoder &) = delete;

        Decoder(Decoder &&) noexcept;

        Decoder &operator=(Decoder &&) noexcept;

        void *readChild(void *const node) const;

        void *readNext(void *const node) const;

        void readChildren(void *const node, std::function<void(size_t idx, void *const child)> &&callback) const;

        void *readNode(void *const node, const std::string &k) const;

        void *operator[](const std::string &k) const;

        std::optional<std::string> readString(void *const node) const;

        std::optional<double> readNumber(void *const node) const;

        ~Decoder();

    private:
        cJSON *cjson{nullptr};

        void moveImp(Decoder &&other) noexcept;

        void cleanup() noexcept;

        const cJSON *reinterpretNode(void *const node) const;
    };
}

#endif

#endif // NGENXX_SRC_CORE_JSON_CODEC_HXX_
