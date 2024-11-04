#ifndef NGENXX_INCLUDE_CODING_HXX_
#define NGENXX_INCLUDE_CODING_HXX_

#include "NGenXXTypes.hxx"

const std::string ngenxxCodingHexBytes2str(const Bytes bytes);

const Bytes ngenxxCodingHexStr2bytes(const std::string &str);

const Bytes ngenxxCodingStr2bytes(const std::string &str);

const std::string ngenxxCodingBytes2str(const Bytes bytes);

const std::string ngenxxCodingCaseUpper(const std::string &str);

const std::string ngenxxCodingCaseLower(const std::string &str);

#endif // NGENXX_INCLUDE_CODING_HXX_