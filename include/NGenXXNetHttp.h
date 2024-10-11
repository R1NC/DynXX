#ifndef NGENXX_NET_HTTP_H_
#define NGENXX_NET_HTTP_H_

#include "NGenXXTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

    static const size NGENXX_HTTP_HEADER_MAX_COUNT = 100;
    static const size NGENXX_HTTP_HEADER_MAX_LENGTH = 8190;

    static const size NGENXX_HTTP_FORM_FIELD_MAX_COUNT = 16;
    static const size NGENXX_HTTP_FORM_FIELD_NAME_MAX_LENGTH = 256;
    static const size NGENXX_HTTP_FORM_FIELD_MINE_MAX_LENGTH = 256;
    static const size NGENXX_HTTP_FORM_FIELD_DATA_MAX_LENGTH = 64 * 1024;

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
     * @param header_v HTTP header vector, max length is `NGENXX_HTTP_HEADER_MAX_LENGTH`
     * @param header_c HTTP header count, max count is `NGENXX_HTTP_HEADER_MAX_COUNT`
     * @param form_field_name_v Form field name vector, max length is `NGENXX_HTTP_FORM_FIELD_NAME_MAX_LENGTH`
     * @param form_field_mime_v Form field mime vector, max length is `NGENXX_HTTP_FORM_FIELD_MIME_MAX_LENGTH`
     * @param form_field_data_v Form field data bytes vector, max length is `NGENXX_HTTP_FORM_FIELD_DATA_MAX_LENGTH`
     * @param form_field_count Form field count, max ount is `NGENXX_HTTP_FORM_FIELD_MAX_COUNT`
     * @param cFILE A `FILE` (pointer) to PUT
     * @param fileSize File size
     * @param timeout Timeout(milliseconds)
     * @return response
     */
    const char *ngenxx_net_http_request(const char *url, const char *params, const int method,
                                        const char **header_v, const size header_c,
                                        const char **form_field_name_v,
                                        const char **form_field_mime_v,
                                        const char **form_field_data_v,
                                        const size form_field_count,
                                        const void *cFILE, const size file_size,
                                        const size timeout);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_NET_HTTP_H_