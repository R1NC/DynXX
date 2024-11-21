#ifndef NGENXX_SRC_NET_HTTP_CLIENT_HXX_
#define NGENXX_SRC_NET_HTTP_CLIENT_HXX_

#ifdef __cplusplus

#include "../../include/NGenXXTypes.hxx"

#include <string>
#include <vector>
#include <cstdio>
#include <unordered_map>

namespace NGenXX
{
    namespace Net
    {
        struct HttpFormField
        {
            std::string name;
            std::string mime;
            std::string data;
        };
        using HttpFormField = struct HttpFormField;

        struct HttpResponse
        {
            int code;
            std::string contentType;
            std::unordered_map<std::string, std::string> headers;
            std::string data;
        };
        using HttpResponse = struct HttpResponse;

        class HttpClient
        {
        public:
            HttpClient();

            const HttpResponse request(const std::string &url, const int method,
                                      const std::vector<std::string> &headers,
                                      const std::string &params,
                                      const Bytes &rawBody,
                                      const std::vector<HttpFormField> &formFields,
                                      const std::FILE *cFILE, const size_t fileSize,
                                      const size_t timeout);

            ~HttpClient();
        };
    }
}

#endif

#endif // NGENXX_SRC_NET_HTTP_CLIENT_HXX_