#ifndef NGENXX_INCLUDE_JSON_DECODER_HXX_
#define NGENXX_INCLUDE_JSON_DECODER_HXX_

#include <string>

void *const ngenxxJsonDecoderInit(const std::string &json);

bool ngenxxJsonDecoderIsArray(void *const decoder, void *const node);

bool ngenxxJsonDecoderIsObject(void *const decoder, void *const node);

void *const ngenxxJsonDecoderReadNode(void *const decoder, void *const node, const std::string &k);

const std::string ngenxxJsonDecoderReadString(void *const decoder, void *const node);

double ngenxxJsonDecoderReadNumber(void *const decoder, void *const node);

void *const ngenxxJsonDecoderReadChild(void *const decoder, void *const node);

void *const ngenxxJsonDecoderReadNext(void *const decoder, void *const node);

void ngenxxJsonDecoderRelease(void *const decoder);

#endif //  NGENXX_INCLUDE_JSON_DECODER_HXX_