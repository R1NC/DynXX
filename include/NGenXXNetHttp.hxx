#ifndef NGENXX_INCLUDE_NET_HTTP_HXX_
#define NGENXX_INCLUDE_NET_HTTP_HXX_

#include "NGenXXTypes.hxx"

#include <unordered_map>

constexpr size_t NGenXXHttpDefaultTimeout = 15 * 1000;

enum class NGenXXHttpMethodX : int
{
    Get,
    Post,
    Put,
};

struct NGenXXHttpResponse
{
    int code;
    std::string contentType;
    std::unordered_map<std::string, std::string> headers;
    std::string data;

    std::string toJson();
};
using NGenXXHttpResponse = struct NGenXXHttpResponse;

NGenXXHttpResponse ngenxxNetHttpRequest(const std::string &url,
                                              const NGenXXHttpMethodX method,
                                              const std::string &params,
                                              const Bytes &rawBody = BytesEmpty,
                                              const std::vector<std::string> &headerV = {},
                                              const std::vector<std::string> &formFieldNameV = {},
                                              const std::vector<std::string> &formFieldMimeV = {},
                                              const std::vector<std::string> &formFieldDataV = {},
                                              const std::FILE *cFILE = nullptr, const size_t fileSize = 0,
                                              const size_t timeout = NGenXXHttpDefaultTimeout);

NGenXXHttpResponse ngenxxNetHttpRequest(const std::string &url,
                                              const NGenXXHttpMethodX method,
                                              const std::unordered_map<std::string, Any> &params,
                                              const Bytes &rawBody = BytesEmpty,
                                              const std::unordered_map<std::string, std::string> &headers = {},
                                              const std::vector<std::string> &formFieldNameV = {},
                                              const std::vector<std::string> &formFieldMimeV = {},
                                              const std::vector<std::string> &formFieldDataV = {},
                                              const std::FILE *cFILE = nullptr, const size_t fileSize = 0,
                                              const size_t timeout = NGenXXHttpDefaultTimeout);

bool ngenxxNetHttpDownload(const std::string &url, const std::string &filePath, const size_t timeout = NGenXXHttpDefaultTimeout);

#endif // NGENXX_INCLUDE_NET_HTTP_HXX_