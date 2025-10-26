#pragma once

#include "Types.hxx"
#include <cstdio>

constexpr auto DynXXHttpDefaultTimeout = 15 * 1000;

enum class DynXXHttpMethodX : uint8_t {
    Get,
    Post,
    Put,
};

struct DynXXHttpResponse {
    std::string contentType;
    std::string data;
    Dict headers;
    int code{0};

    std::optional<std::string> toJson() const;
};
#if !defined(_WIN32)
AssertMove(DynXXHttpResponse);
#endif

DynXXHttpResponse dynxxNetHttpRequest(std::string_view url,
                                        DynXXHttpMethodX method,
                                        std::string_view params,
                                        BytesView rawBody = {},
                                        const std::vector<std::string> &headerV = {},
                                        const std::vector<std::string> &formFieldNameV = {},
                                        const std::vector<std::string> &formFieldMimeV = {},
                                        const std::vector<std::string> &formFieldDataV = {},
                                        std::FILE *cFILE = nullptr, size_t fileSize = 0,
                                        size_t timeout = DynXXHttpDefaultTimeout);

DynXXHttpResponse dynxxNetHttpRequest(std::string_view url,
                                        DynXXHttpMethodX method,
                                        const DictAny &params,
                                        BytesView rawBody = {},
                                        const Dict &headers = {},
                                        const std::vector<std::string> &formFieldNameV = {},
                                        const std::vector<std::string> &formFieldMimeV = {},
                                        const std::vector<std::string> &formFieldDataV = {},
                                        std::FILE *cFILE = nullptr, size_t fileSize = 0,
                                        size_t timeout = DynXXHttpDefaultTimeout);

bool dynxxNetHttpDownload(std::string_view url, std::string_view filePath,
                           size_t timeout = DynXXHttpDefaultTimeout);
