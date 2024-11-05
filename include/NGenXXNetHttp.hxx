#ifndef NGENXX_INCLUDE_NET_HTTP_HXX_
#define NGENXX_INCLUDE_NET_HTTP_HXX_

#include "NGenXXTypes.hxx"

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

constexpr size_t NGenXXHttpDefaultTimeout = 15 * 1000;

enum class NGenXXHttpMethodX : int
{
    Get,
    Post,
    Put,
};

const std::string ngenxxNetHttpRequest(const std::string &url,
                                       const NGenXXHttpMethodX method,
                                       const std::string &params,
                                       const std::vector<std::string> &headerV = {},
                                       const std::vector<std::string> &formFieldNameV = {},
                                       const std::vector<std::string> &formFieldMimeV = {},
                                       const std::vector<std::string> &formFieldDataV = {},
                                       const std::FILE *cFILE = nullptr, const size_t fileSize = 0,
                                       const size_t timeout = NGenXXHttpDefaultTimeout);

const std::string ngenxxNetHttpRequest(const std::string &url,
                                       const NGenXXHttpMethodX method,
                                       const std::unordered_map<std::string, Any> &params,
                                       const std::unordered_map<std::string, std::string> &headers = {},
                                       const std::vector<std::string> &formFieldNameV = {},
                                       const std::vector<std::string> &formFieldMimeV = {},
                                       const std::vector<std::string> &formFieldDataV = {},
                                       const std::FILE *cFILE = nullptr, const size_t fileSize = 0,
                                       const size_t timeout = NGenXXHttpDefaultTimeout);

#endif // NGENXX_INCLUDE_NET_HTTP_HXX_