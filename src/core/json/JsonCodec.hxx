#ifndef DYNXX_SRC_CORE_JSON_CODEC_HXX_
#define DYNXX_SRC_CORE_JSON_CODEC_HXX_

#include <cJSON.h>

#if defined(__cplusplus)

#include <functional>

#include <DynXX/CXX/Types.hxx>
#include <DynXX/CXX/Json.hxx>

namespace DynXX::Core::Json {
    DynXXJsonNodeTypeX nodeReadType(const DynXXJsonNodeHandle node);

    std::optional<std::string> nodeReadName(const DynXXJsonNodeHandle node);

    std::optional<std::string> nodeToStr(const DynXXJsonNodeHandle node);

    std::optional<std::string> jsonFromDictAny(const DictAny &dict);

    std::optional<DictAny> jsonToDictAny(const std::string &json);

    class Decoder {
    public:
        Decoder() = delete;

        explicit Decoder(std::string_view json);

        Decoder(const Decoder &) = delete;

        Decoder &operator=(const Decoder &) = delete;

        Decoder(Decoder &&) noexcept;

        Decoder &operator=(Decoder &&) noexcept;

        DynXXJsonNodeHandle readChild(const DynXXJsonNodeHandle node) const;

        DynXXJsonNodeHandle readNext(const DynXXJsonNodeHandle node) const;

        size_t readChildrenCount(const DynXXJsonNodeHandle node) const;

        void readChildren(const DynXXJsonNodeHandle node, std::function<void(size_t idx, const DynXXJsonNodeHandle childNode, const DynXXJsonNodeTypeX childType, const std::optional<std::string> childName)> &&callback) const;

        DynXXJsonNodeHandle readNode(const DynXXJsonNodeHandle node, std::string_view k) const;

        DynXXJsonNodeHandle operator[](std::string_view k) const;

        std::optional<std::string> readString(const DynXXJsonNodeHandle node) const;

        std::optional<double> readNumber(const DynXXJsonNodeHandle node) const;

        ~Decoder();

    private:
        cJSON *cjson{nullptr};

        void moveImp(Decoder &&other) noexcept;

        void cleanup() noexcept;

        const cJSON *reinterpretNode(const DynXXJsonNodeHandle node) const;
    };
}

#endif

#endif // DYNXX_SRC_CORE_JSON_CODEC_HXX_
