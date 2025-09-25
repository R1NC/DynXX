#pragma once

#include "Types.hxx"

constexpr size_t DynXXHttpDefaultTimeout = 15 * 1000;

enum class DynXXHttpMethodX : int {
    Get,
    Post,
    Put,
};

struct DynXXHttpResponse {
    int code{0};
    std::string contentType;
    Dict headers;
    std::string data;

    std::optional<std::string> toJson() const;
};
#ifndef _WIN32
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
