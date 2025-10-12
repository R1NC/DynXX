#pragma once

#include "Types.hxx"

std::string dynxxCodingCaseUpper(std::string_view str);

std::string dynxxCodingCaseLower(std::string_view str);

std::string dynxxCodingHexBytes2str(BytesView bytes);

Bytes dynxxCodingHexStr2bytes(std::string_view str);

Bytes dynxxCodingStr2bytes(std::string_view str);

std::string dynxxCodingBytes2str(BytesView bytes);

std::string dynxxCodingStrTrim(std::string_view str);
