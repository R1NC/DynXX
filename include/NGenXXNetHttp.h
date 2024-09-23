#ifndef NGENXX_NET_HTTP_H_
#define NGENXX_NET_HTTP_H_

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * HTTP method
 */
enum NGenXXNetHttpMethod
{
    NGenXXNetHttpMethodGet,
    NGenXXNetHttpMethodPost,
};

/**
 * @brief http request
 * @param url URL
 * @param params params(transfer multiple params like `v1=a&v2=b`)
 * @param method HTTP method, see `NGenXXNetHttpMethod`
 * @param headers_v HTTP headers vector, max length is 8190
 * @param headers_c HTTP headers count, max count is 100
 * @param timeout Timeout(milliseconds)
 * @return response
 */
const char *ngenxx_net_http_request(const char *url, const char *params, const int method, const char **headers_v, const unsigned int headers_c, const unsigned long timeout);

#ifdef  __cplusplus
}
#endif

#endif // NGENXX_NET_HTTP_H_