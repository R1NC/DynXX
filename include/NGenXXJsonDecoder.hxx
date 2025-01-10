#ifndef NGENXX_INCLUDE_JSON_DECODER_HXX_
#define NGENXX_INCLUDE_JSON_DECODER_HXX_

#include <string>

void *ngenxxJsonDecoderInit(const std::string &json);

bool ngenxxJsonDecoderIsArray(void *const decoder, void *const node = nullptr);

bool ngenxxJsonDecoderIsObject(void *const decoder, void *const node = nullptr);

void *ngenxxJsonDecoderReadNode(void *const decoder, const std::string &k, void *const node = nullptr);

const std::string ngenxxJsonDecoderReadString(void *const decoder, void *const node = nullptr);

double ngenxxJsonDecoderReadNumber(void *const decoder, void *const node = nullptr);

void *ngenxxJsonDecoderReadChild(void *const decoder, void *const node = nullptr);

void *ngenxxJsonDecoderReadNext(void *const decoder, void *const node = nullptr);

void ngenxxJsonDecoderRelease(void *const decoder);

#endif // NGENXX_INCLUDE_JSON_DECODER_HXX_