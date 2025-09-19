#ifndef DYNXX_SRC_CORE_NET_HTTP_CLIENT_HXX_
#define DYNXX_SRC_CORE_NET_HTTP_CLIENT_HXX_

#if defined(__cplusplus)

#include <curl/curl.h>

#include <DynXX/CXX/Types.hxx>
#include <DynXX/CXX/Net.hxx>

namespace DynXX::Core::Net {
    struct HttpFormField {
        std::string name;
        std::string mime;
        std::string data;

        HttpFormField() = delete;
        explicit HttpFormField(const std::string& name, const std::string& mime, const std::string& data)
            : name(name), mime(mime), data(data) {}
    };

    class HttpClient {
    public:
        HttpClient();

        HttpClient(const HttpClient &) = delete;

        HttpClient &operator=(const HttpClient &) = delete;

        HttpClient(HttpClient &&) = delete;

        HttpClient &operator=(HttpClient &&) = delete;

        [[nodiscard]] DynXXHttpResponse request(std::string_view url, DynXXHttpMethodX method,
                                                 const std::vector<std::string> &headers,
                                                std::string_view params,
                                                 BytesView rawBody,
                                                 const std::vector<HttpFormField> &formFields,
                                                std::FILE *cFILE, size_t fileSize,
                                                 size_t timeout) const;

        [[nodiscard]] bool download(std::string_view url, std::string_view filePath, size_t timeout) const;

        ~HttpClient();
    };
}

#endif

#endif // DYNXX_SRC_CORE_NET_HTTP_CLIENT_HXX_
