#include "Coding.hxx"

#include <iomanip>
#include <sstream>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <vector>

std::string NGenXX::Coding::Hex::bytes2str(const Bytes &bytes)
{
    if (bytes.empty()) [[unlikely]]
    {
        return "";
    }
    std::stringstream strStream;
    strStream << std::hex;
    for (byte byte : bytes)
    {
        strStream << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return strStream.str();
}

Bytes NGenXX::Coding::Hex::str2bytes(const std::string &str)
{
    if (str.length() == 0) [[unlikely]]
    {
        return BytesEmpty;
    }
    auto outLen = str.length();
    Bytes bytes;
    size_t j(0);
    for (size_t i(0); i < str.length(); i += 2, j++)
    {
        auto s = str.substr(i, 2);
        bytes.push_back(static_cast<byte>(std::stoi(s.c_str(), nullptr, 16)));
    }
    return bytes;
}

std::string NGenXX::Coding::Case::upper(const std::string &str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

std::string NGenXX::Coding::Case::lower(const std::string &str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::string NGenXX::Coding::bytes2str(const Bytes &bytes)
{
    return std::string(bytes.begin(), bytes.end());
}

Bytes NGenXX::Coding::str2bytes(const std::string &str)
{
    return Bytes(str.begin(), str.end());
}