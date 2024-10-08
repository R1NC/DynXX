#ifndef NGENXX_NET_HTTP_H_
#define NGENXX_NET_HTTP_H_

#include "NGenXXTypes.h"

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
    NGenXXNetHttpMethodPut,
};

/**
 * @brief http request
 * @param url URL
 * @param params params(transfer multiple params like `v1=a&v2=b`)
 * @param method HTTP method, see `NGenXXNetHttpMethod`
 * @param header_v HTTP header vector, max length is 8190
 * @param header_c HTTP header count, max count is 100
 * @param form_field_name_v Form name vector
 * @param form_field_mime_v Form mime vector
 * @param form_field_data_content_v Form data bytes vector
 * @param form_field_data_length_v Form data bytes length
 * @param form_field_count Form count
 * @param cFILE A `FILE` (pointer) to PUT
 * @param fileSize File size
 * @param timeout Timeout(milliseconds)
 * @return response
 */
const char *ngenxx_net_http_request(const char *url, const char *params, const int method,
                                    const char **header_v, const unsigned int header_c,
                                    const char **form_field_name_v,
                                    const char **form_field_mime_v,
                                    const unsigned char **form_field_data_content_v, const unsigned int *form_field_data_length_v,
                                    const unsigned int form_field_count,
                                    const void *cFILE, const unsigned long file_size,
                                    const unsigned long timeout);

#ifdef  __cplusplus
}
#endif

#endif // NGENXX_NET_HTTP_H_