#ifndef NGENXX_INCLUDE_NET_HXX_
#define NGENXX_INCLUDE_NET_HXX_

#include "NGenXXTypes.hxx"

constexpr size_t NGenXXHttpDefaultTimeout = 15 * 1000;

enum class NGenXXHttpMethodX : int {
    Get,
    Post,
    Put,
};

struct NGenXXHttpResponse {
    int code{0};
    std::string contentType;
    std::unordered_map<std::string, std::string> headers;
    std::string data;

    [[nodiscard]] std::optional<std::string> toJson() const;
};

using NGenXXHttpResponse = NGenXXHttpResponse;

NGenXXHttpResponse ngenxxNetHttpRequest(std::string_view url,
                                        NGenXXHttpMethodX method,
                                        std::string_view params,
                                        const Bytes &rawBody = {},
                                        const std::vector<std::string> &headerV = {},
                                        const std::vector<std::string> &formFieldNameV = {},
                                        const std::vector<std::string> &formFieldMimeV = {},
                                        const std::vector<std::string> &formFieldDataV = {},
                                        const std::FILE *cFILE = nullptr, size_t fileSize = 0,
                                        size_t timeout = NGenXXHttpDefaultTimeout);

NGenXXHttpResponse ngenxxNetHttpRequest(std::string_view url,
                                        NGenXXHttpMethodX method,
                                        const std::unordered_map<std::string, Any> &params,
                                        const Bytes &rawBody = {},
                                        const std::unordered_map<std::string, std::string> &headers = {},
                                        const std::vector<std::string> &formFieldNameV = {},
                                        const std::vector<std::string> &formFieldMimeV = {},
                                        const std::vector<std::string> &formFieldDataV = {},
                                        const std::FILE *cFILE = nullptr, size_t fileSize = 0,
                                        size_t timeout = NGenXXHttpDefaultTimeout);

bool ngenxxNetHttpDownload(std::string_view url, const std::string &filePath,
                           size_t timeout = NGenXXHttpDefaultTimeout);

#endif // NGENXX_INCLUDE_NET_HXX_
