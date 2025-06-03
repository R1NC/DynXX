#ifndef NGENXX_INCLUDE_JSON_CODEC_HXX_
#define NGENXX_INCLUDE_JSON_CODEC_HXX_

#include <string>
#include <functional>

#include "NGenXXTypes.hxx"

enum class NGenXXJsonNodeTypeX : int
{
    Unknown,
    Object,
    Array,
    String,
    Number,
    Boolean,
    Null,
};

NGenXXJsonNodeTypeX ngenxxJsonReadType(void *const json);
std::string ngenxxJsonToStr(void *const cjson);

std::string ngenxxJsonFromDictAny(const DictAny &dict);
DictAny ngenxxJsonToDictAny(const std::string &json);

void *ngenxxJsonDecoderInit(const std::string &json);

void *ngenxxJsonDecoderReadNode(void *const decoder, const std::string &k, void *const node = nullptr);

std::string ngenxxJsonDecoderReadString(void *const decoder, void *const node = nullptr);

double ngenxxJsonDecoderReadNumber(void *const decoder, void *const node = nullptr);

void *ngenxxJsonDecoderReadChild(void *const decoder, void *const node = nullptr);

void ngenxxJsonDecoderReadChildren(void *const decoder, std::function<void(size_t idx, const void * child)> &&callback, void *const node = nullptr);

void *ngenxxJsonDecoderReadNext(void *const decoder, void *const node = nullptr);

void ngenxxJsonDecoderRelease(void *const decoder);

#endif // NGENXX_INCLUDE_JSON_CODEC_HXX_