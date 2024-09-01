#ifndef NGENXX_NET_H_
#define NGENXX_NET_H_

/**
 * @brief http request
 * @param url URL
 * @param params params(transfer multiple params like `v1=a&v2=b`)
 * @return response
 */
const char *ngenxx_net_http_request(const char *url, const char *params);

#endif // NGENXX_NET_H_