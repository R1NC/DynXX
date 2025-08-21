#ifndef DYNXX_INCLUDE_JSON_HXX_
#define DYNXX_INCLUDE_JSON_HXX_

#include <functional>

#include "Types.hxx"

enum class DynXXJsonNodeTypeX : int {
    Unknown,
    Object,
    Array,
    String,
    Int32,
    Int64,
    Float,
    Boolean,
    Null,
};

DynXXJsonNodeTypeX dynxxJsonReadType(void *const json);

std::optional<std::string> dynxxJsonToStr(void *const cjson);

std::optional<std::string> dynxxJsonFromDictAny(const DictAny &dict);

std::optional<DictAny> dynxxJsonToDictAny(const std::string &json);

void *dynxxJsonDecoderInit(const std::string_view json);

void *dynxxJsonDecoderReadNode(void *const decoder, std::string_view k, void *const node = nullptr);

std::optional<std::string> dynxxJsonDecoderReadString(void *const decoder, void *const node = nullptr);

std::optional<double> dynxxJsonDecoderReadNumber(void *const decoder, void *const node = nullptr);

void *dynxxJsonDecoderReadChild(void *const decoder, void *const node = nullptr);

size_t dynxxJsonDecoderReadChildrenCount(void *const decoder, void *const node = nullptr);

void dynxxJsonDecoderReadChildren(void *const decoder, std::function<void(size_t idx, const void *child)> &&callback,
                                   void *const node = nullptr);

void *dynxxJsonDecoderReadNext(void *const decoder, void *const node = nullptr);

void dynxxJsonDecoderRelease(void *const decoder);

#endif // DYNXX_INCLUDE_JSON_HXX_
