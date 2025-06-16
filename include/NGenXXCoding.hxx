#ifndef NGENXX_INCLUDE_CODING_HXX_
#define NGENXX_INCLUDE_CODING_HXX_

#include <string_view>

#include "NGenXXTypes.hxx"

std::string ngenxxCodingCaseUpper(const std::string_view &str);

std::string ngenxxCodingCaseLower(const std::string_view &str);

std::string ngenxxCodingHexBytes2str(const Bytes &bytes);

Bytes ngenxxCodingHexStr2bytes(const std::string &str);

Bytes ngenxxCodingStr2bytes(const std::string_view &str);

std::string ngenxxCodingBytes2str(const Bytes &bytes);

std::string ngenxxCodingStrTrim(const std::string_view &str);

#endif // NGENXX_INCLUDE_CODING_HXX_
