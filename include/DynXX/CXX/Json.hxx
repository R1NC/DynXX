#pragma once

#include <functional>

#include "Types.hxx"
#include "../C/Json.h"

enum class DynXXJsonNodeTypeX : uint8_t {
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

using DynXXJsonDecoderReadChildrenCallback = std::function<void(size_t idx, DynXXJsonNodeHandle childNode, DynXXJsonNodeTypeX childType, std::string_view childName)>;

DynXXJsonNodeTypeX dynxxJsonNodeReadType(DynXXJsonNodeHandle node);

std::optional<std::string> dynxxJsonNodeReadName(DynXXJsonNodeHandle node);

std::optional<std::string> dynxxJsonNodeToStr(DynXXJsonNodeHandle node);

std::optional<std::string> dynxxJsonFromDictAny(const DictAny &dict);

std::optional<DictAny> dynxxJsonToDictAny(const std::string &json);

DynXXJsonDecoderHandle dynxxJsonDecoderInit(std::string_view json);

DynXXJsonNodeHandle dynxxJsonDecoderReadNode(DynXXJsonDecoderHandle decoder, std::string_view k, DynXXJsonNodeHandle node = 0);

std::optional<std::string> dynxxJsonDecoderReadString(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node = 0);

std::optional<int64_t> dynxxJsonDecoderReadInteger(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node = 0);

std::optional<double> dynxxJsonDecoderReadFloat(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node = 0);

DynXXJsonNodeHandle dynxxJsonDecoderReadChild(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node = 0);

size_t dynxxJsonDecoderReadChildrenCount(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node = 0);

void dynxxJsonDecoderReadChildren(DynXXJsonDecoderHandle decoder, DynXXJsonDecoderReadChildrenCallback &&callback, DynXXJsonNodeHandle node = 0);

DynXXJsonNodeHandle dynxxJsonDecoderReadNext(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node = 0);

void dynxxJsonDecoderRelease(DynXXJsonDecoderHandle decoder);
