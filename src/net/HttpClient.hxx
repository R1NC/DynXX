#ifndef NGENXX_NET_HTTP_CLIENT_HXX_
#define NGENXX_NET_HTTP_CLIENT_HXX_

#ifdef __cplusplus

#include <string>
#include <vector>

namespace NGenXX
{
    namespace Net
    {
        namespace HttpClient
        {
            /**
             * 
             */
            void create(void);

            /**
             * @brief http request
             * @param url URL
             * @param params params(transfer multiple params like `v1=a&v2=b`)
             * @return response
             */
            const std::string request(const std::string &url, const std::string &params, int method, std::vector<std::string> &headers, long timeout);

            /**
             * 
             */
            void destroy(void);
        }
    }
}

#endif

#endif // NGENXX_NET_HTTP_CLIENT_HXX_