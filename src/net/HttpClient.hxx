#ifndef NGENXX_NET_HTTP_CLIENT_H_
#define NGENXX_NET_HTTP_CLIENT_H_

#ifdef __cplusplus

#include <string>

namespace NGenXX
{
    namespace Net
    {
        namespace HttpClient
        {
            /**
             * @brief http request
             * @param url URL
             * @param params params(transfer multiple params like `v1=a&v2=b`)
             * @return response
             */
            const std::string Request(const std::string &url, const std::string &params);
        }
    }
}

#endif

#endif // NGENXX_NET_HTTP_CLIENT_H_