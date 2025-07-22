#ifndef NGENXX_INCLUDE_CODING_HXX_
#define NGENXX_INCLUDE_CODING_HXX_

#include "NGenXXTypes.hxx"

std::string ngenxxCodingCaseUpper(std::string_view str);

std::string ngenxxCodingCaseLower(std::string_view str);

std::string ngenxxCodingHexBytes2str(BytesView bytes);

Bytes ngenxxCodingHexStr2bytes(const std::string &str);

Bytes ngenxxCodingStr2bytes(std::string_view str);

std::string ngenxxCodingBytes2str(BytesView bytes);

std::string ngenxxCodingStrTrim(std::string_view str);

#endif // NGENXX_INCLUDE_CODING_HXX_
