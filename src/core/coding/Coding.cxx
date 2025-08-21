#include "Coding.hxx"

#include <algorithm>
#include <charconv>
#if defined(__cpp_lib_ranges)
#include <ranges>
#include <numeric>
#endif

std::string DynXX::Core::Coding::Case::upper(std::string_view str)
{
    std::string s(str);
    s.reserve(str.size());
#if defined(__cpp_lib_ranges)
    std::ranges::transform(s, s.begin(), toupper);
#else
    std::transform(s.begin(), s.end(), s.begin(), toupper);
#endif
    return s;
}

std::string DynXX::Core::Coding::Case::lower(std::string_view str)
{
    std::string s(str);
    s.reserve(str.size());
#if defined(__cpp_lib_ranges)
    std::ranges::transform(s, s.begin(), tolower);
#else
    std::transform(s.begin(), s.end(), s.begin(), tolower);
#endif
    return s;
}

std::string DynXX::Core::Coding::Hex::bytes2str(BytesView bytes)
{
    if (bytes.empty()) [[unlikely]]
    {
        return {};
    }
    std::string str;
    str.resize(bytes.size() * 2);

    auto transF = [](const byte b, char *buf) {
        if (auto [ptr, errCode] = std::to_chars(buf, buf + 2, static_cast<int>(b), 16); errCode != std::errc()) [[unlikely]]
        {
            buf[0] = '0';
            buf[1] = '0';
        }
        else if (ptr - buf == 1) // Zero-pad for single-digit results
        {
            buf[1] = buf[0];
            buf[0] = '0';
        }
    };

#if defined(__cpp_lib_ranges)
#if defined(__cpp_lib_ranges_enumerate)
    auto hexView = bytes | std::ranges::views::enumerate
#else
    auto indices = std::views::iota(size_t{0}, bytes.size());
    auto hexView = std::views::zip(indices, bytes)
#endif
        | std::ranges::views::transform([&str, &transF](const auto& pair) {
            const auto& [idx, b] = pair;
            transF(b, &str[idx * 2]);
            return std::string_view(&str[idx * 2], 2);
        });
    return std::accumulate(hexView.begin(), hexView.end(), std::string{},
        [](std::string&& acc, std::string_view sv) {
            return std::move(acc) + std::string(sv);
        });
#else
    size_t idx = 0;
    for (const auto& b : bytes) {
        transF(b, &str[idx * 2]);
        idx++;
    }
    return str;
#endif
}

Bytes DynXX::Core::Coding::Hex::str2bytes(const std::string &str)
{
    std::string filteredStr;
    filteredStr.reserve(str.size());

#if defined(__cpp_lib_ranges)
    std::ranges::copy_if(str, std::back_inserter(filteredStr),
                         [](const char c) { return std::isxdigit(static_cast<unsigned char>(c)); });
#else
    std::copy_if(str.begin(), str.end(), std::back_inserter(filteredStr),
                [](const char c) { return std::isxdigit(static_cast<unsigned char>(c)); });
#endif

    if (filteredStr.length() < 2) [[unlikely]]
    {
        return {};
    }

    std::string fixedStr = str;
    if (fixedStr.length() % 2 != 0) [[unlikely]]
    {
        fixedStr.insert(0, "0");
    }
    auto sLen = str.length();
      
    auto transF = [](const auto& chunk) { 
        std::string s(chunk.begin(), chunk.end());
        auto hex = 0;
        if (auto [_, errCode] = std::from_chars(s.data(), s.data() + s.size(), hex, 16); errCode != std::errc()) [[unlikely]]
        {
            return static_cast<byte>(0);
        }
        return static_cast<byte>(hex);
    };
#if defined(__cpp_lib_ranges_chunk)
    auto byteView = fixedStr 
        | std::ranges::views::chunk(2) 
        | std::ranges::views::transform(transF);
    return Bytes(byteView.begin(), byteView.end());
#else
    Bytes bytes;
    bytes.reserve(sLen / 2);
    for (decltype(sLen) i(0); i < sLen; i += 2)
    {
        auto s = fixedStr.substr(i, 2);
        bytes.emplace_back(transF(s));
    }
    return bytes;
#endif
}

std::string DynXX::Core::Coding::bytes2str(BytesView bytes)
{
    return {bytes.begin(), bytes.end()};
}

Bytes DynXX::Core::Coding::str2bytes(std::string_view str)
{
    return {str.begin(), str.end()};
}

std::string DynXX::Core::Coding::strTrim(std::string_view str) 
{
#if defined(__cpp_lib_ranges)
    auto findSpaceF = [](char c) { return std::isspace(static_cast<int>(c)); };
    auto trimmed = str 
        | std::ranges::views::drop_while(findSpaceF)
        | std::ranges::views::reverse
        | std::ranges::views::drop_while(findSpaceF)
        | std::ranges::views::reverse;
    return std::string(trimmed.begin(), trimmed.end());
#else
    constexpr auto invalidChars = " \t\n\r\f\v";
    const auto begin = str.find_first_not_of(invalidChars);
    if (begin == std::string_view::npos) [[unlikely]]
    {
        return {};
    }
    const auto end = str.find_last_not_of(invalidChars);
    return std::string(str.substr(begin, end - begin + 1));
#endif
}

std::string DynXX::Core::Coding::strEscapeQuotes(const std::string_view& str) 
{
    std::string result;
    result.reserve(str.size() * 2);
    static const std::string_view escapeChars = R"("\\")";
    
#if defined(__cpp_lib_ranges)
    std::ranges::for_each(str, [&result](char c) 
#else
    for (char c : str) 
#endif
    {
        if (c == '"') 
        {
            result.append(escapeChars);
        } 
        else 
        {
            result.append(1, c);
        }
    }
#if defined(__cpp_lib_ranges)
    );
#endif

    return result;
}
