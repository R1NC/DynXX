#include "Coding.hxx"

#include <cstring>
#include <cctype>

#include <iomanip>
#include <sstream>
#include <algorithm>
#include <vector>

std::string NGenXX::Coding::Case::upper(const std::string_view &str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

std::string NGenXX::Coding::Case::lower(const std::string_view &str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::string NGenXX::Coding::Hex::bytes2str(const Bytes &bytes)
{
    if (bytes.empty()) [[unlikely]]
    {
        return {};
    }
    std::stringstream ss;
    ss << std::hex;
    for (const auto& b : bytes)
    {
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return ss.str();
}

Bytes NGenXX::Coding::Hex::str2bytes(const std::string &str)
{
    auto sLen = str.length();
    if (sLen < 2) [[unlikely]]
    {
        return {};
    }
    Bytes bytes;
    bytes.reserve(sLen / 2);
    for (decltype(sLen) i(0); i < sLen; i += 2)
    {
        auto s = str.substr(i, 2);
        bytes.push_back(static_cast<byte>(std::stoi(s.c_str(), nullptr, 16)));
    }
    return bytes;
}

std::string NGenXX::Coding::bytes2str(const Bytes &bytes)
{
    return std::string(bytes.begin(), bytes.end());
}

Bytes NGenXX::Coding::str2bytes(const std::string_view &str)
{
    return Bytes(str.begin(), str.end());
}

std::string NGenXX::Coding::strTrim(const std::string_view &str) 
{
    constexpr auto invalidChars = " \t\n\r\f\v"; 
    const auto begin = str.find_first_not_of(invalidChars);
    if (begin == std::string_view::npos) [[unlikely]]
    {
        return {};
    }
    const auto end = str.find_last_not_of(invalidChars);
    return std::string(str.substr(begin, end - begin + 1));
}