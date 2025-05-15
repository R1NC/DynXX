#ifndef NGENXX_INCLUDE_NET_HTTP_H_
#define NGENXX_INCLUDE_NET_HTTP_H_

#include <stddef.h>

#include "NGenXXInternal.h"

EXTERN_C_BEGIN

    static const size_t NGENXX_HTTP_DEFAULT_TIMEOUT = 15 * 1000;

    static const size_t NGENXX_HTTP_HEADER_MAX_COUNT = 100;
    static const size_t NGENXX_HTTP_HEADER_MAX_LENGTH = 8190;

    static const size_t NGENXX_HTTP_FORM_FIELD_MAX_COUNT = 16;
    static const size_t NGENXX_HTTP_FORM_FIELD_NAME_MAX_LENGTH = 256;
    static const size_t NGENXX_HTTP_FORM_FIELD_MINE_MAX_LENGTH = 256;
    static const size_t NGENXX_HTTP_FORM_FIELD_DATA_MAX_LENGTH = 64 * 1024;

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
     * @param file_size File size
     * @param timeout Timeout(milliseconds)
     * @return response
     */
    const char *ngenxx_net_http_request(const char *url, const char *params, int method,
                                        const char **header_v, size_t header_c,
                                        const char **form_field_name_v,
                                        const char **form_field_mime_v,
                                        const char **form_field_data_v,
                                        size_t form_field_count,
                                        void *const cFILE, size_t file_size,
                                        size_t timeout);

    /**
     * @brief download file
     * @param url file URL
     * @param file_path file path
     * @param timeout Timeout(milliseconds)
     * @return download result
     */
    bool ngenxx_net_http_download(const char *url, const char *file_path, size_t timeout);

EXTERN_C_END

#endif // NGENXX_INCLUDE_NET_HTTP_H_