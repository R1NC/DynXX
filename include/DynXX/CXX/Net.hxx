#ifndef DYNXX_INCLUDE_NET_HXX_
#define DYNXX_INCLUDE_NET_HXX_

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
    std::unordered_map<std::string, std::string> headers;
    std::string data;

    [[nodiscard]] std::optional<std::string> toJson() const;
};

using DynXXHttpResponse = DynXXHttpResponse;

DynXXHttpResponse dynxxNetHttpRequest(std::string_view url,
                                        DynXXHttpMethodX method,
                                        std::string_view params,
                                        const BytesView rawBody = {},
                                        const std::vector<std::string> &headerV = {},
                                        const std::vector<std::string> &formFieldNameV = {},
                                        const std::vector<std::string> &formFieldMimeV = {},
                                        const std::vector<std::string> &formFieldDataV = {},
                                        const std::FILE *cFILE = nullptr, size_t fileSize = 0,
                                        size_t timeout = DynXXHttpDefaultTimeout);

DynXXHttpResponse dynxxNetHttpRequest(std::string_view url,
                                        DynXXHttpMethodX method,
                                        const std::unordered_map<std::string, Any> &params,
                                        const BytesView rawBody = {},
                                        const std::unordered_map<std::string, std::string> &headers = {},
                                        const std::vector<std::string> &formFieldNameV = {},
                                        const std::vector<std::string> &formFieldMimeV = {},
                                        const std::vector<std::string> &formFieldDataV = {},
                                        const std::FILE *cFILE = nullptr, size_t fileSize = 0,
                                        size_t timeout = DynXXHttpDefaultTimeout);

bool dynxxNetHttpDownload(std::string_view url, const std::string &filePath,
                           size_t timeout = DynXXHttpDefaultTimeout);

#endif // DYNXX_INCLUDE_NET_HXX_
