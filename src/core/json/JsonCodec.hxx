#ifndef DYNXX_SRC_CORE_JSON_CODEC_HXX_
#define DYNXX_SRC_CORE_JSON_CODEC_HXX_

#include <cJSON.h>

#if defined(__cplusplus)

#include <functional>

#include <DynXX/CXX/Types.hxx>
#include <DynXX/CXX/Json.hxx>

namespace DynXX::Core::Json {
    DynXXJsonNodeTypeX cJSONReadType(void *const cjson);
    
    std::optional<std::string> cJSONReadName(void *const cjson);

    std::optional<std::string> cJSONToStr(void *const cjson);

    std::optional<std::string> jsonFromDictAny(const DictAny &dict);

    std::optional<DictAny> jsonToDictAny(const std::string &json);

    class Decoder {
    public:
        Decoder() = delete;

        explicit Decoder(const std::string_view json);

        Decoder(const Decoder &) = delete;

        Decoder &operator=(const Decoder &) = delete;

        Decoder(Decoder &&) noexcept;

        Decoder &operator=(Decoder &&) noexcept;

        void *readChild(void *const node) const;

        void *readNext(void *const node) const;

        size_t readChildrenCount(void *const node) const;

        void readChildren(void *const node, std::function<void(size_t idx, void *const childNode, const DynXXJsonNodeTypeX childType, const std::optional<std::string> childName)> &&callback) const;

        void *readNode(void *const node, std::string_view k) const;

        void *operator[](std::string_view k) const;

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

#endif // DYNXX_SRC_CORE_JSON_CODEC_HXX_
