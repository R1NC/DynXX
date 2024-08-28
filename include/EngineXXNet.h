#ifndef ENGINEXX_NET_H_
#define ENGINEXX_NET_H_

/**
 * @brief http request
 * @param url URL
 * @param params params(transfer multiple params like `v1=a&v2=b`)
 * @return response
 */
const char *enginexx_net_http_req(const char *url, const char *params);

#endif // ENGINEXX_NET_H_