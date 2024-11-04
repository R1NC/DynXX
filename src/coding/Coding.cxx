#include "Coding.hxx"
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cctype>
#include <algorithm>

const std::string NGenXX::Coding::Hex::bytes2str(const Bytes bytes)
{
    auto [byt, len] = bytes;
    if (byt == NULL || len <= 0)
        return "";
    std::stringstream strStream;
    strStream << std::hex;
    for (int i(0); i < len; ++i)
        strStream << std::setw(2) << std::setfill('0') << (int)byt[i];
    return strStream.str();
}

const Bytes NGenXX::Coding::Hex::str2bytes(const std::string &str)
{
    if (str.length() == 0)
        return BytesEmpty;
    size_t outLen = str.length();
    byte outBytes[outLen];
    std::memset(outBytes, 0, outLen);
    size_t j(0);
    for (size_t i(0); i < str.length(); i += 2, j++)
    {
        auto s = str.substr(i, 2);
        outBytes[j] = static_cast<byte>(std::stoi(s.c_str(), nullptr, 16));
    }
    return {outBytes, j};
}

const std::string NGenXX::Coding::Case::upper(const std::string &str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

const std::string NGenXX::Coding::Case::lower(const std::string &str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

const std::string NGenXX::Coding::bytes2str(const Bytes bytes)
{
    auto [data, len] = bytes;
    return std::string(reinterpret_cast<char*>(const_cast<byte *>(data)));
}

const Bytes NGenXX::Coding::str2bytes(const std::string &str)
{
    if (str.length() == 0)
        return BytesEmpty;
    byte bytes[str.length() + 1];
    std::strcpy(reinterpret_cast<char*>(bytes), str.c_str());
    return {bytes, str.length()};
}