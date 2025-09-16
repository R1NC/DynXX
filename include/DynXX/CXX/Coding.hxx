#ifndef DYNXX_INCLUDE_CXX_CODING_HXX_
#define DYNXX_INCLUDE_CXX_CODING_HXX_

#include "Types.hxx"

std::string dynxxCodingCaseUpper(std::string_view str);

std::string dynxxCodingCaseLower(std::string_view str);

std::string dynxxCodingHexBytes2str(BytesView bytes);

Bytes dynxxCodingHexStr2bytes(const std::string_view str);

Bytes dynxxCodingStr2bytes(std::string_view str);

std::string dynxxCodingBytes2str(BytesView bytes);

std::string dynxxCodingStrTrim(std::string_view str);

#endif // DYNXX_INCLUDE_CXX_CODING_HXX_
