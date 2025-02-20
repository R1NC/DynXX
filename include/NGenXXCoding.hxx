#ifndef NGENXX_INCLUDE_CODING_HXX_
#define NGENXX_INCLUDE_CODING_HXX_

#include "NGenXXTypes.hxx"

std::string ngenxxCodingHexBytes2str(const Bytes &bytes);

Bytes ngenxxCodingHexStr2bytes(const std::string &str);

Bytes ngenxxCodingStr2bytes(const std::string &str);

std::string ngenxxCodingBytes2str(const Bytes &bytes);

std::string ngenxxCodingCaseUpper(const std::string &str);

std::string ngenxxCodingCaseLower(const std::string &str);

#endif // NGENXX_INCLUDE_CODING_HXX_