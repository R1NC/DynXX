#ifndef DYNXX_INCLUDE_JSON_HXX_
#define DYNXX_INCLUDE_JSON_HXX_

#include <functional>

#include "Types.hxx"
#include "../C/Json.h"

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

DynXXJsonNodeTypeX dynxxJsonNodeReadType(const DynXXJsonNodeHandle node);

std::optional<std::string> dynxxJsonNodeReadName(const DynXXJsonNodeHandle node);

std::optional<std::string> dynxxJsonNodeToStr(const DynXXJsonNodeHandle node);

std::optional<std::string> dynxxJsonFromDictAny(const DictAny &dict);

std::optional<DictAny> dynxxJsonToDictAny(const std::string &json);

DynXXJsonDecoderHandle dynxxJsonDecoderInit(const std::string_view json);

DynXXJsonNodeHandle dynxxJsonDecoderReadNode(const DynXXJsonDecoderHandle decoder, std::string_view k, const DynXXJsonNodeHandle node = nullptr);

std::optional<std::string> dynxxJsonDecoderReadString(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node = nullptr);

std::optional<double> dynxxJsonDecoderReadNumber(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node = nullptr);

DynXXJsonNodeHandle dynxxJsonDecoderReadChild(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node = nullptr);

size_t dynxxJsonDecoderReadChildrenCount(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node = nullptr);

void dynxxJsonDecoderReadChildren(const DynXXJsonDecoderHandle decoder, std::function<void(size_t idx, const DynXXJsonNodeHandle childNode, const DynXXJsonNodeTypeX childType, const std::optional<std::string> childName)> &&callback,
                                   const DynXXJsonNodeHandle node = nullptr);

DynXXJsonNodeHandle dynxxJsonDecoderReadNext(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node = nullptr);

void dynxxJsonDecoderRelease(const DynXXJsonDecoderHandle decoder);

#endif // DYNXX_INCLUDE_JSON_HXX_
