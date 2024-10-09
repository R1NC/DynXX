#ifndef NGENXX_NET_HTTP_CLIENT_HXX_
#define NGENXX_NET_HTTP_CLIENT_HXX_

#ifdef __cplusplus

#include "../NGenXX-Types.hxx"

#include <string>
#include <vector>
#include <cstdio>

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
        typedef struct HttpFormField HttpFormField;

        class HttpClient
        {
        public:
            /**
             *
             */
            HttpClient();

            /**
             * @brief http request
             * @param url URL
             * @param params params(transfer multiple params like `v1=a&v2=b`)
             * @return response
             */
            const std::string request(const std::string &url, const std::string &params, const int method,
                                      const std::vector<std::string> &headers,
                                      const std::vector<HttpFormField> &formFields,
                                      const std::FILE *cFILE, const size fileSize,
                                      const size timeout);

            /**
             *
             */
            ~HttpClient();
        };
    }
}

#endif

#endif // NGENXX_NET_HTTP_CLIENT_HXX_