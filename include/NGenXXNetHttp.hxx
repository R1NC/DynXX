#ifndef NGENXX_INCLUDE_NET_HTTP_HXX_
#define NGENXX_INCLUDE_NET_HTTP_HXX_

#include "NGenXXTypes.hxx"

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

enum class NGenXXHttpMethodX : int
{
    Get,
    Post,
    Put,
};

const std::string ngenxxNetHttpRequest(const std::string &url,
                                       const std::string &params,
                                       const NGenXXHttpMethodX method,
                                       const std::vector<std::string> &headerV,
                                       const std::vector<std::string> &formFieldNameV,
                                       const std::vector<std::string> &formFieldMimeV,
                                       const std::vector<std::string> &formFieldDataV,
                                       const std::FILE *cFILE, const size_t fileSize,
                                       const size_t timeout);

const std::string ngenxxNetHttpRequest(const std::string &url,
                                       const std::unordered_map<std::string, Any> &params,
                                       const NGenXXHttpMethodX method,
                                       const std::unordered_map<std::string, std::string> &headers,
                                       const std::vector<std::string> &formFieldNameV,
                                       const std::vector<std::string> &formFieldMimeV,
                                       const std::vector<std::string> &formFieldDataV,
                                       const std::FILE *cFILE, const size_t fileSize,
                                       const size_t timeout);

#endif //  NGENXX_INCLUDE_NET_HTTP_HXX_