#pragma once

#if defined(USE_VCPKG)
#include <cjson/cJSON.h>
#else
#include <cJSON.h>
#endif

#include <functional>

#include <DynXX/CXX/Json.hxx>

namespace DynXX::Core::Json {
    DynXXJsonNodeTypeX nodeReadType(DynXXJsonNodeHandle node);

    std::optional<std::string> nodeReadName(DynXXJsonNodeHandle node);

    std::optional<std::string> nodeToStr(DynXXJsonNodeHandle node);

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

        DynXXJsonNodeHandle readChild(DynXXJsonNodeHandle node) const;

        DynXXJsonNodeHandle readNext(DynXXJsonNodeHandle node) const;

        size_t readChildrenCount(DynXXJsonNodeHandle node) const;

        void readChildren(DynXXJsonNodeHandle node, DynXXJsonDecoderReadChildrenCallback &&callback) const;

        DynXXJsonNodeHandle readNode(DynXXJsonNodeHandle node, std::string_view k) const;

        DynXXJsonNodeHandle operator[](std::string_view k) const;

        std::optional<std::string> readString(DynXXJsonNodeHandle node) const;

        std::optional<Num> readNumber(DynXXJsonNodeHandle node) const;

        template <typename T> 
        requires (IntegerT<T> || EnumT<T>)
        std::optional<T> readNumInt(DynXXJsonNodeHandle node) const {
            const auto v = this->readNumber(node);
            if (v == std::nullopt) [[unlikely]] {
                return std::nullopt;
            }
            const auto i = *std::get_if<int64_t>(&v.value());
            return {static_cast<T>(i)};
        }

        template <typename T> 
        requires FloatT<T>
        std::optional<T> readNumFloat(DynXXJsonNodeHandle node) const {
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

        const cJSON *reinterpretNode(DynXXJsonNodeHandle node) const;
    };
}  // namespace DynXX::Core::Json
