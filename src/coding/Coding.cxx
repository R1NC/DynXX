#include "Coding.hxx"
#include <iomanip>
#include <sstream>
#include <cstring>

const std::string NGenXX::Coding::Hex::bytes2str(const NGenXX::Bytes bytes)
{
    const byte *byt = bytes.first;
    const size len = bytes.second;
    if (byt == NULL || len <= 0)
        return "";
    std::stringstream strStream;
    strStream << std::hex;
    for (int i(0); i < len; ++i)
        strStream << std::setw(2) << std::setfill('0') << (int)byt[i];
    return strStream.str();
}

const NGenXX::Bytes NGenXX::Coding::Hex::str2bytes(const std::string &str)
{
    if (str.length() == 0) return BytesEmpty;
    size outLen = str.length();
    byte outBytes[outLen];
    std::memset(outBytes, 0, outLen);
    size j(0);
    for (size i(0); i < str.length(); i += 2, j++) {
        auto s = str.substr(i, 2);
        outBytes[j] = (unsigned char)std::stoi(s.c_str(), nullptr, 16);
    }
    return {outBytes, j};
}