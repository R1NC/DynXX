#pragma once

#include <cJSON.h>

#include <functional>

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

        bool valid() const;

        DynXXJsonNodeHandle readChild(const DynXXJsonNodeHandle node) const;

        DynXXJsonNodeHandle readNext(const DynXXJsonNodeHandle node) const;

        size_t readChildrenCount(const DynXXJsonNodeHandle node) const;

        void readChildren(const DynXXJsonNodeHandle node, std::function<void(size_t idx, const DynXXJsonNodeHandle childNode, const DynXXJsonNodeTypeX childType, std::string_view childName)> &&callback) const;

        DynXXJsonNodeHandle readNode(const DynXXJsonNodeHandle node, std::string_view k) const;

        DynXXJsonNodeHandle operator[](std::string_view k) const;

        std::optional<std::string> readString(const DynXXJsonNodeHandle node) const;

        std::optional<Num> readNumber(const DynXXJsonNodeHandle node) const;

        template <typename T> 
        requires (IntegerT<T> || EnumT<T>)
        std::optional<T> readNumInt(const DynXXJsonNodeHandle node) const {
            const auto v = this->readNumber(node);
            if (v == std::nullopt) [[unlikely]] {
                return std::nullopt;
            }
            const auto i = *std::get_if<int64_t>(&v.value());
            return {static_cast<T>(i)};
        }

        template <typename T> 
        requires FloatT<T>
        std::optional<T> readNumFloat(const DynXXJsonNodeHandle node) const {
            const auto v = this->readNumber(node);
            if (v == std::nullopt) [[unlikely]] {
                return std::nullopt;
            }
            const auto f = *std::get_if<double>(&v.value());
            return {static_cast<T>(f)};
        }

        ~Decoder();

    private:
        cJSON *cjson{nullptr};

        void moveImp(Decoder &&other) noexcept;

        void cleanup() noexcept;

        const cJSON *reinterpretNode(const DynXXJsonNodeHandle node) const;
    };
}
